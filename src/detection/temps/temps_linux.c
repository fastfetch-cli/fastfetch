#include "fastfetch.h"
#include "common/io.h"
#include "common/thread.h"
#include "temps_linux.h"

#include <string.h>
#include <dirent.h>

static bool parseHwmonDir(FFstrbuf* dir, FFTempValue* value)
{
    uint32_t dirLength = dir->length;

    FFstrbuf valueBuffer;
    ffStrbufInit(&valueBuffer);

    ffStrbufAppendS(dir, "temp1_input");
    ffReadFileBuffer(dir->chars, &valueBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    value->value = ffStrbufToDouble(&valueBuffer);

    ffStrbufDestroy(&valueBuffer);

    if(value->value != value->value)
        return false;

    ffStrbufAppendS(dir, "name");
    ffReadFileBuffer(dir->chars, &value->name);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufAppendS(dir, "device/class");
    ffReadFileBuffer(dir->chars, &value->deviceClass);
    ffStrbufSubstrBefore(dir, dirLength);

    return value->name.length > 0 || value->deviceClass.length > 0;
}

const FFTempsResult* ffDetectTemps(const FFinstance* instance)
{
    static FFTempsResult result;
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;
    static bool init = false;

    ffThreadMutexLock(&mutex);
    if(init)
    {
        ffThreadMutexUnlock(&mutex);
        return &result;
    }
    init = true;

    if(!instance->config.allowSlowOperations)
    {
        ffListInitA(&result.values, sizeof(FFTempValue), 0);
        ffThreadMutexUnlock(&mutex);
        return &result;
    }

    ffListInitA(&result.values, sizeof(FFTempValue), 16);

    FFstrbuf baseDir;
    ffStrbufInitA(&baseDir, 64);
    ffStrbufAppendS(&baseDir, "/sys/class/hwmon/");

    uint32_t baseDirLength = baseDir.length;

    DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
    {
        ffStrbufDestroy(&baseDir);
        ffThreadMutexUnlock(&mutex);
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
        if(!parseHwmonDir(&baseDir, temp))
        {
            ffStrbufDestroy(&temp->name);
            ffStrbufDestroy(&temp->deviceClass);
            --result.values.length;
        }

        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    closedir(dirp);
    ffStrbufDestroy(&baseDir);

    ffThreadMutexUnlock(&mutex);
    return &result;
}
