#include "fastfetch.h"
#include "common/printing.h"

#if FF_HAVE_UTMPX_H
    #include <utmpx.h>
#else
    //for Android compatibility
    #include <utmp.h>
    #define utmpx utmp
    #define setutxent setutent
    #define getutxent getutent
#endif

#define FF_USERS_MODULE_NAME "Users"
#define FF_USERS_NUM_FORMAT_ARGS 1

void ffPrintUsers(FFinstance* instance)
{
    struct utmpx* n = NULL;
    setutxent();

    FFlist users;
    ffListInit(&users, sizeof(n->ut_user) + 1);

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

            char* dest = ffListAdd(&users);
            strncpy(dest, n->ut_user, sizeof(n->ut_user));
            dest[sizeof(n->ut_user)] = '\0';
        }
    }

    if(users.length == 0)
    {
        ffListDestroy(&users);
        ffPrintError(instance, FF_USERS_MODULE_NAME, 0, &instance->config.users, "Unable to detect users");
        return;
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
