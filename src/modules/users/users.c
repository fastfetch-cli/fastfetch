#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/users/users.h"
#include "modules/users/users.h"
#include "util/stringUtils.h"

#include <time.h>

#define FF_USERS_NUM_FORMAT_ARGS 1

void ffPrintUsers(FFUsersOptions* options)
{
    FF_LIST_AUTO_DESTROY users = ffListCreate(sizeof(FFUserResult));

    const char* error = ffDetectUsers(&users);

    if(error)
    {
        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(users.length == 0)
    {
        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, "%s", "Unable to detect any users");
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
                {
                    char buf[64];
                    time_t time = (time_t) (user->loginTime / 1000);
                    strftime(buf, sizeof(buf), "%FT%T%z", localtime(&time));
                    ffStrbufAppendF(&result, " - login time %s", buf);
                }

                ffStrbufPutTo(&result, stdout);
            }
        }
    }
    else
    {
        for(uint32_t i = 0; i < users.length; ++i)
        {
            FFUserResult* user = (FFUserResult*)ffListGet(&users, i);

            ffPrintFormat(FF_USERS_MODULE_NAME, users.length == 1 ? 0 : (uint8_t) (i + 1), &options->moduleArgs, FF_USERS_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &user->name},
                {FF_FORMAT_ARG_TYPE_STRBUF, &user->hostName},
                {FF_FORMAT_ARG_TYPE_STRBUF, &user->sessionName},
                {FF_FORMAT_ARG_TYPE_STRBUF, &user->clientIp},
                {FF_FORMAT_ARG_TYPE_UINT64, &user->loginTime},
            });
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

void ffInitUsersOptions(FFUsersOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_USERS_MODULE_NAME, ffParseUsersCommandOptions, ffParseUsersJsonObject, ffPrintUsers, ffGenerateUsersJson, ffPrintUsersHelpFormat);
    ffOptionInitModuleArg(&options->moduleArgs);

    options->compact = false;
}

bool ffParseUsersCommandOptions(FFUsersOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_USERS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if(ffStrEqualsIgnCase(subKey, "compact"))
    {
        options->compact = ffOptionParseBoolean(value);
        return true;
    }

    return false;
}

void ffDestroyUsersOptions(FFUsersOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseUsersJsonObject(FFUsersOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffStrEqualsIgnCase(key, "compact"))
        {
            options->compact = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateUsersJson(FF_MAYBE_UNUSED FFUsersOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFUserResult));

    const char* error = ffDetectUsers(&results);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFUserResult, user, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &user->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "hostName", &user->hostName);
        yyjson_mut_obj_add_strbuf(doc, obj, "sessionName", &user->sessionName);
        yyjson_mut_obj_add_strbuf(doc, obj, "clientIp", &user->clientIp);
        yyjson_mut_obj_add_uint(doc, obj, "loginTime", user->loginTime);
    }

exit:
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
    ffPrintModuleFormatHelp(FF_USERS_MODULE_NAME, "{1}@{2} - login time {5}", FF_USERS_NUM_FORMAT_ARGS, (const char* []) {
        "User name",
        "Host name",
        "Session name",
        "Client IP",
        "Login Time"
    });
}
