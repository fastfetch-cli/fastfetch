#include "fastfetch.h"
#include "detection/temps.h"
#include "common/io.h"

#include <string.h>
#include <pthread.h>
#include <dirent.h>

static bool isTempFile(const char* name)
{
    return
        strncmp(name, "temp", 4) == 0 &&
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
    value->value = 0.0 / 0.0; //use NaN as error value

    FFstrbuf valueString;
    ffStrbufInit(&valueString);

    struct dirent* dirent;
    while((dirent = readdir(dirp)) != NULL)
    {
        if(!isTempFile(dirent->d_name))
            continue;

        ffStrbufAppendS(dir, dirent->d_name);
        ffReadFileBuffer(dir->chars, &valueString);
        ffStrbufSubstrBefore(dir, dirLength);

        //ffStrbufToDouble() returns NaN if the string couldn't be parsed
        value->value = ffStrbufToDouble(&valueString);
        if(value->value != value->value)
            continue;

        value->value /= 1000.0; //millidegrees to degrees
        break;
    }

    closedir(dirp);
    ffStrbufDestroy(&valueString);

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
        if(!parseHwmonDir(&baseDir, temp))
        {
            ffStrbufDestroy(&temp->name);
            ffStrbufDestroy(&temp->deviceClass);
            --result.values.length;
        }

        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    pthread_mutex_unlock(&mutex);
    return &result;
}
