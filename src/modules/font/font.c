#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/font/font.h"
#include "modules/font/font.h"
#include "util/stringUtils.h"

bool ffPrintFont(FFFontOptions* options)
{
    bool success = false;
    FFFontResult font;
    for(uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
        ffStrbufInit(&font.fonts[i]);
    ffStrbufInit(&font.display);

    const char* error = ffDetectFont(&font);

    if(error)
    {
        ffPrintError(FF_FONT_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
    }
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_FONT_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            ffStrbufPutTo(&font.display, stdout);
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_FONT_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                FF_FORMAT_ARG(font.fonts[0], "font1"),
                FF_FORMAT_ARG(font.fonts[1], "font2"),
                FF_FORMAT_ARG(font.fonts[2], "font3"),
                FF_FORMAT_ARG(font.fonts[3], "font4"),
                FF_FORMAT_ARG(font.display, "combined"),
            }));
        }

        success = true;
    }

    ffStrbufDestroy(&font.display);
    for (uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
        ffStrbufDestroy(&font.fonts[i]);

    return success;
}

void ffParseFontJsonObject(FFFontOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_FONT_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateFontJsonConfig(FFFontOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateFontJsonResult(FF_MAYBE_UNUSED FFFontOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFFontResult font;
    for(uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
        ffStrbufInit(&font.fonts[i]);
    ffStrbufInit(&font.display);

    const char* error = ffDetectFont(&font);
    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
    }
    else
    {
        yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
        yyjson_mut_obj_add_strbuf(doc, obj, "display", &font.display);
        yyjson_mut_val* fontsArr = yyjson_mut_obj_add_arr(doc, obj, "fonts");
        for (uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
            yyjson_mut_arr_add_strbuf(doc, fontsArr, &font.fonts[i]);
        success = true;
    }

    ffStrbufDestroy(&font.display);
    for (uint32_t i = 0; i < FF_DETECT_FONT_NUM_FONTS; ++i)
        ffStrbufDestroy(&font.fonts[i]);

    return success;
}

void ffInitFontOptions(FFFontOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyFontOptions(FFFontOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffFontModuleInfo = {
    .name = FF_FONT_MODULE_NAME,
    .description = "Print system font names",
    .initOptions = (void*) ffInitFontOptions,
    .destroyOptions = (void*) ffDestroyFontOptions,
    .parseJsonObject = (void*) ffParseFontJsonObject,
    .printModule = (void*) ffPrintFont,
    .generateJsonResult = (void*) ffGenerateFontJsonResult,
    .generateJsonConfig = (void*) ffGenerateFontJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Font 1", "font1"},
        {"Font 2", "font2"},
        {"Font 3", "font3"},
        {"Font 4", "font4"},
        {"Combined fonts for display", "combined"},
    }))
};
