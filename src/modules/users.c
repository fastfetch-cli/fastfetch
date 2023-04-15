#include "fastfetch.h"
#include "common/printing.h"
#include "detection/users/users.h"

#define FF_USERS_MODULE_NAME "Users"
#define FF_USERS_NUM_FORMAT_ARGS 1

void ffPrintUsers(FFinstance* instance)
{
    FF_LIST_AUTO_DESTROY users;
    ffListInit(&users, sizeof(FFstrbuf));

    FF_STRBUF_AUTO_DESTROY error = ffStrbufCreate();

    ffDetectUsers(&users, &error);

    if(error.length > 0)
    {
        ffPrintError(instance, FF_USERS_MODULE_NAME, 0, &instance->config.users, "%*s", error.length, error.chars);
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
}
