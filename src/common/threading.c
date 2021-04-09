#include "fastfetch.h"

#include <pthread.h>

static inline void* calculatePlasmaThreadMain(void* instance)
{
    ffCalculatePlasma((FFinstance*)instance, NULL, NULL, NULL);
    return NULL;
}

static inline void* calculateGTK2ThreadMain(void* instance)
{
    ffCalculateGTK2((FFinstance*)instance, NULL, NULL, NULL);
    return NULL;
}

static inline void* calculateGTK3ThreadMain(void* instance)
{
    ffCalculateGTK3((FFinstance*)instance, NULL, NULL, NULL);
    return NULL;
}

static inline void* calculateGTK4ThreadMain(void* instance)
{
    ffCalculateGTK4((FFinstance*)instance, NULL, NULL, NULL);
    return NULL;
}

static inline void* calculateWMThreadMain(void* instance)
{
    ffCalculateWM((FFinstance*)instance, NULL, NULL, NULL);
    return NULL;
}

static inline void* calculateTerminalThreadMain(void* instance)
{
    ffCalculateTerminal((FFinstance*)instance, NULL, NULL, NULL);
    return NULL;
}

static inline void* startThreadsThreadMain(void* instance)
{
    pthread_t wmThread;
    pthread_create(&wmThread, NULL, calculateWMThreadMain, instance);
    pthread_detach(wmThread);

    pthread_t gtk2Thread;
    pthread_create(&gtk2Thread, NULL, calculateGTK2ThreadMain, instance);
    pthread_detach(gtk2Thread);

    pthread_t gtk3Thread;
    pthread_create(&gtk3Thread, NULL, calculateGTK3ThreadMain, instance);
    pthread_detach(gtk3Thread);

    pthread_t gtk4Thread;
    pthread_create(&gtk4Thread, NULL, calculateGTK4ThreadMain, instance);
    pthread_detach(gtk4Thread);

    pthread_t terminalThread;
    pthread_create(&terminalThread, NULL, calculateTerminalThreadMain, instance);
    pthread_detach(terminalThread);

    pthread_t plasmaThread;
    pthread_create(&plasmaThread, NULL, calculatePlasmaThreadMain, instance);
    pthread_detach(plasmaThread);

    return NULL;
}

void ffStartCalculationThreads(FFinstance* instance)
{
    pthread_t startThreadsThread;
    pthread_create(&startThreadsThread, NULL, startThreadsThreadMain, instance);
    pthread_detach(startThreadsThread);
}
