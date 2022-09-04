#include "host.h"

#include "pthread.h"

void ffDetectHostImpl(FFHostResult* host);

const FFHostResult* ffDetectHost()
{
    static FFHostResult host;

    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &host;
    }
    init = true;

    ffDetectHostImpl(&host);

    pthread_mutex_unlock(&mutex);
    return &host;
}
