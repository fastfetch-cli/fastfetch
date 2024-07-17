#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/separator/separator.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"
#include "util/wcwidth.h"
#include "util/textModifier.h"

#include <locale.h>

static inline uint32_t max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

static inline uint32_t getWcsWidth(const FFstrbuf* mbstr, wchar_t* wstr, mbstate_t* state)
{
    int result = 1;
    for (uint32_t i = 0; i < mbstr->length; i++)
    {
        if (!isascii(mbstr->chars[i]))
        {
            result = 0;
            break;
        }
    }
    if (__builtin_expect(result, 1)) return mbstr->length;

    const char* str = mbstr->chars;
    uint32_t wstrLength = (uint32_t) mbsrtowcs(wstr, &str, mbstr->length, state);
    result = mk_wcswidth(wstr, wstrLength);
    return result > 0 ? (uint32_t) result : mbstr->length;
}

void ffPrintSeparator(FFSeparatorOptions* options)
{
    ffLogoPrintLine();

    if (options->length > 0)
    {
        if(__builtin_expect(options->string.length == 1, 1))
            ffPrintCharTimes(options->string.chars[0], options->length);
        else
        {
            for (uint32_t i = 0; i < options->length; i++)
            {
                fputs(options->string.chars, stdout);
            }
        }
        putchar('\n');
        return;
    }

    setlocale(LC_CTYPE, "");
    mbstate_t state = {};
    bool fqdn = instance.config.modules.title.fqdn;
    const FFPlatform* platform = &instance.state.platform;

    FF_AUTO_FREE wchar_t* wstr = malloc((max(
        platform->userName.length, options->string.length) + 1) * sizeof(*wstr));

    uint32_t titleLength = 1 // @
        + getWcsWidth(&platform->userName, wstr, &state) // user name
        + (fqdn ? platform->hostName.length : ffStrbufFirstIndexC(&platform->hostName, '.')); // host name

    if(options->outputColor.length && !instance.config.display.pipe)
        ffPrintColor(&options->outputColor);
    if(__builtin_expect(options->string.length == 1, 1))
    {
        ffPrintCharTimes(options->string.chars[0], titleLength);
    }
    else
    {
        uint32_t wcsLength = getWcsWidth(&options->string, wstr, &state);

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
                for(int i = 0; remaining > 0; ++i)
                {
                    #ifdef __linux__
                    // https://stackoverflow.com/questions/75126743/i-have-difficulties-with-putwchar-in-c#answer-75137784
                    char wch[16] = "";
                    uint32_t wchLength = (uint32_t) wcrtomb(wch, wstr[i], &state);
                    fwrite(wch, wchLength, 1, stdout);
                    #else
                    putwchar(wstr[i]);
                    #endif
                    int width = mk_wcwidth(wstr[i]);
                    remaining -= width < 0 ? 0 : width;
                }
            }
            else
            {
                for(int i = 0; i < remaining; i++)
                    putchar(options->string.chars[i]);
            }
        }
    }
    if(options->outputColor.length && !instance.config.display.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
    putchar('\n');
    setlocale(LC_CTYPE, "C");
}

bool ffParseSeparatorCommandOptions(FFSeparatorOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_SEPARATOR_MODULE_NAME);
    if (!subKey) return false;

    if (ffStrEqualsIgnCase(subKey, "string"))
    {
        ffOptionParseString(key, value, &options->string);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "output-color"))
    {
        ffOptionParseColor(value, &options->outputColor);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "length"))
    {
        options->length = ffOptionParseUInt32(key, value);
        return true;
    }

    return false;
}

void ffParseSeparatorJsonObject(FFSeparatorOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffStrEqualsIgnCase(key, "string"))
        {
            ffStrbufSetS(&options->string, yyjson_get_str(val));
            continue;
        }

        if (ffStrEndsWithIgnCase(key, "outputColor"))
        {
            ffOptionParseColor(yyjson_get_str(val), &options->outputColor);
            continue;
        }

        if (ffStrEndsWithIgnCase(key, "length"))
        {
            options->length = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        ffPrintError(FF_SEPARATOR_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", key);
    }
}

void ffGenerateSeparatorJsonConfig(FFSeparatorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroySeparatorOptions))) FFSeparatorOptions defaultOptions;
    ffInitSeparatorOptions(&defaultOptions);

    if (!ffStrbufEqual(&options->string, &defaultOptions.string))
        yyjson_mut_obj_add_strbuf(doc, module, "string", &options->string);
}

void ffInitSeparatorOptions(FFSeparatorOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_SEPARATOR_MODULE_NAME,
        "Print a separator line",
        ffParseSeparatorCommandOptions,
        ffParseSeparatorJsonObject,
        ffPrintSeparator,
        NULL,
        NULL,
        ffGenerateSeparatorJsonConfig
    );
    ffStrbufInitStatic(&options->string, "-");
    ffStrbufInit(&options->outputColor);
    options->length = 0;
}

void ffDestroySeparatorOptions(FFSeparatorOptions* options)
{
    ffStrbufDestroy(&options->string);
}
