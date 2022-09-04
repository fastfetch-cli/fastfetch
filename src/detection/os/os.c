#include "os.h"

#include <pthread.h>

void ffDetectOSImpl(FFOSResult* os, const FFinstance* instance);

const FFOSResult* ffDetectOS(const FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static FFOSResult result;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffDetectOSImpl(&result, instance);

    pthread_mutex_unlock(&mutex);
    return &result;
}
