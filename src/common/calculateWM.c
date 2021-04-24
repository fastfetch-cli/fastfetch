#include "fastfetch.h"

#include <dirent.h>
#include <pthread.h>

const FFWMResult* ffCalculateWM(FFinstance* instance)
{
    UNUSED(instance);

    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static FFWMResult result;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.processName);
    ffStrbufInit(&result.prettyName);
    ffStrbufInit(&result.error);

    DIR* proc = opendir("/proc/");
    if(proc == NULL)
    {
        ffStrbufSetS(&result.error, "opendir(\"/proc/\") == NULL");
        pthread_mutex_unlock(&mutex);
        return &result;
    }

    struct dirent* dirent;

    while((dirent = readdir(proc)) != NULL)
    {
        if(dirent->d_type != DT_DIR)
            continue;

        char path[20];
        sprintf(path, "/proc/%.8s/comm", dirent->d_name);

        ffGetFileContent(path, &result.processName);

        if(ffStrbufIgnCaseCompS(&result.processName, "kwin_wayland") == 0 || ffStrbufIgnCaseCompS(&result.processName, "kwin_x11") == 0)
            ffStrbufSetS(&result.prettyName, "KWin");
        
        if(ffStrbufIgnCaseCompS(&result.processName, "openbox") == 0)
            ffStrbufSetS(&result.prettyName, "Openbox");

		if(ffStrbufIgnCaseCompS(&result.processName, "cinnamon") == 0)
            ffStrbufSetS(&result.prettyName, "Mutter");

		if(ffStrbufIgnCaseCompS(&result.processName, "xfwm4") == 0)
            ffStrbufSetS(&result.prettyName, "XFCE Window Manager");

		if(ffStrbufIgnCaseCompS(&result.processName, "wayfire") == 0)
            ffStrbufSetS(&result.prettyName, "Wayfire");

        if(result.prettyName.length > 0)
            break;
    }

    closedir(proc);

    if(result.prettyName.length == 0)
    {
        ffStrbufSetS(&result.error, "No process name matches the name of known display managers");
        ffStrbufClear(&result.processName);
    }

    pthread_mutex_unlock(&mutex);

    return &result;
}
