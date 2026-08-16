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
        .de = "Benutzerdefiniert",
        .es = "Personalizado",
        .fr = "Personnalisé",
        .it = "Personalizzato",
        .ja = "カスタム",
        .ko = "사용자 정의",
        .pl = "Niestandardowy",
        .pt = "Personalizado",
        .ru = "Пользовательский",
        .zh_CN = "自定义",
        .zh_TW = "自訂",
    },
    .initOptions = (void*) ffInitCustomOptions,
    .destroyOptions = (void*) ffDestroyCustomOptions,
    .parseJsonObject = (void*) ffParseCustomJsonObject,
    .printModule = (void*) ffPrintCustom,
    .generateJsonConfig = (void*) ffGenerateCustomJsonConfig,
};
