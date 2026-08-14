#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/bootmgr/bootmgr.h"
#include "modules/bootmgr/bootmgr.h"

bool ffPrintBootmgr(FFBootmgrOptions* options) {
    bool success = false;
    FFBootmgrResult bootmgr = {
        .name = ffStrbufCreate(),
        .firmware = ffStrbufCreate(),
    };

    const char* error = ffDetectBootmgr(&bootmgr);

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Bootmgr), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    } else {
        FF_STRBUF_AUTO_DESTROY firmwareName = ffStrbufCreateCopy(&bootmgr.firmware);
#ifndef __APPLE__
        ffStrbufSubstrAfterLastC(&firmwareName, '\\');
#else
        ffStrbufSubstrAfterLastC(&firmwareName, '/');
#endif

        if (options->moduleArgs.outputFormat.length == 0) {
            ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Bootmgr), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            ffStrbufWriteTo(&bootmgr.name, stdout);
            if (firmwareName.length > 0) {
                printf(" - %s\n", firmwareName.chars);
            } else {
                putchar('\n');
            }
        } else {
            FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(Bootmgr), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                                                                                                                FF_ARG(bootmgr.name, "name"),
                                                                                                                FF_ARG(bootmgr.firmware, "firmware-path"),
                                                                                                                FF_ARG(firmwareName, "firmware-name"),
                                                                                                                FF_ARG(bootmgr.secureBoot, "secure-boot"),
                                                                                                                FF_ARG(bootmgr.order, "order"),
                                                                                                            }));
        }
    }
    success = true;

exit:
    ffStrbufDestroy(&bootmgr.name);
    ffStrbufDestroy(&bootmgr.firmware);

    return success;
}

void ffParseBootmgrJsonObject(FFBootmgrOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Bootmgr), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateBootmgrJsonConfig(FFBootmgrOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateBootmgrJsonResult([[maybe_unused]] FFBootmgrOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    bool success = false;
    FFBootmgrResult bootmgr = {
        .name = ffStrbufCreate(),
        .firmware = ffStrbufCreate(),
    };

    const char* error = ffDetectBootmgr(&bootmgr);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &bootmgr.name);
    yyjson_mut_obj_add_strbuf(doc, obj, "firmware", &bootmgr.firmware);
    yyjson_mut_obj_add_uint(doc, obj, "order", bootmgr.order);
    yyjson_mut_obj_add_bool(doc, obj, "secureBoot", bootmgr.secureBoot);
    success = true;

exit:
    ffStrbufDestroy(&bootmgr.name);
    ffStrbufDestroy(&bootmgr.firmware);
    return success;
}

void ffInitBootmgrOptions(FFBootmgrOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyBootmgrOptions(FFBootmgrOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffBootmgrModuleInfo = {
    .name = "Bootmgr",
    .description = "Print second-stage bootloader information (name, firmware, etc.)",
    .displayName = {
        .en = "Boot Manager",
        .de = "Boot-Manager",
        .es = "Administrador de arranque",
        .fr = "Gestionnaire de démarrage",
        .it = "Gestore di avvio",
        .ja = "ブートマネージャー",
        .ko = "부트 매니저",
        .pl = "Menedżer rozruchu",
        .pt_BR = "Gerenciador de inicialização",
        .ru = "Менеджер загрузки",
        .zh_CN = "启动管理器",
        .zh_TW = "啟動管理器",
    },
    .initOptions = (void*) ffInitBootmgrOptions,
    .destroyOptions = (void*) ffDestroyBootmgrOptions,
    .parseJsonObject = (void*) ffParseBootmgrJsonObject,
    .printModule = (void*) ffPrintBootmgr,
    .generateJsonResult = (void*) ffGenerateBootmgrJsonResult,
    .generateJsonConfig = (void*) ffGenerateBootmgrJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "Name / description", "name" },
        { "Firmware file path", "firmware-path" },
        { "Firmware file name", "firmware-name" },
        { "Is secure boot enabled", "secure-boot" },
        { "Boot order", "order" },
    })),
    .defaultOrder = 6,
};
