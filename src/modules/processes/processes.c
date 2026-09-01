#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/processes/processes.h"
#include "modules/processes/processes.h"

bool ffPrintProcesses(FFProcessesOptions* options) {
    FFProcessesResult result = {};
    const char* error = ffDetectProcesses(options, &result);

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Processes), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Processes), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        printf("%u (%u threads)\n", result.processes, result.threads);
    } else {
        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(Processes), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) { FF_ARG(result.processes, "result") }));
    }

    return true;
}

void ffParseProcessesJsonObject(FFProcessesOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "countKprocs")) {
            options->countKprocs = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Processes), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateProcessesJsonConfig(FFProcessesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
    yyjson_mut_obj_add_bool(doc, module, "countKprocs", options->countKprocs);
}

bool ffGenerateProcessesJsonResult([[maybe_unused]] FFProcessesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FFProcessesResult result = {};
    const char* error = ffDetectProcesses(options, &result);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_uint(doc, obj, "processes", result.processes);
    yyjson_mut_obj_add_uint(doc, obj, "threads", result.threads);

    return true;
}

void ffInitProcessesOptions(FFProcessesOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->countKprocs = false;
}

void ffDestroyProcessesOptions(FFProcessesOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffProcessesModuleInfo = {
    .name = "Processes",
    .description = "Print number of running processes and threads",
    .displayName = {
        .en = "Processes",
        .ar = "العمليات",
        .cs = "Procesy",
        .de = "Prozesse",
        .es = "Procesos",
        .fr = "Processus",
        .gl = "Procesos",
        .he = "תהליכים",
        .id = "Proses",
        .it = "Processi",
        .ja = "プロセス",
        .ko = "프로세스",
        .pl = "Procesy",
        .pt = "Processos",
        .ru = "Процессы",
        .tr = "Süreçler",
        .uk = "Процеси",
        .vi = "Tiến trình",
        .zh_CN = "进程数",
        .zh_TW = "進程數",
    },
    .initOptions = (void*) ffInitProcessesOptions,
    .destroyOptions = (void*) ffDestroyProcessesOptions,
    .parseJsonObject = (void*) ffParseProcessesJsonObject,
    .printModule = (void*) ffPrintProcesses,
    .generateJsonResult = (void*) ffGenerateProcessesJsonResult,
    .generateJsonConfig = (void*) ffGenerateProcessesJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "Process count", "result" },
    })),
    .defaultOrder = 13,
};
