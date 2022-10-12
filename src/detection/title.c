#include "fastfetch.h"
#include "detection/title.h"
#include "common/thread.h"

#include <limits.h>
#include <unistd.h>
#include <netdb.h>

#ifndef HOST_NAME_MAX
    #define HOST_NAME_MAX 64
#endif

#ifdef __linux__
static void detectFQDN(FFTitleResult* title)
{
    struct addrinfo hints = {0};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    struct addrinfo* info = NULL;

    if(getaddrinfo(title->hostname.chars, "80", &hints, &info) == 0)
    {
        struct addrinfo* current = info;
        while(title->fqdn.length == 0 && current != NULL)
        {
            ffStrbufAppendS(&title->fqdn, current->ai_canonname);
            current = current->ai_next;
        }

        freeaddrinfo(info);
    }
}
#endif

const FFTitleResult* ffDetectTitle(const FFinstance* instance)
{
    static FFTitleResult result;

    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;
    static bool init = false;
    ffThreadMutexLock(&mutex);
    if(init)
    {
        ffThreadMutexUnlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.userName);
    ffStrbufAppendS(&result.userName, instance->state.passwd->pw_name);

    ffStrbufInitA(&result.hostname, HOST_NAME_MAX);
    ffStrbufAppendS(&result.hostname, instance->state.utsname.nodename);

    ffStrbufInitA(&result.fqdn, HOST_NAME_MAX);
    #ifdef __linux
        detectFQDN(&result);
    #endif
    if(result.fqdn.length == 0)
        ffStrbufAppend(&result.fqdn, &result.hostname);

    ffThreadMutexUnlock(&mutex);
    return &result;
}
