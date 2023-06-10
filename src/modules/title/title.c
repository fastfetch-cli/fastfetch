#include "fastfetch.h"
#include "common/printing.h"
#include "util/textModifier.h"
#include "modules/title/title.h"

static inline void printTitlePart(FFinstance* instance, const FFstrbuf* content)
{
    if(!instance->config.pipe)
    {
        fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
        ffPrintColor(&instance->config.colorTitle);
    }

    ffStrbufWriteTo(content, stdout);

    if(!instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
}

void ffPrintTitle(FFinstance* instance, FFTitleOptions* options)
{
    ffLogoPrintLine(instance);

    printTitlePart(instance, &instance->state.platform.userName);
    putchar('@');
    FFstrbuf* host = options->fdqn ?
        &instance->state.platform.domainName :
        &instance->state.platform.hostName;
    printTitlePart(instance, host);

    instance->state.titleLength = instance->state.platform.userName.length + host->length + 1;

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

    if (strcasecmp(subKey, "fdqn") == 0)
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

#ifdef FF_HAVE_JSONC
void ffParseTitleJsonObject(FFinstance* instance, json_object* module)
{
    FFTitleOptions __attribute__((__cleanup__(ffDestroyTitleOptions))) options;
    ffInitTitleOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (strcasecmp(key, "fdqn") == 0)
            {
                options.fdqn = json_object_get_boolean(val);
                continue;
            }

            ffPrintErrorString(instance, FF_TITLE_MODULE_NAME, 0, NULL, NULL, "Unknown JSON key %s", key);
        }
    }

    ffPrintTitle(instance, &options);
}
#endif
