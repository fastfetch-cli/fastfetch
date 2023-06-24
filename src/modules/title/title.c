#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/title/title.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

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
    ffLogoPrintLine();

    printTitlePart(&instance.state.platform.userName);
    putchar('@');
    FFstrbuf* host = options->fdqn ?
        &instance.state.platform.domainName :
        &instance.state.platform.hostName;
    printTitlePart(host);

    instance.state.titleLength = instance.state.platform.userName.length + host->length + 1;

    putchar('\n');
}

void ffInitTitleOptions(FFTitleOptions* options)
{
    options->moduleName = FF_TITLE_MODULE_NAME;
    options->fdqn = false;
}

bool ffParseTitleCommandOptions(FFTitleOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TITLE_MODULE_NAME);
    if (!subKey) return false;

    if (ffStrEqualsIgnCase(subKey, "fdqn"))
    {
        options->fdqn = ffOptionParseBoolean(value);
        return true;
    }

    return false;
}

void ffDestroyTitleOptions(FFTitleOptions* options)
{
    FF_UNUSED(options);
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

            if (ffStrEqualsIgnCase(key, "fdqn"))
            {
                options.fdqn = yyjson_get_bool(val);
                continue;
            }

            ffPrintErrorString(FF_TITLE_MODULE_NAME, 0, NULL, NULL, "Unknown JSON key %s", key);
        }
    }

    ffPrintTitle(&options);
}
