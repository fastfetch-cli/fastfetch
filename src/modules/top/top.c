#include "common/jsonconfig.h"
#include "common/percent.h"
#include "common/printing.h"
#include "common/size.h"
#include "detection/top/top.h"
#include "modules/top/top.h"

static void printTopResult(FFTopOptions* options, uint32_t index, uint32_t total, FFTopProcessResult* process) {
    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;
    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Top), total == 1 ? 0 : (uint8_t) (index + 1), &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();
        ffStrbufAppendF(&output, "%s (%u)", process->name.chars, process->pid);
        if ((options->showTypes & FF_TOP_TYPE_CPU) && (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)) {
            ffStrbufAppendS(&output, " - CPU ");
            ffPercentAppendNum(&output, process->cpuPercent, options->percent, false, &options->moduleArgs);
        }
        if (!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT)) {
            if (options->showTypes & FF_TOP_TYPE_MEMORY) {
                ffStrbufAppendS(&output, " - MEM ");
                ffSizeAppendNum(process->memBytes, &output);
            }
            if (options->showTypes & FF_TOP_TYPE_DISK) {
                ffStrbufAppendS(&output, " - DSK ");
                ffSizeAppendNum(process->bytesRead, &output);
                ffStrbufAppendS(&output, "/s / ");
                ffSizeAppendNum(process->bytesWritten, &output);
                ffStrbufAppendS(&output, "/s");
            }
        }
        ffStrbufPutTo(&output, stdout);
    } else {
        FF_STRBUF_AUTO_DESTROY cpuFormatted = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT) {
            ffPercentAppendNum(&cpuFormatted, process->cpuPercent, options->percent, false, &options->moduleArgs);
        }
        FF_STRBUF_AUTO_DESTROY memFormatted = ffStrbufCreate();
        ffSizeAppendNum(process->memBytes, &memFormatted);
        FF_STRBUF_AUTO_DESTROY diskReadFormatted = ffStrbufCreate();
        ffSizeAppendNum(process->bytesRead, &diskReadFormatted);
        ffStrbufAppendS(&diskReadFormatted, "/s");
        FF_STRBUF_AUTO_DESTROY diskWriteFormatted = ffStrbufCreate();
        ffSizeAppendNum(process->bytesWritten, &diskWriteFormatted);
        ffStrbufAppendS(&diskWriteFormatted, "/s");
        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(Top), total == 1 ? 0 : (uint8_t) (index + 1), &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
                                                                                                                                   FF_ARG(process->name, "name"),
                                                                                                                                   FF_ARG(process->pid, "pid"),
                                                                                                                                   FF_ARG(process->cpuPercent, "cpu"),
                                                                                                                                   FF_ARG(process->memBytes, "mem"),
                                                                                                                                   FF_ARG(process->bytesRead, "disk-read"),
                                                                                                                                   FF_ARG(process->bytesWritten, "disk-write"),
                                                                                                                                   FF_ARG(cpuFormatted, "cpu-percentage"),
                                                                                                                                   FF_ARG(memFormatted, "mem-formatted"),
                                                                                                                                   FF_ARG(diskReadFormatted, "disk-read-formatted"),
                                                                                                                                   FF_ARG(diskWriteFormatted, "disk-write-formatted"),
                                                                                                                               }));
    }
}

bool ffPrintTop(FFTopOptions* options) {
    FF_LIST_AUTO_DESTROY results = ffListCreate();
    const char* error = ffDetectTopProcesses(options, &results);
    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Top), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (!options->compact) {
        FF_LIST_FOR_EACH (FFTopProcessResult, process, results) {
            printTopResult(options, (uint32_t) (process - (FFTopProcessResult*) results.data), results.length, process);
        }
    } else {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Top), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        FF_LIST_FOR_EACH (FFTopProcessResult, process, results) {
            if ((void*) process != results.data) {
                putchar(' ');
            }
            ffStrbufWriteTo(&process->name, stdout);
        }
        putchar('\n');
    }

    FF_LIST_FOR_EACH (FFTopProcessResult, item, results) {
        ffStrbufDestroy(&item->name);
    }
    return true;
}

