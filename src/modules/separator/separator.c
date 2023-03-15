#include "fastfetch.h"
#include "common/printing.h"
#include "modules/separator/separator.h"

#define FF_SEPARATOR_MODULE_NAME "Separator"

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

    if (strcasecmp(subKey, "string") == 0)
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

#ifdef FF_HAVE_JSONC
bool ffParseSeparatorJsonObject(FFinstance* instance, const char* type, json_object* module)
{
    if (strcasecmp(type, FF_SEPARATOR_MODULE_NAME) != 0)
        return false;

    FFSeparatorOptions __attribute__((__cleanup__(ffDestroySeparatorOptions))) options;
    ffInitSeparatorOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (strcasecmp(key, "type") == 0)
                continue;

            if (strcasecmp(key, "string") == 0)
            {
                ffStrbufSetS(&options.string, json_object_get_string(val));
                continue;
            }

            ffPrintErrorString(instance, FF_SEPARATOR_MODULE_NAME, 0, NULL, NULL, "Unknown JSON key %s", key);
        }
    }

    ffPrintSeparator(instance, &options);
    return true;
}
#endif
