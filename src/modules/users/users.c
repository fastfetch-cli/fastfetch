#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/users/users.h"
#include "modules/users/users.h"
#include "util/stringUtils.h"

#define FF_USERS_NUM_FORMAT_ARGS 1

void ffPrintUsers(FFinstance* instance, FFUsersOptions* options)
{
    FF_LIST_AUTO_DESTROY users = ffListCreate(sizeof(FFstrbuf));

    FF_STRBUF_AUTO_DESTROY error = ffStrbufCreate();

    ffDetectUsers(&users, &error);

    if(error.length > 0)
    {
        ffPrintError(instance, FF_USERS_MODULE_NAME, 0, &options->moduleArgs, "%*s", error.length, error.chars);
        return;
    }

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    for(uint32_t i = 0; i < users.length; ++i)
    {
        if(i > 0)
            ffStrbufAppendS(&result, ", ");
        FFstrbuf* user = (FFstrbuf*)ffListGet(&users, i);
        ffStrbufAppend(&result, user);
        ffStrbufDestroy(user);
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_USERS_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        puts(result.chars);
    }
    else
    {
        ffPrintFormat(instance, FF_USERS_MODULE_NAME, 0, &options->moduleArgs, FF_USERS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result},
        });
    }
}

void ffInitUsersOptions(FFUsersOptions* options)
{
    options->moduleName = FF_USERS_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseUsersCommandOptions(FFUsersOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_USERS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyUsersOptions(FFUsersOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseUsersJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFUsersOptions __attribute__((__cleanup__(ffDestroyUsersOptions))) options;
    ffInitUsersOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_USERS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintUsers(instance, &options);
}
