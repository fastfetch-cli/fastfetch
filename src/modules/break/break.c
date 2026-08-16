#include "common/printing.h"
#include "logo/logo.h"
#include "modules/break/break.h"

bool ffPrintBreak([[maybe_unused]] FFBreakOptions* options) {
    ffLogoPrintLine();
    putchar('\n');
    return true;
}

void ffParseBreakJsonObject([[maybe_unused]] FFBreakOptions* options, [[maybe_unused]] yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (unsafe_yyjson_equals_str(key, "type") || unsafe_yyjson_equals_str(key, "condition")) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Break), 0, nullptr, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffInitBreakOptions([[maybe_unused]] FFBreakOptions* options) {
}

void ffDestroyBreakOptions([[maybe_unused]] FFBreakOptions* options) {
}

FFModuleBaseInfo ffBreakModuleInfo = {
    .name = "Break",
    .description = "Print an empty line",
    .displayName = {
        .en = "Break",
        .ar = "فاصل",
        .de = "Umbruch",
        .es = "Salto de línea",
        .fr = "Saut de ligne",
        .he = "מעבר שורה",
        .it = "Interruzione",
        .ja = "改行",
        .ko = "줄 바꿈",
        .pl = "Przerwa",
        .pt = "Quebra",
        .ru = "Разрыв",
        .zh_CN = "换行",
        .zh_TW = "換行",
    },
    .initOptions = (void*) ffInitBreakOptions,
    .destroyOptions = (void*) ffDestroyBreakOptions,
    .parseJsonObject = (void*) ffParseBreakJsonObject,
    .printModule = (void*) ffPrintBreak,
    .defaultOrder = 71,
};
