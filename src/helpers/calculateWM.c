#include "fastfetch.h"

#include <dirent.h>
#include <pthread.h>

void ffCalculateWM(FFinstance* instance, FFstrbuf** prettyNamePtr, FFstrbuf** processNamePtr, FFstrbuf** errorPtr)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    static FFstrbuf prettyName;
    static FFstrbuf processName;
    static FFstrbuf error;
    static bool init = false;

    if(prettyNamePtr != NULL)
        *prettyNamePtr = &prettyName;

    if(processNamePtr != NULL)
        *processNamePtr = &processName;

    if(errorPtr != NULL)
        *errorPtr = &error;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }

    init = true;

    ffStrbufInit(&prettyName);
    ffStrbufInit(&processName);
    ffStrbufInit(&error);

    DIR* proc = opendir("/proc/");
    if(proc == NULL)
    {
        ffStrbufSetS(&error, "opendir(\"/proc/\") == NULL");
        return;
    }

    struct dirent* dirent;

    while((dirent = readdir(proc)) != NULL)
    {
        if(dirent->d_type != DT_DIR)
            continue;

        char path[20];
        sprintf(path, "/proc/%.8s/comm", dirent->d_name);

        ffGetFileContent(path, &processName);

        if(ffStrbufIgnCaseCompS(&processName, "kwin_wayland") == 0 || ffStrbufIgnCaseCompS(&processName, "kwin_x11") == 0)
            ffStrbufSetS(&prettyName, "KWin");

        if(prettyName.length > 0)
            break;
    }

    closedir(proc);

    if(prettyName.length == 0)
    {
        ffStrbufSetS(&error, "No process name matches the name of known display managers");
        return;
    }

    pthread_mutex_unlock(&mutex);
}
