#include "fastfetch.h"
#include "common/io/io.h"
#include "common/thread.h"
#include "temps_linux.h"

#include <string.h>
#include <dirent.h>

static bool parseHwmonDir(FFstrbuf* dir, FFTempValue* value)
{
    //https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface
    uint32_t dirLength = dir->length;

    FF_STRBUF_AUTO_DESTROY valueBuffer = ffStrbufCreate();

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

const FFTempsResult* ffDetectTemps(void)
{
    static FFTempsResult result;
    static bool init = false;

    if(init)
        return &result;
    init = true;

    ffListInitA(&result.values, sizeof(FFTempValue), 16);

    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&baseDir, "/sys/class/hwmon/");

    uint32_t baseDirLength = baseDir.length;

    DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
        return &result;

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

    return &result;
}