void ffParseTopJsonObject(FFTopOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }
        if (unsafe_yyjson_equals_str(key, "sort")) {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]){
                                                                       { "cpu", FF_TOP_TYPE_CPU },
                                                                       { "memory", FF_TOP_TYPE_MEMORY },
                                                                       { "disk-read", FF_TOP_TYPE_DISK_READ },
                                                                       { "disk-write", FF_TOP_TYPE_DISK_WRITE },
                                                                       {},
                                                                   });
            if (error) {
                ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Top), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
            } else {
                options->sort = (FFTopTypes) value;
            }
            continue;
        } else if (unsafe_yyjson_equals_str(key, "showTypes")) {
            if (yyjson_is_arr(val)) {
                options->showTypes = 0;
                yyjson_val* item;
                size_t aidx, amax;
                yyjson_arr_foreach (val, aidx, amax, item) {
                    int value;
                    const char* error = ffJsonConfigParseEnum(item, &value, (FFKeyValuePair[]){
                                                                                { "cpu", FF_TOP_TYPE_CPU },
                                                                                { "memory", FF_TOP_TYPE_MEMORY },
                                                                                { "disk", FF_TOP_TYPE_DISK },
                                                                                {},
                                                                            });
                    if (error == nullptr) {
                        options->showTypes |= (FFTopTypes) value;
                    }
                    if (error) {
                        options->showTypes = FF_TOP_TYPE_CPU | FF_TOP_TYPE_MEMORY | FF_TOP_TYPE_DISK;
                        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Top), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
                        break;
                    }
                }
            } else {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]){
                                                                           { "cpu", FF_TOP_TYPE_CPU },
                                                                           { "memory", FF_TOP_TYPE_MEMORY },
                                                                           { "disk", FF_TOP_TYPE_DISK },
                                                                           {},
                                                                       });
                if (error == nullptr) {
                    options->showTypes = (FFTopTypes) value;
                }
                if (error) {
                    ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Top), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
                }
            }
            continue;
        } else if (unsafe_yyjson_equals_str(key, "processes")) {
            options->nProcesses = (uint32_t) yyjson_get_uint(val);
        } else if (unsafe_yyjson_equals_str(key, "waitTime")) {
            options->waitTime = (uint32_t) yyjson_get_uint(val);
        } else if (unsafe_yyjson_equals_str(key, "compact")) {
            options->compact = yyjson_get_bool(val);
        } else if (ffPercentParseJsonObject(key, val, &options->percent)) {
            continue;
        } else {
            ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Top), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
        }
    }
}

