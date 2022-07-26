#include "fastfetch.h"
#include "detection/title.h"

#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>

const FFTitleResult* ffDetectTitle(const FFinstance* instance)
{
    static FFTitleResult result;

    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.userName);
    ffStrbufAppendS(&result.userName, instance->state.passwd->pw_name);

    ffStrbufInitA(&result.hostname, HOST_NAME_MAX);
    ffStrbufAppendS(&result.hostname, instance->state.utsname.nodename);

    ffStrbufInitA(&result.fqdn, HOST_NAME_MAX);
    struct hostent* host = gethostbyname(result.hostname.chars);
    if(host != NULL)
        ffStrbufAppendS(&result.fqdn, host->h_name);
    if(result.fqdn.length == 0)
        ffStrbufAppend(&result.fqdn, &result.hostname);

    pthread_mutex_unlock(&mutex);
    return &result;
}
