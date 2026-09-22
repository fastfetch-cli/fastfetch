#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/textModifier.h"
#include "common/strutil.h"
#include "modules/custom/custom.h"

bool ffPrintCustom(FFCustomOptions* options) {
    ffPrintFormat(FF_MODULE_GET_DISPLAY_NAME(Custom), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, 0, ((FFformatarg[]) {}));
    return true;
}

void ffGenerateCustomJsonConfig(FFCustomOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateCustomJsonResult(FFCustomOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();

    // This module's output *is* its format string, so the JSON result is that string rendered —
    // for a `qjs:`/`lua:` format that means the script's output, not the script. `ffPrintCustom()`
    // gets the same value through `ffPrintFormat()`. Without this the module had no JSON result at
    // all, so a script consuming `--format json` could not read a custom line's value.
    if (!ffParseFormatString(&result, &options->moduleArgs.outputFormat, 0, (FFformatarg[]) {})) {
        // `yyjson_mut_obj_add_str()` only copies strings that need escaping; for anything else it
        // stores the pointer as-is. `result` is freed when this function returns, long before the
        // document is serialised, so the error has to be copied into the document explicitly.
        yyjson_mut_obj_add_strcpy(doc, module, "error", result.chars);
        return false;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &result);
    return true;
}

void ffParseCustomJsonObject(FFCustomOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Custom), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffInitCustomOptions(FFCustomOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "");
    ffStrbufSetStatic(&options->moduleArgs.key, " ");
}

void ffDestroyCustomOptions(FFCustomOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffCustomModuleInfo = {
    .name = "Custom",
    .description = "Print a custom string, with or without key",
    .displayName = {
        .en = "Custom",
        .ar = "مخصص",
        .cs = "Vlastní",
        .de = "Benutzerdefiniert",
        .es = "Personalizado",
        .fr = "Personnalisé",
        .gl = "Personalizado",
        .he = "מותאם אישית",
        .id = "Kustom",
        .it = "Personalizzato",
        .ja = "カスタム",
        .ko = "사용자 정의",
        .pl = "Niestandardowy",
        .pt = "Personalizado",
        .ru = "Пользовательский",
        .tr = "Özel",
        .uk = "Власний",
        .vi = "Tùy chỉnh",
        .zh_CN = "自定义",
        .zh_TW = "自訂",
    },
    .initOptions = (void*) ffInitCustomOptions,
    .destroyOptions = (void*) ffDestroyCustomOptions,
    .parseJsonObject = (void*) ffParseCustomJsonObject,
    .printModule = (void*) ffPrintCustom,
    .generateJsonResult = (void*) ffGenerateCustomJsonResult,
    .generateJsonConfig = (void*) ffGenerateCustomJsonConfig,
};
