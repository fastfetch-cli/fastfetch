#include "fastfetch.h"

#include <dirent.h>
#include <pthread.h>

void ffCalculateWM(FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }

    init = true;

    ffStrbufInit(&instance->state.wm.processName);
    ffStrbufInit(&instance->state.wm.prettyName);
    ffStrbufInit(&instance->state.wm.error);

    DIR* proc = opendir("/proc/");
    if(proc == NULL)
    {
        ffStrbufSetS(&instance->state.wm.error, "opendir(\"/proc/\") == NULL");
        pthread_mutex_unlock(&mutex);
        return;
    }

    struct dirent* dirent;

    while((dirent = readdir(proc)) != NULL)
    {
        if(dirent->d_type != DT_DIR)
            continue;

        char path[20];
        sprintf(path, "/proc/%.8s/comm", dirent->d_name);

        ffGetFileContent(path, &instance->state.wm.processName);

        if(ffStrbufIgnCaseCompS(&instance->state.wm.processName, "kwin_wayland") == 0 || ffStrbufIgnCaseCompS(&instance->state.wm.processName, "kwin_x11") == 0)
            ffStrbufSetS(&instance->state.wm.prettyName, "KWin");

        if(instance->state.wm.prettyName.length > 0)
            break;
    }

    closedir(proc);

    if(instance->state.wm.prettyName.length == 0)
    {
        ffStrbufSetS(&instance->state.wm.error, "No process name matches the name of known display managers");
        ffStrbufClear(&instance->state.wm.processName);
    }

    pthread_mutex_unlock(&mutex);
}
