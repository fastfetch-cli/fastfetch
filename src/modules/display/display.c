#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "modules/display/display.h"

#define FF_DISPLAY_NUM_FORMAT_ARGS 8

void ffPrintDisplay(FFinstance* instance, FFDisplayOptions* options)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_DISPLAY_MODULE_NAME, 0, &instance->config.display.moduleArgs, "Display detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* dsResult = ffConnectDisplayServer(instance);

    if(dsResult->displays.length == 0)
    {
        ffPrintError(instance, FF_DISPLAY_MODULE_NAME, 0, &instance->config.display.moduleArgs, "Couldn't detect display");
        return;
    }

    if (options->compactType != FF_DISPLAY_COMPACT_TYPE_NONE)
    {
        ffPrintLogoAndKey(instance, FF_DISPLAY_MODULE_NAME, 0, &options->moduleArgs.key);

        int index = 0;
        FF_LIST_FOR_EACH(FFDisplayResult, result, dsResult->displays)
        {
            if (options->compactType & FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT)
            {
                if (index++) putchar(' ');
                printf("%ix%i", result->width, result->height);
            }
            if (options->compactType & FF_DISPLAY_COMPACT_TYPE_SCALED_BIT)
            {
                if (index++) putchar(' ');
                printf("%ix%i", result->scaledWidth, result->scaledHeight);
            }
        }
        putchar('\n');
        return;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    for(uint32_t i = 0; i < dsResult->displays.length; i++)
    {
        FFDisplayResult* result = ffListGet(&dsResult->displays, i);
        uint8_t moduleIndex = dsResult->displays.length == 1 ? 0 : (uint8_t) (i + 1);
        const char* displayType = result->type == FF_DISPLAY_TYPE_UNKNOWN ? NULL : result->type == FF_DISPLAY_TYPE_BUILTIN ? "built-in" : "external";

        if(options->moduleArgs.outputFormat.length == 0)
        {
            if((options->detectName && result->name.length) || (moduleIndex > 0 && displayType))
            {
                ffStrbufClear(&key);
                if(options->moduleArgs.key.length == 0)
                {
                    ffStrbufAppendF(&key, "%s (%s)", FF_DISPLAY_MODULE_NAME, result->name.length ? result->name.chars : displayType);
                }
                else
                {
                    ffParseFormatString(&key, &options->moduleArgs.key, 1, (FFformatarg[]){
                        {FF_FORMAT_ARG_TYPE_UINT, &i},
                        {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
                        {FF_FORMAT_ARG_TYPE_STRING, displayType},
                    });
                }
                ffPrintLogoAndKey(instance, key.chars, 0, NULL);
            }
            else
            {
                ffPrintLogoAndKey(instance, FF_DISPLAY_MODULE_NAME, moduleIndex, &options->moduleArgs.key);
            }

            printf("%ix%i", result->width, result->height);

            if(result->refreshRate > 0)
            {
                if(options->preciseRefreshRate)
                    printf(" @ %gHz", ((int) (result->refreshRate * 1000 + 0.5)) / 1000.0);
                else
                    printf(" @ %iHz", (uint32_t) (result->refreshRate + 0.5));
            }

            if(
                result->scaledWidth > 0 && result->scaledWidth != result->width &&
                result->scaledHeight > 0 && result->scaledHeight != result->height)
                printf(" (as %ix%i)", result->scaledWidth, result->scaledHeight);

            putchar('\n');
        }
        else
        {
            ffPrintFormat(instance, FF_DISPLAY_MODULE_NAME, moduleIndex, &options->moduleArgs, FF_DISPLAY_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_UINT, &result->width},
                {FF_FORMAT_ARG_TYPE_UINT, &result->height},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &result->refreshRate},
                {FF_FORMAT_ARG_TYPE_UINT, &result->scaledWidth},
                {FF_FORMAT_ARG_TYPE_UINT, &result->scaledHeight},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
                {FF_FORMAT_ARG_TYPE_STRING, displayType},
                {FF_FORMAT_ARG_TYPE_UINT, &result->rotation},
            });
        }
    }
}

void ffInitDisplayOptions(FFDisplayOptions* options)
{
    options->moduleName = FF_DISPLAY_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
    options->compactType = FF_DISPLAY_COMPACT_TYPE_NONE;
    options->detectName = false;
    options->preciseRefreshRate = false;
}

bool ffParseDisplayCommandOptions(FFDisplayOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DISPLAY_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "compact-type") == 0)
    {
        options->compactType = (FFDisplayCompactType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "none", FF_DISPLAY_COMPACT_TYPE_NONE },
            { "original", FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT },
            { "scaled", FF_DISPLAY_COMPACT_TYPE_SCALED_BIT },
            {},
        });
        return true;
    }

    if (strcasecmp(subKey, "detect-name") == 0)
    {
        options->detectName = ffOptionParseBoolean(value);
        return true;
    }

    if (strcasecmp(subKey, "precise-refresh-rate") == 0)
    {
        options->preciseRefreshRate = ffOptionParseBoolean(value);
        return true;
    }

    return false;
}

void ffDestroyDisplayOptions(FFDisplayOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
void ffParseDisplayJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFDisplayOptions __attribute__((__cleanup__(ffDestroyDisplayOptions))) options;
    ffInitDisplayOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "compactType") == 0)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                    { "none", FF_DISPLAY_COMPACT_TYPE_NONE },
                    { "original", FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT },
                    { "scaled", FF_DISPLAY_COMPACT_TYPE_SCALED_BIT },
                    {},
                });
                if (error)
                    ffPrintError(instance, FF_DISPLAY_MODULE_NAME, 0, &options.moduleArgs, "Invalid %s value: %s", key, error);
                else
                    options.compactType = (FFDisplayCompactType) value;
                continue;
            }

            if (strcasecmp(key, "detectName") == 0)
            {
                options.detectName = yyjson_get_bool(val);
                continue;
            }

            if (strcasecmp(key, "preciseRefreshRate") == 0)
            {
                options.preciseRefreshRate = yyjson_get_bool(val);
                continue;
            }

            ffPrintError(instance, FF_DISPLAY_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintDisplay(instance, &options);
}
