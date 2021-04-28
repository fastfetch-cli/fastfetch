#include "fastfetch.h"

#include <pthread.h>

static inline void* detectPlasmaThreadMain(void* instance)
{
    ffDetectPlasma((FFinstance*)instance);
    return NULL;
}

static inline void* detectGTK2ThreadMain(void* instance)
{
    ffDetectGTK2((FFinstance*)instance);
    return NULL;
}

static inline void* detectGTK3ThreadMain(void* instance)
{
    ffDetectGTK3((FFinstance*)instance);
    return NULL;
}

static inline void* detectGTK4ThreadMain(void* instance)
{
    ffDetectGTK4((FFinstance*)instance);
    return NULL;
}

static inline void* detectWMThreadMain(void* instance)
{
    ffDetectWM((FFinstance*)instance);
    return NULL;
}

static inline void* detectTerminalThreadMain(void* instance)
{
    ffDetectTerminal((FFinstance*)instance);
    return NULL;
}

static inline void* startThreadsThreadMain(void* instance)
{
    pthread_t wmThread;
    pthread_create(&wmThread, NULL, detectWMThreadMain, instance);
    pthread_detach(wmThread);

    pthread_t gtk2Thread;
    pthread_create(&gtk2Thread, NULL, detectGTK2ThreadMain, instance);
    pthread_detach(gtk2Thread);

    pthread_t gtk3Thread;
    pthread_create(&gtk3Thread, NULL, detectGTK3ThreadMain, instance);
    pthread_detach(gtk3Thread);

    pthread_t gtk4Thread;
    pthread_create(&gtk4Thread, NULL, detectGTK4ThreadMain, instance);
    pthread_detach(gtk4Thread);

    pthread_t terminalThread;
    pthread_create(&terminalThread, NULL, detectTerminalThreadMain, instance);
    pthread_detach(terminalThread);

    pthread_t plasmaThread;
    pthread_create(&plasmaThread, NULL, detectPlasmaThreadMain, instance);
    pthread_detach(plasmaThread);

    return NULL;
}

void ffStartCalculationThreads(FFinstance* instance)
{
    pthread_t startThreadsThread;
    pthread_create(&startThreadsThread, NULL, startThreadsThreadMain, instance);
    pthread_detach(startThreadsThread);
}
