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
    ffStrbufTrimRightSpace(&value->name);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufAppendS(dir, "device/class");
    if(!ffReadFileBuffer(dir->chars, &valueBuffer))
    {
        ffStrbufSubstrBefore(dir, dirLength);
        ffStrbufAppendS(dir, "device/device/class");
        ffReadFileBuffer(dir->chars, &valueBuffer);
    }
    ffStrbufTrimRightSpace(&valueBuffer);
    ffStrbufSubstrBefore(dir, dirLength);
    if(valueBuffer.length)
        value->deviceClass = (uint32_t) strtoul(valueBuffer.chars, NULL, 16);

    ffStrbufClear(&valueBuffer);
    ffStrbufEnsureFree(&valueBuffer, 64);
    ffStrbufAppendS(dir, "device");
    ssize_t linkLen = readlink(dir->chars, valueBuffer.chars, valueBuffer.allocated - 1);
    if (linkLen > 0)
    {
        valueBuffer.length = (uint32_t) linkLen;
        valueBuffer.chars[linkLen] = 0;
        ffStrbufSubstrAfterLastC(&valueBuffer, '/');
        ffStrbufInitMove(&value->deviceName, &valueBuffer);
    }
    else
        ffStrbufInit(&value->deviceName);

    return value->name.length > 0 || value->deviceClass > 0;
}

const FFlist* ffDetectTemps(void)
{
    static FFlist result;

    if(result.elementSize > 0)
        return &result;

    ffListInitA(&result, sizeof(FFTempValue), 16);

    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&baseDir, "/sys/class/hwmon/");

    uint32_t baseDirLength = baseDir.length;

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
        return &result;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(&baseDir, entry->d_name);
        ffStrbufAppendC(&baseDir, '/');

        FFTempValue* temp = ffListAdd(&result);
        ffStrbufInit(&temp->name);
        temp->deviceClass = 0;
        if(!parseHwmonDir(&baseDir, temp))
        {
            ffStrbufDestroy(&temp->name);
            --result.length;
        }

        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    return &result;
}
