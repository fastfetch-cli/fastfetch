#include "common/printing.h"
#include "logo/logo.h"
#include "modules/logo/logo.h"
#include "options/logo.h"

bool ffPrintLogo(FF_MAYBE_UNUSED FFLogoOptions* options)
{
    ffPrintError(FF_LOGO_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_DEFAULT, "Supported in JSON format only");
    return false;
}

void ffParseLogoJsonObject(FF_MAYBE_UNUSED FFLogoOptions* options, FF_MAYBE_UNUSED yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (unsafe_yyjson_equals_str(key, "type") || unsafe_yyjson_equals_str(key, "condition"))
            continue;

        ffPrintError(FF_LOGO_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

bool ffGenerateLogoJsonResult(FF_MAYBE_UNUSED FFLogoOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFLogoSize size = FF_LOGO_SIZE_UNKNOWN;
    FFOptionsLogo* logoOptions = &instance.config.logo;
    if (logoOptions->type == FF_LOGO_TYPE_SMALL)
        size = FF_LOGO_SIZE_SMALL;
    else if (logoOptions->type != FF_LOGO_TYPE_BUILTIN && logoOptions->type != FF_LOGO_TYPE_AUTO)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Only 'builtin' and 'small' logo types are supported");
        return false;
    }

    const FFlogo* logo = logoOptions->source.length > 0
        ? ffLogoGetBuiltinForName(&logoOptions->source, size)
        : ffLogoGetBuiltinDetected(size);

    if (!logo)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No built-in logo found for the specified name/size");
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");

    yyjson_mut_obj_add_str(doc, obj, "lines", logo->lines);

    yyjson_mut_val* namesArr = yyjson_mut_obj_add_arr(doc, obj, "names");
    for (size_t i = 0; i < FASTFETCH_LOGO_MAX_NAMES && logo->names[i]; i++)
        yyjson_mut_arr_add_str(doc, namesArr, logo->names[i]);

    yyjson_mut_val* colorsArr = yyjson_mut_obj_add_arr(doc, obj, "colors");
    for (size_t i = 0; i < FASTFETCH_LOGO_MAX_COLORS && logo->colors[i]; i++)
        yyjson_mut_arr_add_str(doc, colorsArr, logo->colors[i]);

    yyjson_mut_obj_add_str(doc, obj, "colorKeys", logo->colorKeys);
    yyjson_mut_obj_add_str(doc, obj, "colorTitle", logo->colorTitle);

    yyjson_mut_val* typeArr = yyjson_mut_obj_add_arr(doc, obj, "type");
    if (logo->type & FF_LOGO_LINE_TYPE_NORMAL)
        yyjson_mut_arr_add_str(doc, typeArr, "normal");
    if (logo->type & FF_LOGO_LINE_TYPE_SMALL_BIT)
        yyjson_mut_arr_add_str(doc, typeArr, "small");
    if (logo->type & FF_LOGO_LINE_TYPE_ALTER_BIT)
        yyjson_mut_arr_add_str(doc, typeArr, "alter");

    return true;
}

void ffInitLogoOptions(FF_MAYBE_UNUSED FFLogoOptions* options)
{
}

void ffDestroyLogoOptions(FF_MAYBE_UNUSED FFLogoOptions* options)
{
}

FFModuleBaseInfo ffLogoModuleInfo = {
    .name = FF_LOGO_MODULE_NAME,
    .description = "Query built-in logo for JSON output",
    .initOptions = (void*) ffInitLogoOptions,
    .destroyOptions = (void*) ffDestroyLogoOptions,
    .parseJsonObject = (void*) ffParseLogoJsonObject,
    .printModule = (void*) ffPrintLogo,
    .generateJsonResult = (void*) ffGenerateLogoJsonResult,
};
