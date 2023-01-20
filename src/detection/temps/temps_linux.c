#include "fastfetch.h"
#include "common/io.h"
#include "common/thread.h"
#include "temps_linux.h"

#include <string.h>
#include <dirent.h>

static bool parseHwmonDir(FFstrbuf* dir, FFTempValue* value)
{
    //https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface
    uint32_t dirLength = dir->length;

    FF_STRBUF_AUTO_DESTROY valueBuffer;
    ffStrbufInit(&valueBuffer);

    ffStrbufAppendS(dir, "temp1_input");
    if(!ffReadFileBuffer(dir->chars, &valueBuffer))
        return false;

    ffStrbufSubstrBefore(dir, dirLength);

    value->value = ffStrbufToDouble(&valueBuffer) / 1000; // valueBuffer is millidegree Celsius

    if(value->value != value->value)
        return false;

    ffStrbufAppendS(dir, "name");
    ffReadFileBuffer(dir->chars, &value->name);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufAppendS(dir, "device/class");
    if(!ffReadFileBuffer(dir->chars, &valueBuffer))
    {
        ffStrbufSubstrBefore(dir, dirLength);
        ffStrbufAppendS(dir, "device/device/class");
        ffReadFileBuffer(dir->chars, &valueBuffer);
    }
    if(valueBuffer.length)
        value->deviceClass = (uint32_t) strtoul(valueBuffer.chars, NULL, 16);

    return value->name.length > 0 || value->deviceClass > 0;
}

const FFTempsResult* ffDetectTemps()
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
        temp->deviceClass = 0;
        if(!parseHwmonDir(&baseDir, temp))
        {
            ffStrbufDestroy(&temp->name);
            --result.values.length;
        }

        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    closedir(dirp);
    ffStrbufDestroy(&baseDir);

    ffThreadMutexUnlock(&mutex);
    return &result;
}
