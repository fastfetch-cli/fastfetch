#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/lm/lm.h"
#include "modules/lm/lm.h"

bool ffPrintLM(FFLMOptions* options) {
    bool success = false;
    FFLMResult result;
    ffStrbufInit(&result.service);
    ffStrbufInit(&result.prettyName);
    ffStrbufInit(&result.version);
    const char* error = ffDetectLM(&result);

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(LM), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if (result.service.length == 0) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(LM), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No LM service found");
        goto exit;
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(LM), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result.prettyName, stdout);
        if (result.version.length) {
            printf(" %s", result.version.chars);
        }
        putchar('\n');
    } else {
        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(LM), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                                                                                                       FF_ARG(result.service, "service"),
                                                                                                       FF_ARG(result.prettyName, "pretty-name"),
                                                                                                       FF_ARG(result.version, "version"),
                                                                                                   }));
    }
    success = true;

exit:
    ffStrbufDestroy(&result.service);
    ffStrbufDestroy(&result.prettyName);
    ffStrbufDestroy(&result.version);

    return success;
}

void ffParseLMJsonObject(FFLMOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(LM), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateLMJsonConfig(FFLMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateLMJsonResult([[maybe_unused]] FFLMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    bool success = false;
    FFLMResult result;
    ffStrbufInit(&result.service);
    ffStrbufInit(&result.prettyName);
    ffStrbufInit(&result.version);
    const char* error = ffDetectLM(&result);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if (result.service.length == 0) {
        yyjson_mut_obj_add_str(doc, module, "error", "No LM service found");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "service", &result.service);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result.prettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);
    success = true;

exit:
    ffStrbufDestroy(&result.service);
    ffStrbufDestroy(&result.prettyName);
    ffStrbufDestroy(&result.version);

    return success;
}

void ffInitLMOptions(FFLMOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰧨");
}

void ffDestroyLMOptions(FFLMOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffLMModuleInfo = {
    .name = "LM",
    .description = "Print login manager (desktop manager) name and version",
    .displayName = {
        .en = "Login Manager",
        .ar = "مدير تسجيل الدخول",
        .cs = "Správce přihlášení",
        .de = "Anmeldemanager",
        .es = "Gestor de inicio de sesión",
        .fr = "Gestionnaire de connexion",
        .gl = "Xestor de inicio de sesión",
        .he = "מנהל התחברות",
        .id = "Manajer Login",
        .it = "Gestore di accesso",
        .ja = "ログインマネージャー",
        .ko = "로그인 관리자",
        .pl = "Menedżer logowania",
        .pt = "Gerenciador de login",
        .ru = "Менеджер входа",
        .tr = "Oturum Açma Yöneticisi",
        .uk = "Менеджер входу",
        .vi = "Trình quản lý đăng nhập",
        .zh_CN = "登录管理器",
        .zh_TW = "登入管理器",
    },
    .initOptions = (void*) ffInitLMOptions,
    .destroyOptions = (void*) ffDestroyLMOptions,
    .parseJsonObject = (void*) ffParseLMJsonObject,
    .printModule = (void*) ffPrintLM,
    .generateJsonResult = (void*) ffGenerateLMJsonResult,
    .generateJsonConfig = (void*) ffGenerateLMJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "LM service / process name", "service" },
        { "LM pretty name", "pretty-name" },
        { "LM version", "version" },
    })),
    .defaultOrder = 20,
};