void ffGenerateTopJsonConfig(FFTopOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
    yyjson_mut_obj_add_str(doc, module, "type", "top");
    yyjson_mut_val* showTypes = yyjson_mut_arr(doc);
    if (options->showTypes & FF_TOP_TYPE_CPU) {
        yyjson_mut_arr_add_str(doc, showTypes, "cpu");
    }
    if (options->showTypes & FF_TOP_TYPE_MEMORY) {
        yyjson_mut_arr_add_str(doc, showTypes, "memory");
    }
    if (options->showTypes & FF_TOP_TYPE_DISK) {
        yyjson_mut_arr_add_str(doc, showTypes, "disk");
    }
    yyjson_mut_obj_add_val(doc, module, "showTypes", showTypes);
    if (options->sort == FF_TOP_TYPE_CPU) {
        yyjson_mut_obj_add_str(doc, module, "sort", "cpu");
    } else if (options->sort == FF_TOP_TYPE_MEMORY) {
        yyjson_mut_obj_add_str(doc, module, "sort", "memory");
    } else if (options->sort == FF_TOP_TYPE_DISK_READ) {
        yyjson_mut_obj_add_str(doc, module, "sort", "disk-read");
    } else if (options->sort == FF_TOP_TYPE_DISK_WRITE) {
        yyjson_mut_obj_add_str(doc, module, "sort", "disk-write");
    }
    yyjson_mut_obj_add_uint(doc, module, "processes", options->nProcesses);
    yyjson_mut_obj_add_uint(doc, module, "waitTime", options->waitTime);
    yyjson_mut_obj_add_bool(doc, module, "compact", options->compact);
    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateTopJsonResult(FFTopOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FF_LIST_AUTO_DESTROY results = ffListCreate();
    const char* error = ffDetectTopProcesses(options, &results);
    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }
    yyjson_mut_val* array = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH (FFTopProcessResult, process, results) {
        yyjson_mut_val* item = yyjson_mut_arr_add_obj(doc, array);
        yyjson_mut_obj_add_strbuf(doc, item, "name", &process->name);
        yyjson_mut_obj_add_uint(doc, item, "pid", process->pid);
        if (options->showTypes & FF_TOP_TYPE_CPU) {
            yyjson_mut_obj_add_real(doc, item, "cpuPercent", process->cpuPercent);
        }
        if (options->showTypes & FF_TOP_TYPE_MEMORY) {
            yyjson_mut_obj_add_uint(doc, item, "mem", process->memBytes);
        }
        if (options->showTypes & FF_TOP_TYPE_DISK) {
            yyjson_mut_obj_add_uint(doc, item, "bytesRead", process->bytesRead);
            yyjson_mut_obj_add_uint(doc, item, "bytesWritten", process->bytesWritten);
        }
        yyjson_mut_obj_add_uint(doc, item, "startTime", process->startTime);
    }

    FF_LIST_FOR_EACH (FFTopProcessResult, item, results) {
        ffStrbufDestroy(&item->name);
    }
    return true;
}

void ffInitTopOptions(FFTopOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰍛");
    options->sort = FF_TOP_TYPE_CPU;
    options->showTypes = FF_TOP_TYPE_CPU | FF_TOP_TYPE_MEMORY
    #if !__GNU__ && !__HAIKU__
        | FF_TOP_TYPE_DISK
    #endif
    ;
    options->nProcesses = 5;
    options->waitTime = 500;
    options->compact = false;
    options->percent = (FFPercentageModuleConfig){ 50, 80, 0 };
}

void ffDestroyTopOptions(FFTopOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffTopModuleInfo = {
    .name = "Top",
    .description = "Print processes with the highest CPU, memory or disk I/O usage",
    .displayName = {
        .en = "Top Processes",
        .ar = "أعلى العمليات",
        .cs = "Nejvytíženější procesy",
        .de = "Top-Prozesse",
        .es = "Procesos principales",
        .fr = "Processus principaux",
        .gl = "Procesos principais",
        .he = "התהליכים המובילים",
        .id = "Proses teratas",
        .it = "Processi principali",
        .ja = "上位プロセス",
        .ko = "상위 프로세스",
        .pl = "Najaktywniejsze procesy",
        .pt = "Processos principais",
        .ru = "Топ процессов",
        .tr = "En yoğun işlemler",
        .uk = "Найактивніші процеси",
        .vi = "Tiến trình hàng đầu",
        .zh_CN = "进程排行",
        .zh_TW = "進程排行",
    },
    .initOptions = (void*) ffInitTopOptions,
    .destroyOptions = (void*) ffDestroyTopOptions,
    .parseJsonObject = (void*) ffParseTopJsonObject,
    .printModule = (void*) ffPrintTop,
    .generateJsonResult = (void*) ffGenerateTopJsonResult,
    .generateJsonConfig = (void*) ffGenerateTopJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]){
        { "Process name", "name" },
        { "Process ID", "pid" },
        { "CPU usage", "cpu" },
        { "Memory usage (RSS) in bytes", "mem" },
        { "Disk read bytes per second (0 if unsupported)", "disk-read" },
        { "Disk write bytes per second (0 if unsupported)", "disk-write" },
        { "CPU usage percentage", "cpu-percentage" },
        { "Memory usage (RSS) formatted", "mem-formatted" },
        { "Disk read formatted", "disk-read-formatted" },
        { "Disk write formatted", "disk-write-formatted" },
    })),
    .defaultOrder = 36,
};
