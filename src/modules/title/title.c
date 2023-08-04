#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/title/title.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

#define FF_TITLE_NUM_FORMAT_ARGS 2

static inline void printTitlePart(const FFstrbuf* content, const FFstrbuf* color)
{
    if(!instance.config.pipe)
    {
        if (instance.config.brightColor)
            fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
        ffPrintColor(color->length > 0 ? color : &instance.config.colorTitle);
    }

    ffStrbufWriteTo(content, stdout);

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
}

void ffPrintTitle(FFTitleOptions* options)
{
    FFstrbuf* host = options->fqdn ?
        &instance.state.platform.domainName :
        &instance.state.platform.hostName;

    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(options->moduleArgs.key.length == 0 ? NULL : FF_TITLE_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);

        printTitlePart(&instance.state.platform.userName, &options->colorUser);

        if (!instance.config.pipe && options->colorAt.length > 0)
            ffPrintColor(&options->colorAt);
        putchar('@');

        printTitlePart(host, &options->colorHost);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_TITLE_MODULE_NAME, 0, &options->moduleArgs, FF_TITLE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance.state.platform.userName},
            {FF_FORMAT_ARG_TYPE_STRBUF, host},
        });
    }
}

void ffInitTitleOptions(FFTitleOptions* options)
{
    options->moduleName = FF_TITLE_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
    options->fqdn = false;
    ffStrbufInit(&options->colorUser);
    ffStrbufInit(&options->colorAt);
    ffStrbufInit(&options->colorHost);
}

bool ffParseTitleCommandOptions(FFTitleOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TITLE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "fqdn"))
    {
        options->fqdn = ffOptionParseBoolean(value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "color-user"))
    {
        ffOptionParseColor(value, &options->colorUser);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "color-at"))
    {
        ffOptionParseColor(value, &options->colorAt);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "color-host"))
    {
        ffOptionParseColor(value, &options->colorHost);
        return true;
    }

    return false;
}

void ffDestroyTitleOptions(FFTitleOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->colorUser);
    ffStrbufDestroy(&options->colorAt);
    ffStrbufDestroy(&options->colorHost);
}

void ffParseTitleJsonObject(yyjson_val* module)
{
    FFTitleOptions __attribute__((__cleanup__(ffDestroyTitleOptions))) options;
    ffInitTitleOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (ffStrEqualsIgnCase(key, "fqdn"))
            {
                options.fqdn = yyjson_get_bool(val);
                continue;
            }

            if (ffStrEqualsIgnCase(key, "color"))
            {
                if (!yyjson_is_obj(val))
                    continue;

                yyjson_val* color = yyjson_obj_get(val, "user");
                if (color)
                    ffOptionParseColor(yyjson_get_str(color), &options.colorUser);
                color = yyjson_obj_get(val, "at");
                if (color)
                    ffOptionParseColor(yyjson_get_str(color), &options.colorAt);
                color = yyjson_obj_get(val, "host");
                if (color)
                    ffOptionParseColor(yyjson_get_str(color), &options.colorHost);
                continue;
            }

            ffPrintErrorString(FF_TITLE_MODULE_NAME, 0, NULL, NULL, "Unknown JSON key %s", key);
        }
    }

    ffPrintTitle(&options);
}
