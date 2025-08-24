#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalfont/terminalfont.h"
#include "modules/terminalfont/terminalfont.h"
#include "util/stringUtils.h"

#define FF_TERMINALFONT_DISPLAY_NAME "Terminal Font"

bool ffPrintTerminalFont(FFTerminalFontOptions* options)
{
    bool success = false;
    FFTerminalFontResult terminalFont;
    ffFontInit(&terminalFont.font);
    ffFontInit(&terminalFont.fallback);
    ffStrbufInit(&terminalFont.error);

    if(!ffDetectTerminalFont(&terminalFont))
    {
        ffPrintError(FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", terminalFont.error.chars);
    }
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            ffStrbufWriteTo(&terminalFont.font.pretty, stdout);
            if(terminalFont.fallback.pretty.length)
            {
                fputs(" / ", stdout);
                ffStrbufWriteTo(&terminalFont.fallback.pretty, stdout);
            }
            putchar('\n');
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_TERMINALFONT_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
                FF_FORMAT_ARG(terminalFont.font.pretty, "combined"),
                FF_FORMAT_ARG(terminalFont.font.name, "name"),
                FF_FORMAT_ARG(terminalFont.font.size, "size"),
                FF_FORMAT_ARG(terminalFont.font.styles, "styles"),
            }));
        }
        success = true;
    }

    ffStrbufDestroy(&terminalFont.error);
    ffFontDestroy(&terminalFont.font);
    ffFontDestroy(&terminalFont.fallback);

    return success;
}

void ffParseTerminalFontJsonObject(FFTerminalFontOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_TERMINALFONT_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateTerminalFontJsonConfig(FFTerminalFontOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateTerminalFontJsonResult(FF_MAYBE_UNUSED FFTerminalFontOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFTerminalFontResult result;
    ffFontInit(&result.font);
    ffFontInit(&result.fallback);
    ffStrbufInit(&result.error);

    if(!ffDetectTerminalFont(&result))
        yyjson_mut_obj_add_strbuf(doc, module, "error", &result.error);
    else
    {
        yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");

        yyjson_mut_val* font = yyjson_mut_obj_add_obj(doc, obj, "font");
        yyjson_mut_obj_add_strbuf(doc, font, "name", &result.font.name);
        yyjson_mut_obj_add_strbuf(doc, font, "size", &result.font.size);
        yyjson_mut_val* fontStyles = yyjson_mut_obj_add_arr(doc, font, "styles");
        FF_LIST_FOR_EACH(FFstrbuf, style, result.font.styles)
        {
            yyjson_mut_arr_add_strbuf(doc, fontStyles, style);
        }
        yyjson_mut_obj_add_strbuf(doc, font, "pretty", &result.font.pretty);

        yyjson_mut_val* fallback = yyjson_mut_obj_add_obj(doc, obj, "fallback");
        yyjson_mut_obj_add_strbuf(doc, fallback, "name", &result.fallback.name);
        yyjson_mut_obj_add_strbuf(doc, fallback, "size", &result.fallback.size);
        yyjson_mut_val* fallbackStyles = yyjson_mut_obj_add_arr(doc, fallback, "styles");
        FF_LIST_FOR_EACH(FFstrbuf, style, result.fallback.styles)
        {
            yyjson_mut_arr_add_strbuf(doc, fallbackStyles, style);
        }
        yyjson_mut_obj_add_strbuf(doc, fallback, "pretty", &result.fallback.pretty);
        success = true;
    }

    ffStrbufDestroy(&result.error);
    ffFontDestroy(&result.font);
    ffFontDestroy(&result.fallback);
    return success;
}

void ffInitTerminalFontOptions(FFTerminalFontOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyTerminalFontOptions(FFTerminalFontOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffTerminalFontModuleInfo = {
    .name = FF_TERMINALFONT_MODULE_NAME,
    .description = "Print font name and size used by current terminal",
    .initOptions = (void*) ffInitTerminalFontOptions,
    .destroyOptions = (void*) ffDestroyTerminalFontOptions,
    .parseJsonObject = (void*) ffParseTerminalFontJsonObject,
    .printModule = (void*) ffPrintTerminalFont,
    .generateJsonResult = (void*) ffGenerateTerminalFontJsonResult,
    .generateJsonConfig = (void*) ffGenerateTerminalFontJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Terminal font combined", "combined"},
        {"Terminal font name", "name"},
        {"Terminal font size", "size"},
        {"Terminal font styles", "styles"},
    })),
};
