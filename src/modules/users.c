#include "fastfetch.h"
#include "common/printing.h"

#include <utmpx.h>

#define FF_USERS_MODULE_NAME "Users"
#define FF_USERS_NUM_FORMAT_ARGS 1

void ffPrintUsers(FFinstance* instance)
{
    struct utmpx* n = NULL;
    setutxent();

    FFlist users;
    ffListInit(&users, sizeof(n->ut_user));

next:
    while((n = getutxent()))
    {
        if(n->ut_type == USER_PROCESS)
        {
            for(uint32_t i = 0; i < users.length; ++i)
            {
                if(strcmp((const char*)ffListGet(&users, i), n->ut_user) == 0)
                    goto next;
            }
            strcpy((char*)ffListAdd(&users), n->ut_user);
        }
    }

    FFstrbuf result;
    ffStrbufInit(&result);
    for(uint32_t i = 0; i < users.length; ++i)
    {
        if(i > 0)
            ffStrbufAppendS(&result, ", ");
        ffStrbufAppendS(&result, (const char*)ffListGet(&users, i));
    }
    ffListDestroy(&users);

    if(instance->config.uptime.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_USERS_MODULE_NAME, 0, &instance->config.uptime.key);
        puts(result.chars);
    }
    else
    {
        ffPrintFormat(instance, FF_USERS_MODULE_NAME, 0, &instance->config.uptime, FF_USERS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result},
        });
    }

    ffStrbufDestroy(&result);
}
