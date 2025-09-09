#include "common/printing.h"
#include "common/jsonconfig.h"
#include "logo/logo.h"
#include "modules/separator/separator.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"
#include "util/wcwidth.h"
#include "util/textModifier.h"

#include <locale.h>

#if __SIZEOF_WCHAR_T__ == 4
    static inline size_t mbrtoc32(uint32_t* restrict pc32, const char* restrict s, size_t n, mbstate_t* restrict ps)
    {
        return mbrtowc((wchar_t*) pc32, s, n, ps);
    }
#else
    #include <uchar.h>
#endif

static uint8_t getMbrWidth(const char* mbstr, uint32_t length, const char** next, mbstate_t* state)
{
    if (__builtin_expect((uint8_t) *mbstr < 0x80, true)) // ASCII fast path
    {
        if (next) *next = mbstr + 1;
        return 1;
    }

    uint32_t c32;
    uint32_t len = (uint32_t) mbrtoc32(&c32, mbstr, length, state);
    if (len >= (uint32_t) -3)
    {
        // Invalid or incomplete multibyte sequence
        if (next) *next = mbstr + 1;
        return 1;
    }

    if (next) *next = mbstr + len;
    int width = mk_wcwidth(c32);
    return width < 0 ? 0 : (uint8_t) width;
}

static uint32_t getWcsWidth(const FFstrbuf* mbstr)
{
    mbstate_t state = {};
    uint32_t remainLength = mbstr->length;
    uint32_t result = 0;

    const char* ptr = mbstr->chars;
    while (remainLength > 0 && *ptr != '\0')
    {
        const char* lastPtr = NULL;
        result += getMbrWidth(ptr, remainLength, &lastPtr, &state);
        remainLength -= (uint32_t)(lastPtr - ptr);
        ptr = lastPtr;
    }

    return result > 0 ? (uint32_t) result : mbstr->length;
}

bool ffPrintSeparator(FFSeparatorOptions* options)
{
    ffLogoPrintLine();

    if(options->outputColor.length && !instance.config.display.pipe)
        ffPrintColor(&options->outputColor);

    if (options->times > 0)
    {
        if(__builtin_expect(options->string.length == 1, 1))
            ffPrintCharTimes(options->string.chars[0], options->times);
        else
        {
            for (uint32_t i = 0; i < options->times; i++)
            {
                fputs(options->string.chars, stdout);
            }
        }
    }
    else
    {
        setlocale(LC_CTYPE, "");
        const FFPlatform* platform = &instance.state.platform;

        uint32_t titleLength = 1 // @
            + getWcsWidth(&platform->userName) // user name
            + (instance.state.titleFqdn ? platform->hostName.length : ffStrbufFirstIndexC(&platform->hostName, '.')); // host name

        if(__builtin_expect(options->string.length == 1, 1))
        {
            ffPrintCharTimes(options->string.chars[0], titleLength);
        }
        else
        {
            uint32_t wcsLength = getWcsWidth(&options->string);

            int remaining = (int) titleLength;
            //Write the whole separator as often as it fits fully into titleLength
            for (; remaining >= (int) wcsLength; remaining -= (int) wcsLength)
                ffStrbufWriteTo(&options->string, stdout);

            if (remaining > 0)
            {
                //Write as much of the separator as needed to fill titleLength
                if (wcsLength != options->string.length)
                {
                    // Unicode chars
                    const char* ptr = options->string.chars;
                    mbstate_t state = {};
                    const char* next = NULL;
                    while (remaining > 0 && *ptr != '\0')
                    {
                        remaining -= (int) getMbrWidth(ptr, (uint32_t)(options->string.length - (ptr - options->string.chars)), &next, &state);
                        ptr = next;
                    }
                    fwrite(options->string.chars, (size_t) (ptr - options->string.chars), 1, stdout);
                }
                else
                {
                    fwrite(options->string.chars, (size_t) remaining, 1, stdout);
                }
            }
        }
        setlocale(LC_CTYPE, "C");
    }

    if(options->outputColor.length && !instance.config.display.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
    putchar('\n');

    return true;
}

void ffParseSeparatorJsonObject(FFSeparatorOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (unsafe_yyjson_equals_str(key, "type") || unsafe_yyjson_equals_str(key, "condition"))
            continue;

        if (unsafe_yyjson_equals_str(key, "string"))
        {
            ffStrbufSetJsonVal(&options->string, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "outputColor"))
        {
            ffOptionParseColor(yyjson_get_str(val), &options->outputColor);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "times"))
        {
            options->times = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "length"))
        {
            ffPrintError(FF_SEPARATOR_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "The option length has been renamed to times.");
            continue;
        }

        ffPrintError(FF_SEPARATOR_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateSeparatorJsonConfig(FFSeparatorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    yyjson_mut_obj_add_strbuf(doc, module, "string", &options->string);
    yyjson_mut_obj_add_strbuf(doc, module, "outputColor", &options->outputColor);
    yyjson_mut_obj_add_uint(doc, module, "times", options->times);
}

void ffInitSeparatorOptions(FFSeparatorOptions* options)
{
    ffStrbufInitStatic(&options->string, "-");
    ffStrbufInit(&options->outputColor);
    options->times = 0;
}

void ffDestroySeparatorOptions(FFSeparatorOptions* options)
{
    ffStrbufDestroy(&options->string);
}

FFModuleBaseInfo ffSeparatorModuleInfo = {
    .name = FF_SEPARATOR_MODULE_NAME,
    .description = "Print a separator line",
    .initOptions = (void*) ffInitSeparatorOptions,
    .destroyOptions = (void*) ffDestroySeparatorOptions,
    .parseJsonObject = (void*) ffParseSeparatorJsonObject,
    .printModule = (void*) ffPrintSeparator,
    .generateJsonConfig = (void*) ffGenerateSeparatorJsonConfig,
};
