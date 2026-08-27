#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/cursor/cursor.h"
#include "modules/cursor/cursor.h"

bool ffPrintCursor(FFCursorOptions* options) {
    bool success = false;
    FFCursorResult result;
    ffStrbufInit(&result.error);
    ffStrbufInit(&result.theme);
    ffStrbufInit(&result.size);

    ffDetectCursor(&result);

    if (result.error.length) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Cursor), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", result.error.chars);
    } else {
        ffStrbufRemoveIgnCaseEndS(&result.theme, "cursors");
        ffStrbufRemoveIgnCaseEndS(&result.theme, "cursor");
        ffStrbufTrimRight(&result.theme, '_');
        ffStrbufTrimRight(&result.theme, '-');
        if (result.theme.length == 0) {
            ffStrbufAppendS(&result.theme, "default");
        }

        if (options->moduleArgs.outputFormat.length == 0) {
            ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Cursor), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            ffStrbufWriteTo(&result.theme, stdout);

            if (result.size.length > 0 && !ffStrbufEqualS(&result.size, "0")) {
                printf(" (%spx)", result.size.chars);
            }

            putchar('\n');
        } else {
            FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(Cursor), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                                                                                                               FF_ARG(result.theme, "theme"),
                                                                                                               FF_ARG(result.size, "size"),
                                                                                                           }));
        }

        success = true;
    }

    ffStrbufDestroy(&result.error);
    ffStrbufDestroy(&result.theme);
    ffStrbufDestroy(&result.size);

    return success;
}

void ffParseCursorJsonObject(FFCursorOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Cursor), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateCursorJsonConfig(FFCursorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateCursorJsonResult([[maybe_unused]] FFCursorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    bool success = false;
    FFCursorResult result;
    ffStrbufInit(&result.error);
    ffStrbufInit(&result.theme);
    ffStrbufInit(&result.size);

    ffDetectCursor(&result);

    if (result.error.length) {
        yyjson_mut_obj_add_strbuf(doc, module, "error", &result.error);
    } else {
        yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
        yyjson_mut_obj_add_strbuf(doc, obj, "theme", &result.theme);
        yyjson_mut_obj_add_strbuf(doc, obj, "size", &result.size);
        success = true;
    }

    ffStrbufDestroy(&result.error);
    ffStrbufDestroy(&result.theme);
    ffStrbufDestroy(&result.size);

    return success;
}

void ffInitCursorOptions(FFCursorOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰆿");
}

void ffDestroyCursorOptions(FFCursorOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffCursorModuleInfo = {
    .name = "Cursor",
    .description = "Print cursor style name",
    .displayName = {
        .en = "Cursor",
        .ar = "المؤشر",
        .cs = "Kurzor",
        .de = "Mauszeiger",
        .es = "Cursor",
        .fr = "Curseur",
        .gl = "Cursor",
        .he = "סמן",
        .id = "Kursor",
        .it = "Cursore",
        .ja = "カーソル",
        .ko = "커서",
        .pl = "Kursor",
        .pt = "Cursor",
        .ru = "Курсор",
        .tr = "İmleç",
        .uk = "Курсор",
        .vi = "Con trỏ",
        .zh_CN = "光标",
        .zh_TW = "游標",
    },
    .initOptions = (void*) ffInitCursorOptions,
    .destroyOptions = (void*) ffDestroyCursorOptions,
    .parseJsonObject = (void*) ffParseCursorJsonObject,
    .printModule = (void*) ffPrintCursor,
    .generateJsonResult = (void*) ffGenerateCursorJsonResult,
    .generateJsonConfig = (void*) ffGenerateCursorJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "Cursor theme", "theme" },
        { "Cursor size", "size" },
    })),
    .defaultOrder = 27,
};
