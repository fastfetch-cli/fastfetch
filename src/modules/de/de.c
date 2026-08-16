#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/displayserver/displayserver.h"
#include "detection/de/de.h"
#include "modules/de/de.h"

bool ffPrintDE(FFDEOptions* options) {
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if (result->dePrettyName.length == 0) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(DE), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No DE found");
        return false;
    }

    FF_STRBUF_AUTO_DESTROY version = ffStrbufCreate();
    ffDetectDEVersion(&result->dePrettyName, &version, options);

    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(DE), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        ffStrbufWriteTo(&result->dePrettyName, stdout);

        if (version.length > 0) {
            putchar(' ');
            ffStrbufWriteTo(&version, stdout);
        }

        putchar('\n');
    } else {
        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(DE), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) { FF_ARG(result->deProcessName, "process-name"), FF_ARG(result->dePrettyName, "pretty-name"), FF_ARG(version, "version") }));
    }

    return true;
}

void ffParseDEJsonObject(FFDEOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "slowVersionDetection")) {
            ffPrintError(FF_MODULE_GET_DISPLAY_NAME(DE), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Key `slowVersionDetection` is deprecated, it's always true");
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(DE), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateDEJsonConfig(FFDEOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateDEJsonResult([[maybe_unused]] FFDEOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if (result->dePrettyName.length == 0) {
        yyjson_mut_obj_add_str(doc, module, "error", "No DE found");
        return false;
    }

    FF_STRBUF_AUTO_DESTROY version = ffStrbufCreate();
    ffDetectDEVersion(&result->dePrettyName, &version, options);

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->deProcessName);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->dePrettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &version);
    return true;
}

void ffInitDEOptions(FFDEOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyDEOptions(FFDEOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffDEModuleInfo = {
    .name = "DE",
    .description = "Print desktop environment name",
    .displayName = {
        .en = "Desktop Environment",
        .ar = "بيئة سطح المكتب",
        .de = "Desktop-Umgebung",
        .es = "Entorno de Escritorio",
        .fr = "Environnement de Bureau",
        .he = "סביבת שולחן עבודה",
        .it = "Ambiente Desktop",
        .ja = "デスクトップ環境",
        .ko = "데스크탑 환경",
        .pl = "Środowisko graficzne",
        .pt = "Ambiente de desktop",
        .ru = "Рабочее окружение",
        .zh_CN = "桌面环境",
        .zh_TW = "桌面環境",
    },
    .initOptions = (void*) ffInitDEOptions,
    .destroyOptions = (void*) ffDestroyDEOptions,
    .parseJsonObject = (void*) ffParseDEJsonObject,
    .printModule = (void*) ffPrintDE,
    .generateJsonResult = (void*) ffGenerateDEJsonResult,
    .generateJsonConfig = (void*) ffGenerateDEJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "DE process name", "process-name" },
        { "DE pretty name", "pretty-name" },
        { "DE version", "version" },
    })),
    .defaultOrder = 21,
};
