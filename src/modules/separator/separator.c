#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "common/textModifier.h"
#include "logo/logo.h"
#include "modules/separator/separator.h"

bool ffPrintSeparator(FFSeparatorOptions* options) {
    ffLogoPrintLine();

    if (options->outputColor.length && !instance.config.display.pipe) {
        ffPrintColor(&options->outputColor);
    }

    if (options->times > 0) {
        if (__builtin_expect(options->string.length == 1, 1)) {
            ffPrintCharTimes(options->string.chars[0], options->times);
        } else {
            for (uint32_t i = 0; i < options->times; i++) {
                fputs(options->string.chars, stdout);
            }
        }
    } else {
        const FFPlatform* platform = &instance.state.platform;

        uint32_t titleLength = 1                                                                                      // @
            + ffUtf8StrWidth(platform->userName.chars, platform->userName.length)                                     // user name
            + (instance.state.titleFqdn ? platform->hostName.length : ffStrbufFirstIndexC(&platform->hostName, '.')); // host name

        if (__builtin_expect(options->string.length == 1, 1)) {
            ffPrintCharTimes(options->string.chars[0], titleLength);
        } else {
            uint32_t wcsLength = ffUtf8StrWidth(options->string.chars, options->string.length);

            int remaining = (int) titleLength;
            // Write the whole separator as often as it fits fully into titleLength
            for (; remaining >= (int) wcsLength; remaining -= (int) wcsLength) {
                ffStrbufWriteTo(&options->string, stdout);
            }

            if (remaining > 0) {
                // Write as much of the separator as needed to fill titleLength
                if (wcsLength != options->string.length) {
                    // Unicode chars
                    const char* ptr = options->string.chars;
                    uint32_t remainBytes = options->string.length;
                    while (remaining > 0 && remainBytes > 0 && *ptr != '\0') {
                        uint8_t charWidth = 0;
                        uint8_t bytes = ffUtf8CharLenWidth(ptr, remainBytes, &charWidth);
                        if (__builtin_expect(bytes == 0, false)) {
                            break;
                        }

                        remaining -= (int) charWidth;
                        ptr += bytes;
                        remainBytes -= bytes;
                    }
                    fwrite(options->string.chars, (size_t) (ptr - options->string.chars), 1, stdout);
                } else {
                    fwrite(options->string.chars, (size_t) remaining, 1, stdout);
                }
            }
        }
    }

    if (options->outputColor.length && !instance.config.display.pipe) {
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
    }
    putchar('\n');

    return true;
}

void ffParseSeparatorJsonObject(FFSeparatorOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (unsafe_yyjson_equals_str(key, "type") || unsafe_yyjson_equals_str(key, "condition")) {
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "string")) {
            ffStrbufSetJsonVal(&options->string, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "outputColor")) {
            ffOptionParseColor(yyjson_get_str(val), &options->outputColor);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "times")) {
            options->times = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "length")) {
            ffPrintError(FF_SEPARATOR_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "The option length has been renamed to times.");
            continue;
        }

        ffPrintError(FF_SEPARATOR_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateSeparatorJsonConfig(FFSeparatorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    yyjson_mut_obj_add_strbuf(doc, module, "string", &options->string);
    yyjson_mut_obj_add_strbuf(doc, module, "outputColor", &options->outputColor);
    yyjson_mut_obj_add_uint(doc, module, "times", options->times);
}

void ffInitSeparatorOptions(FFSeparatorOptions* options) {
    ffStrbufInitStatic(&options->string, "-");
    ffStrbufInit(&options->outputColor);
    options->times = 0;
}

void ffDestroySeparatorOptions(FFSeparatorOptions* options) {
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
