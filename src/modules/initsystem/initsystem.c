#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/initsystem/initsystem.h"
#include "modules/initsystem/initsystem.h"

bool ffPrintInitSystem(FFInitSystemOptions* options) {
    bool success = false;
    FFInitSystemResult result = {
        .name = ffStrbufCreate(),
        .exe = ffStrbufCreate(),
        .version = ffStrbufCreate(),
        .pid = 1,
    };

    const char* error = ffDetectInitSystem(&result);

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(InitSystem), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(InitSystem), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result.name, stdout);
        if (result.version.length) {
            printf(" %s\n", result.version.chars);
        } else {
            putchar('\n');
        }
    } else {
        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(InitSystem), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                                                                                                                FF_ARG(result.name, "name"),
                                                                                                                FF_ARG(result.exe, "exe"),
                                                                                                                FF_ARG(result.version, "version"),
                                                                                                                FF_ARG(result.pid, "pid"),
                                                                                                            }));
    }
    success = true;

exit:
    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.exe);
    ffStrbufDestroy(&result.version);

    return success;
}

void ffParseInitSystemJsonObject(FFInitSystemOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(InitSystem), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateInitSystemJsonConfig(FFInitSystemOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateInitSystemJsonResult([[maybe_unused]] FFInitSystemOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    bool success = false;
    FFInitSystemResult result = {
        .name = ffStrbufCreate(),
        .exe = ffStrbufCreate(),
        .version = ffStrbufCreate(),
        .pid = 1,
    };

    const char* error = ffDetectInitSystem(&result);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &result.name);
    yyjson_mut_obj_add_strbuf(doc, obj, "exe", &result.exe);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);
    yyjson_mut_obj_add_uint(doc, obj, "pid", result.pid);
    success = true;

exit:
    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.exe);
    ffStrbufDestroy(&result.version);
    return success;
}

void ffInitInitSystemOptions(FFInitSystemOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰿄");
}

void ffDestroyInitSystemOptions(FFInitSystemOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffInitSystemModuleInfo = {
    .name = "InitSystem",
    .description = "Print init system (pid 1) name and version",
    .displayName = {
        .en = "Init System",
        .ar = "نظام الإقلاع",
        .cs = "Init systém",
        .de = "Init-System",
        .es = "Sistema de inicio",
        .fr = "Système d'init",
        .gl = "Sistema de inicio",
        .he = "מערכת אתחול",
        .id = "Sistem Init",
        .it = "Sistema di init",
        .ja = "initシステム",
        .ko = "초기화 시스템",
        .pl = "System init",
        .pt = "Sistema de inicialização",
        .ru = "Система инициализации",
        .tr = "Init Sistemi",
        .uk = "Init-система",
        .vi = "Hệ thống init",
        .zh_CN = "Init 系统",
        .zh_TW = "Init 系統",
    },
    .initOptions = (void*) ffInitInitSystemOptions,
    .destroyOptions = (void*) ffDestroyInitSystemOptions,
    .parseJsonObject = (void*) ffParseInitSystemJsonObject,
    .printModule = (void*) ffPrintInitSystem,
    .generateJsonResult = (void*) ffGenerateInitSystemJsonResult,
    .generateJsonConfig = (void*) ffGenerateInitSystemJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "Init system name", "name" },
        { "Init system exe path", "exe" },
        { "Init system version path", "version" },
        { "Init system pid", "pid" },
    })),
    .defaultOrder = 10,
};
