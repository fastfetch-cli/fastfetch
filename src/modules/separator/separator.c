#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/separator/separator.h"
#include "util/stringUtils.h"

void ffPrintSeparator(FFinstance* instance, FFSeparatorOptions* options)
{
    uint32_t titleLength = instance->state.titleLength;
    if (titleLength == 0)
    {
        // Title was not printed, should we support this case?
        titleLength = instance->state.platform.userName.length + 1 + (instance->config.title.fdqn ?
            instance->state.platform.domainName.length :
            instance->state.platform.hostName.length
        );
    }

    ffLogoPrintLine(instance);

    if(options->string.length == 0)
    {
        ffPrintCharTimes('-', titleLength);
    }
    else
    {
        //Write the whole separator as often as it fits fully into titleLength
        for(uint32_t i = 0; i < titleLength / options->string.length; i++)
            ffStrbufWriteTo(&options->string, stdout);

        //Write as much of the separator as needed to fill titleLength
        for(uint32_t i = 0; i < titleLength % options->string.length; i++)
            putchar(options->string.chars[i]);
    }
    putchar('\n');
}

void ffInitSeparatorOptions(FFSeparatorOptions* options)
{
    options->moduleName = FF_SEPARATOR_MODULE_NAME;
    ffStrbufInit(&options->string);
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

    return false;
}

void ffDestroySeparatorOptions(FFSeparatorOptions* options)
{
    ffStrbufDestroy(&options->string);
}

void ffParseSeparatorJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFSeparatorOptions __attribute__((__cleanup__(ffDestroySeparatorOptions))) options;
    ffInitSeparatorOptions(&options);

    if (module)
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
                ffStrbufSetS(&options.string, yyjson_get_str(val));
                continue;
            }

            ffPrintErrorString(instance, FF_SEPARATOR_MODULE_NAME, 0, NULL, NULL, "Unknown JSON key %s", key);
        }
    }

    ffPrintSeparator(instance, &options);
}
