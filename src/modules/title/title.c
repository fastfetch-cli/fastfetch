#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/title/title.h"
#include "util/textModifier.h"

static void appendText(FFstrbuf* output, const FFstrbuf* text, const FFstrbuf* color)
{
    if (!instance.config.display.pipe)
    {
        if (instance.config.display.brightColor)
            ffStrbufAppendS(output, FASTFETCH_TEXT_MODIFIER_BOLT);
        if (color->length > 0)
            ffStrbufAppendF(output, "\e[%sm", color->chars);
        else if (instance.config.display.colorTitle.length > 0)
            ffStrbufAppendF(output, "\e[%sm", instance.config.display.colorTitle.chars);
    }

    ffStrbufAppend(output, text);

    if(!instance.config.display.pipe)
        ffStrbufAppendS(output, FASTFETCH_TEXT_MODIFIER_RESET);
}

bool ffPrintTitle(FFTitleOptions* options)
{
    FF_STRBUF_AUTO_DESTROY userNameColored = ffStrbufCreate();
    appendText(&userNameColored, &instance.state.platform.userName, &options->colorUser);

    FF_STRBUF_AUTO_DESTROY hostName = ffStrbufCreateCopy(&instance.state.platform.hostName);
    if (!options->fqdn)
        ffStrbufSubstrBeforeFirstC(&hostName, '.');
    instance.state.titleFqdn = options->fqdn;

    FF_STRBUF_AUTO_DESTROY hostNameColored = ffStrbufCreate();
    appendText(&hostNameColored, &hostName, &options->colorHost);

    FF_STRBUF_AUTO_DESTROY atColored = ffStrbufCreate();
    if (!instance.config.display.pipe && options->colorAt.length > 0)
    {
        ffStrbufAppendF(&atColored, "\e[%sm", options->colorAt.chars);
        ffStrbufAppendC(&atColored, '@');
        ffStrbufAppendS(&atColored, FASTFETCH_TEXT_MODIFIER_RESET);
    }
    else
        ffStrbufAppendC(&atColored, '@');

    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_TITLE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        ffStrbufWriteTo(&userNameColored, stdout);
        ffStrbufWriteTo(&atColored, stdout);
        ffStrbufPutTo(&hostNameColored, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_TITLE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(instance.state.platform.userName, "user-name"),
            FF_FORMAT_ARG(hostName, "host-name"),
            FF_FORMAT_ARG(instance.state.platform.homeDir, "home-dir"),
            FF_FORMAT_ARG(instance.state.platform.exePath, "exe-path"),
            FF_FORMAT_ARG(instance.state.platform.userShell, "user-shell"),
            FF_FORMAT_ARG(userNameColored, "user-name-colored"),
            FF_FORMAT_ARG(atColored, "at-symbol-colored"),
            FF_FORMAT_ARG(hostNameColored, "host-name-colored"),
            FF_FORMAT_ARG(instance.state.platform.fullUserName, "full-user-name"),
        }));
    }

    return true;
}

void ffParseTitleJsonObject(FFTitleOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "fqdn"))
        {
            options->fqdn = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "color"))
        {
            if (!yyjson_is_obj(val))
                continue;

            yyjson_val* color = yyjson_obj_get(val, "user");
            if (color)
                ffOptionParseColor(yyjson_get_str(color), &options->colorUser);
            color = yyjson_obj_get(val, "at");
            if (color)
                ffOptionParseColor(yyjson_get_str(color), &options->colorAt);
            color = yyjson_obj_get(val, "host");
            if (color)
                ffOptionParseColor(yyjson_get_str(color), &options->colorHost);
            continue;
        }

        ffPrintError(FF_TITLE_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateTitleJsonConfig(FFTitleOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "fqdn", options->fqdn);

    yyjson_mut_val* color = yyjson_mut_obj_add_obj(doc, module, "color");
    yyjson_mut_obj_add_strbuf(doc, color, "user", &options->colorUser);
    yyjson_mut_obj_add_strbuf(doc, color, "at", &options->colorAt);
    yyjson_mut_obj_add_strbuf(doc, color, "host", &options->colorHost);
}

bool ffGenerateTitleJsonResult(FF_MAYBE_UNUSED FFTitleOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "userName", &instance.state.platform.userName);
    yyjson_mut_obj_add_strbuf(doc, obj, "fullUserName", &instance.state.platform.fullUserName);
    yyjson_mut_obj_add_strbuf(doc, obj, "hostName", &instance.state.platform.hostName);
    yyjson_mut_obj_add_strbuf(doc, obj, "homeDir", &instance.state.platform.homeDir);
    yyjson_mut_obj_add_strbuf(doc, obj, "exePath", &instance.state.platform.exePath);
    yyjson_mut_obj_add_strbuf(doc, obj, "userShell", &instance.state.platform.userShell);

    return true;
}

void ffInitTitleOptions(FFTitleOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
    ffStrbufSetStatic(&options->moduleArgs.key, " ");

    options->fqdn = false;
    ffStrbufInit(&options->colorUser);
    ffStrbufInit(&options->colorAt);
    ffStrbufInit(&options->colorHost);
}

void ffDestroyTitleOptions(FFTitleOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->colorUser);
    ffStrbufDestroy(&options->colorAt);
    ffStrbufDestroy(&options->colorHost);
}

FFModuleBaseInfo ffTitleModuleInfo = {
    .name = FF_TITLE_MODULE_NAME,
    .description = "Print title, which contains your user name, hostname",
    .initOptions = (void*) ffInitTitleOptions,
    .destroyOptions = (void*) ffDestroyTitleOptions,
    .parseJsonObject = (void*) ffParseTitleJsonObject,
    .printModule = (void*) ffPrintTitle,
    .generateJsonResult = (void*) ffGenerateTitleJsonResult,
    .generateJsonConfig = (void*) ffGenerateTitleJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"User name", "user-name"},
        {"Host name", "host-name"},
        {"Home directory", "home-dir"},
        {"Executable path of current process", "exe-path"},
        {"User's default shell", "user-shell"},
        {"User name (colored)", "user-name-colored"},
        {"@ symbol (colored)", "at-symbol-colored"},
        {"Host name (colored)", "host-name-colored"},
        {"Full user name", "full-user-name"},
    }))
};
