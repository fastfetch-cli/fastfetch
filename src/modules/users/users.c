#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/time.h"
#include "detection/users/users.h"
#include "modules/users/users.h"
#include "util/stringUtils.h"

#pragma GCC diagnostic ignored "-Wformat" // warning: unknown conversion type character 'F' in format

#define FF_USERS_NUM_FORMAT_ARGS 5

void ffPrintUsers(FFUsersOptions* options)
{
    FF_LIST_AUTO_DESTROY users = ffListCreate(sizeof(FFUserResult));

    const char* error = ffDetectUsers(options, &users);

    if(error)
    {
        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(users.length == 0)
    {
        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "Unable to detect any users");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        if(options->compact)
        {
            ffPrintLogoAndKey(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

            FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
            for(uint32_t i = 0; i < users.length; ++i)
            {
                if(i > 0)
                    ffStrbufAppendS(&result, ", ");
                FFUserResult* user = (FFUserResult*)ffListGet(&users, i);
                ffStrbufAppend(&result, &user->name);
            }
            ffStrbufPutTo(&result, stdout);
        }
        else
        {
            for(uint32_t i = 0; i < users.length; ++i)
            {
                FFUserResult* user = (FFUserResult*)ffListGet(&users, i);

                ffPrintLogoAndKey(FF_USERS_MODULE_NAME, users.length == 1 ? 0 : (uint8_t) (i + 1), &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

                FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateCopy(&user->name);
                if(user->hostName.length)
                    ffStrbufAppendF(&result, "@%s", user->hostName.chars);

                if(user->loginTime)
                    ffStrbufAppendF(&result, " - login time %s", ffTimeToShortStr(user->loginTime));

                ffStrbufPutTo(&result, stdout);
            }
        }
    }
    else
    {
        for(uint32_t i = 0; i < users.length; ++i)
        {
            FFUserResult* user = (FFUserResult*)ffListGet(&users, i);

            FF_PRINT_FORMAT_CHECKED(FF_USERS_MODULE_NAME, users.length == 1 ? 0 : (uint8_t) (i + 1), &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_USERS_NUM_FORMAT_ARGS, ((FFformatarg[]){
                FF_FORMAT_ARG(user->name, "name"),
                FF_FORMAT_ARG(user->hostName, "host-name"),
                FF_FORMAT_ARG(user->sessionName, "session-name"),
                FF_FORMAT_ARG(user->clientIp, "client-ip"),
                {FF_FORMAT_ARG_TYPE_STRING, ffTimeToShortStr(user->loginTime), "login-time"},
            }));
        }
    }

    FF_LIST_FOR_EACH(FFUserResult, user, users)
    {
        ffStrbufDestroy(&user->clientIp);
        ffStrbufDestroy(&user->hostName);
        ffStrbufDestroy(&user->sessionName);
        ffStrbufDestroy(&user->name);
    }
}

bool ffParseUsersCommandOptions(FFUsersOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_USERS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "compact"))
    {
        options->compact = ffOptionParseBoolean(value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "myself-only"))
    {
        options->myselfOnly = ffOptionParseBoolean(value);
        return true;
    }

    return false;
}

void ffParseUsersJsonObject(FFUsersOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if (ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffStrEqualsIgnCase(key, "compact"))
        {
            options->compact = yyjson_get_bool(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "myselfOnly"))
        {
            options->myselfOnly = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateUsersJsonConfig(FFUsersOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyUsersOptions))) FFUsersOptions defaultOptions;
    ffInitUsersOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (options->compact != defaultOptions.compact)
        yyjson_mut_obj_add_bool(doc, module, "compact", options->compact);

    if (options->myselfOnly != defaultOptions.myselfOnly)
        yyjson_mut_obj_add_bool(doc, module, "myselfOnly", options->myselfOnly);
}

void ffGenerateUsersJsonResult(FFUsersOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFUserResult));

    const char* error = ffDetectUsers(options, &results);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFUserResult, user, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &user->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "hostName", &user->hostName);
        yyjson_mut_obj_add_strbuf(doc, obj, "sessionName", &user->sessionName);
        yyjson_mut_obj_add_strbuf(doc, obj, "clientIp", &user->clientIp);
        const char* pstr = ffTimeToFullStr(user->loginTime);
        if (*pstr)
            yyjson_mut_obj_add_strcpy(doc, obj, "loginTime", pstr);
        else
            yyjson_mut_obj_add_null(doc, obj, "loginTime");
    }

    FF_LIST_FOR_EACH(FFUserResult, user, results)
    {
        ffStrbufDestroy(&user->clientIp);
        ffStrbufDestroy(&user->hostName);
        ffStrbufDestroy(&user->sessionName);
        ffStrbufDestroy(&user->name);
    }
}

void ffPrintUsersHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_USERS_MODULE_NAME, "{1}@{2} - login time {5}", FF_USERS_NUM_FORMAT_ARGS, ((const char* []) {
        "User name - user-name",
        "Host name - host-name",
        "Session name - session",
        "Client IP - client-ip",
        "Login Time in local timezone - login-time",
    }));
}

void ffInitUsersOptions(FFUsersOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_USERS_MODULE_NAME,
        "Print users currently logged in",
        ffParseUsersCommandOptions,
        ffParseUsersJsonObject,
        ffPrintUsers,
        ffGenerateUsersJsonResult,
        ffPrintUsersHelpFormat,
        ffGenerateUsersJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->compact = false;
    options->myselfOnly = false;
}

void ffDestroyUsersOptions(FFUsersOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
