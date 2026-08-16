#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/libc/libc.h"
#include "detection/editor/editor.h"
#include "modules/editor/editor.h"

bool ffPrintEditor(FFEditorOptions* options) {
    FFEditorResult result = {
        .type = "Unknown",
        .name = ffStrbufCreate(),
        .path = ffStrbufCreate(),
        .exe = ffStrbufCreate(),
        .version = ffStrbufCreate(),
    };
    const char* error = ffDetectEditor(&result);

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Editor), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Editor), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        if (result.exe.length) {
            ffStrbufWriteTo(&result.exe, stdout);
            if (result.version.length) {
                printf(" %s", result.version.chars);
            }
        } else {
            ffStrbufWriteTo(&result.name, stdout);
        }
        putchar('\n');
    } else {
        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(Editor), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                                                                                                           FF_ARG(result.type, "type"),
                                                                                                           FF_ARG(result.name, "name"),
                                                                                                           FF_ARG(result.exe, "exe-name"),
                                                                                                           FF_ARG(result.path, "path"),
                                                                                                           FF_ARG(result.version, "version"),
                                                                                                       }));
    }

    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.path);
    ffStrbufDestroy(&result.exe);
    ffStrbufDestroy(&result.version);

    return true;
}

void ffParseEditorJsonObject(FFEditorOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Editor), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateEditorJsonConfig(FFEditorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateEditorJsonResult([[maybe_unused]] FFEditorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FFEditorResult result = {
        .name = ffStrbufCreate(),
        .path = ffStrbufCreate(),
        .version = ffStrbufCreate(),
    };

    const char* error = ffDetectEditor(&result);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_str(doc, obj, "type", result.type);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &result.name);
    yyjson_mut_obj_add_strbuf(doc, obj, "path", &result.path);
    yyjson_mut_obj_add_strbuf(doc, obj, "exe", &result.exe);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);

    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.path);
    ffStrbufDestroy(&result.exe);
    ffStrbufDestroy(&result.version);

    return true;
}

void ffInitEditorOptions(FFEditorOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󱞎");
}

void ffDestroyEditorOptions(FFEditorOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffEditorModuleInfo = {
    .name = "Editor",
    .description = "Print information about the default editor ($VISUAL or $EDITOR)",
    .displayName = {
        .en = "Editor",
        .de = "Editor",
        .es = "Editor",
        .fr = "Éditeur",
        .it = "Editor",
        .ja = "エディター",
        .ko = "편집기",
        .pl = "Edytor",
        .pt = "Editor",
        .ru = "Редактор",
        .zh_CN = "编辑器",
        .zh_TW = "編輯器",
    },
    .initOptions = (void*) ffInitEditorOptions,
    .destroyOptions = (void*) ffDestroyEditorOptions,
    .parseJsonObject = (void*) ffParseEditorJsonObject,
    .printModule = (void*) ffPrintEditor,
    .generateJsonResult = (void*) ffGenerateEditorJsonResult,
    .generateJsonConfig = (void*) ffGenerateEditorJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "Type (Visual / Editor)", "type" },
        { "Name", "name" },
        { "Exe name of real path", "exe-name" },
        { "Full path of real path", "path" },
        { "Version", "version" },
    })),
    .defaultOrder = 16,
};
