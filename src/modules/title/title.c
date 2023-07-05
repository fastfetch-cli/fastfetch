#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/title/title.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

#define FF_TITLE_NUM_FORMAT_ARGS 2

static inline void printTitlePart(const FFstrbuf* content)
{
    if(!instance.config.pipe)
    {
        fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
        ffPrintColor(&instance.config.colorTitle);
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
        printTitlePart(&instance.state.platform.userName);
        putchar('@');
        printTitlePart(host);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_TITLE_MODULE_NAME, 0, &options->moduleArgs, FF_TITLE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance.state.platform.userName},
            {FF_FORMAT_ARG_TYPE_STRBUF, host},
        });
    }
    instance.state.titleLength = instance.state.platform.userName.length + host->length + 1;
}

void ffInitTitleOptions(FFTitleOptions* options)
{
    options->moduleName = FF_TITLE_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
    options->fqdn = false;
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

    return false;
}

void ffDestroyTitleOptions(FFTitleOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
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

            ffPrintErrorString(FF_TITLE_MODULE_NAME, 0, NULL, NULL, "Unknown JSON key %s", key);
        }
    }

    ffPrintTitle(&options);
}
