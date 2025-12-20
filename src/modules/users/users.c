#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/time.h"
#include "detection/users/users.h"
#include "modules/users/users.h"
#include "util/stringUtils.h"

#pragma GCC diagnostic ignored "-Wformat" // warning: unknown conversion type character 'F' in format

bool ffPrintUsers(FFUsersOptions* options)
{
    FF_LIST_AUTO_DESTROY users = ffListCreate(sizeof(FFUserResult));

    const char* error = ffDetectUsers(options, &users);

    if(error)
    {
        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if(users.length == 0)
    {
        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "Unable to detect any users");
        return false;
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
                FFUserResult* user = FF_LIST_GET(FFUserResult, users, i);
                ffStrbufAppend(&result, &user->name);
            }
            ffStrbufPutTo(&result, stdout);
        }
        else
        {
            for(uint32_t i = 0; i < users.length; ++i)
            {
                FFUserResult* user = FF_LIST_GET(FFUserResult, users, i);

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
        uint64_t now = ffTimeGetNow();
        for(uint32_t i = 0; i < users.length; ++i)
        {
            FFUserResult* user = FF_LIST_GET(FFUserResult, users, i);

            uint64_t duration = now - user->loginTime;
            uint32_t milliseconds = (uint32_t) (duration % 1000);
            duration /= 1000;
            uint32_t seconds = (uint32_t) (duration % 60);
            duration /= 60;
            uint32_t minutes = (uint32_t) (duration % 60);
            duration /= 60;
            uint32_t hours = (uint32_t) (duration % 24);
            duration /= 24;
            uint32_t days = (uint32_t) duration;

            FFTimeGetAgeResult age = ffTimeGetAge(user->loginTime, ffTimeGetNow());

            FF_PRINT_FORMAT_CHECKED(FF_USERS_MODULE_NAME, users.length == 1 ? 0 : (uint8_t) (i + 1), &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
                FF_FORMAT_ARG(user->name, "name"),
                FF_FORMAT_ARG(user->hostName, "host-name"),
                FF_FORMAT_ARG(user->sessionName, "session-name"),
                FF_FORMAT_ARG(user->clientIp, "client-ip"),
                {FF_FORMAT_ARG_TYPE_STRING, ffTimeToShortStr(user->loginTime), "login-time"},
                FF_FORMAT_ARG(days, "days"),
                FF_FORMAT_ARG(hours, "hours"),
                FF_FORMAT_ARG(minutes, "minutes"),
                FF_FORMAT_ARG(seconds, "seconds"),
                FF_FORMAT_ARG(milliseconds, "milliseconds"),
                FF_FORMAT_ARG(age.years, "years"),
                FF_FORMAT_ARG(age.daysOfYear, "days-of-year"),
                FF_FORMAT_ARG(age.yearsFraction, "years-fraction"),
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

    return true;
}

void ffParseUsersJsonObject(FFUsersOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (unsafe_yyjson_equals_str(key, "type") || unsafe_yyjson_equals_str(key, "condition"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "compact"))
        {
            options->compact = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "myselfOnly"))
        {
            options->myselfOnly = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_USERS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateUsersJsonConfig(FFUsersOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "compact", options->compact);

    yyjson_mut_obj_add_bool(doc, module, "myselfOnly", options->myselfOnly);
}

bool ffGenerateUsersJsonResult(FFUsersOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFUserResult));

    const char* error = ffDetectUsers(options, &results);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
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

    return true;
}

void ffInitUsersOptions(FFUsersOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->compact = false;
    options->myselfOnly = false;
}

void ffDestroyUsersOptions(FFUsersOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffUsersModuleInfo = {
    .name = FF_USERS_MODULE_NAME,
    .description = "Print users currently logged in",
    .initOptions = (void*) ffInitUsersOptions,
    .destroyOptions = (void*) ffDestroyUsersOptions,
    .parseJsonObject = (void*) ffParseUsersJsonObject,
    .printModule = (void*) ffPrintUsers,
    .generateJsonResult = (void*) ffGenerateUsersJsonResult,
    .generateJsonConfig = (void*) ffGenerateUsersJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"User name", "name"},
        {"Host name", "host-name"},
        {"Session name", "session-name"},
        {"Client IP", "client-ip"},
        {"Login Time in local timezone", "login-time"},
        {"Days after login", "days"},
        {"Hours after login", "hours"},
        {"Minutes after login", "minutes"},
        {"Seconds after login", "seconds"},
        {"Milliseconds after login", "milliseconds"},
        {"Years integer after login", "years"},
        {"Days of year after login", "days-of-year"},
        {"Years fraction after login", "years-fraction"},
    }))
};
