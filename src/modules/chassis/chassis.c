#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/chassis/chassis.h"
#include "modules/chassis/chassis.h"

bool ffPrintChassis(FFChassisOptions* options) {
    bool success = false;

    FFChassisResult result;
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.serial);

    const char* error = ffDetectChassis(&result);

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Chassis), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if (result.type.length == 0) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Chassis), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "chassis_type is not set by O.E.M.");
        goto exit;
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Chassis), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result.type, stdout);
        if (result.version.length) {
            printf(" (%s)", result.version.chars);
        }
        putchar('\n');
    } else {
        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(Chassis), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                                                                                                            FF_ARG(result.type, "type"),
                                                                                                            FF_ARG(result.vendor, "vendor"),
                                                                                                            FF_ARG(result.version, "version"),
                                                                                                            FF_ARG(result.serial, "serial"),
                                                                                                        }));
    }
    success = true;

exit:
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.serial);
    return success;
}

void ffParseChassisJsonObject(FFChassisOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Chassis), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateChassisJsonConfig(FFChassisOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateChassisJsonResult([[maybe_unused]] FFChassisOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    bool success = false;
    FFChassisResult result;
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.serial);

    const char* error = ffDetectChassis(&result);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if (result.type.length == 0) {
        yyjson_mut_obj_add_str(doc, module, "error", "chassis_type is not set by O.E.M.");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "type", &result.type);
    yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &result.vendor);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);
    yyjson_mut_obj_add_strbuf(doc, obj, "serial", &result.serial);
    success = true;

exit:
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.serial);
    return success;
}

void ffInitChassisOptions(FFChassisOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyChassisOptions(FFChassisOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffChassisModuleInfo = {
    .name = "Chassis",
    .description = "Print chassis type information (desktop, laptop, etc.)",
    .displayName = {
        .en = "Chassis",
        .ar = "الهيكل",
        .de = "Gehäuse",
        .es = "Chasis",
        .fr = "Châssis",
        .gl = "Chasis",
        .he = "מארז",
        .it = "Telaio",
        .ja = "シャーシ",
        .ko = "섀시",
        .pl = "Obudowa",
        .pt = "Chassi",
        .ru = "Корпус",
        .zh_CN = "机箱",
        .zh_TW = "機箱",
    },
    .initOptions = (void*) ffInitChassisOptions,
    .destroyOptions = (void*) ffDestroyChassisOptions,
    .parseJsonObject = (void*) ffParseChassisJsonObject,
    .printModule = (void*) ffPrintChassis,
    .generateJsonResult = (void*) ffGenerateChassisJsonResult,
    .generateJsonConfig = (void*) ffGenerateChassisJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "Chassis type", "type" },
        { "Chassis vendor", "vendor" },
        { "Chassis version", "version" },
        { "Chassis serial number", "serial" },
    })),
    .defaultOrder = 8,
};
