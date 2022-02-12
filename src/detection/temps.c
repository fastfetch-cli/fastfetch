#include "fastfetch.h"

#include <pthread.h>
#include <dirent.h>
#include <string.h>

static bool isTempFile(const char* name)
{
    return
        name[0] == 't' &&
        name[1] == 'e' &&
        name[2] == 'm' &&
        name[3] == 'p' &&
        name[4] >= '0' &&
        name[4] <= '9' &&
        strncmp(name + 5, "_input", 6) == 0;
}

static bool parseHwmonDir(FFstrbuf* dir, FFTempValue* value)
{
    DIR* dirp = opendir(dir->chars);
    if(dirp == NULL)
        return false;

    uint32_t dirLength = dir->length;

    struct dirent* dirent;
    while((dirent = readdir(dirp)) != NULL)
    {
        if(isTempFile(dirent->d_name))
        {
            ffStrbufAppendS(dir, dirent->d_name);
            ffGetFileContent(dir->chars, &value->value);
            ffStrbufSubstrBefore(dir, dirLength);
            break;
        }
    }

    closedir(dirp);

    if(value->value.length == 0)
        return false;

    ffStrbufAppendS(dir, "name");
    ffGetFileContent(dir->chars, &value->name);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufAppendS(dir, "device/class");
    ffGetFileContent(dir->chars, &value->deviceClass);
    ffStrbufSubstrBefore(dir, dirLength);

    return value->name.length > 0 || value->deviceClass.length > 0;
}

const FFTempsResult* ffDetectTemps(const FFinstance* instance)
{
    FF_UNUSED(instance)

    static FFTempsResult result;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffListInitA(&result.values, sizeof(FFTempValue), 16);

    FFstrbuf baseDir;
    ffStrbufInitA(&baseDir, 64);
    ffStrbufAppendS(&baseDir, "/sys/class/hwmon/");

    uint32_t baseDirLength = baseDir.length;

    DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(&baseDir, entry->d_name);
        ffStrbufAppendC(&baseDir, '/');

        FFTempValue* temp = ffListAdd(&result.values);
        ffStrbufInit(&temp->name);
        ffStrbufInit(&temp->deviceClass);
        ffStrbufInit(&temp->value);
        if(!parseHwmonDir(&baseDir, temp))
        {
            ffStrbufDestroy(&temp->name);
            ffStrbufDestroy(&temp->deviceClass);
            ffStrbufDestroy(&temp->value);
            --result.values.length;
        }

        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    pthread_mutex_unlock(&mutex);
    return &result;
}
