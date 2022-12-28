#include "fastfetch.h"
#include "common/printing.h"
#include "detection/users/users.h"

#define FF_USERS_MODULE_NAME "Users"
#define FF_USERS_NUM_FORMAT_ARGS 1

void ffPrintUsers(FFinstance* instance)
{
    FFlist users;
    ffListInit(&users, sizeof(FFstrbuf));

    FFstrbuf error;
    ffStrbufInit(&error);

    ffDetectUsers(&users, &error);

    if(error.length > 0)
    {
        ffPrintError(instance, FF_USERS_MODULE_NAME, 0, &instance->config.users, "%*s", error.length, error.chars);
        ffListDestroy(&users);
        ffStrbufDestroy(&error);
        return;
    }

    FFstrbuf result;
    ffStrbufInit(&result);
    for(uint32_t i = 0; i < users.length; ++i)
    {
        if(i > 0)
            ffStrbufAppendS(&result, ", ");
        FFstrbuf* user = (FFstrbuf*)ffListGet(&users, i);
        ffStrbufAppend(&result, user);
        ffStrbufDestroy(user);
    }

    ffListDestroy(&users);

    if(instance->config.users.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_USERS_MODULE_NAME, 0, &instance->config.users.key);
        puts(result.chars);
    }
    else
    {
        ffPrintFormat(instance, FF_USERS_MODULE_NAME, 0, &instance->config.users, FF_USERS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result},
        });
    }

    ffStrbufDestroy(&result);
}
