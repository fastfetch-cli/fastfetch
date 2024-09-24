#include "fastfetch.h"
#include "common/io/io.h"
#include "temps_linux.h"

#include <dirent.h>

static double parseHwmonDir(FFstrbuf* dir, FFstrbuf* buffer)
{
    //https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface
    uint32_t dirLength = dir->length;
    ffStrbufAppendS(dir, "temp1_input");

    if(!ffReadFileBuffer(dir->chars, buffer))
    {
        // Some badly implemented system put temp file in /hwmonN/device
        ffStrbufSubstrBefore(dir, dirLength);
        ffStrbufAppendS(dir, "device/");
        dirLength = dir->length;
        ffStrbufAppendS(dir, "temp1_input");

        if(!ffReadFileBuffer(dir->chars, buffer))
            return 0.0/0.0;
    }

    ffStrbufSubstrBefore(dir, dirLength);

    double value = ffStrbufToDouble(buffer);// millidegree Celsius

    if(value != value)
        return 0.0/0.0;

    ffStrbufAppendS(dir, "name");
    if (!ffReadFileBuffer(dir->chars, buffer))
        return 0.0/0.0;

    ffStrbufTrimRightSpace(buffer);

    if(
        ffStrbufContainS(buffer, "cpu") ||
        ffStrbufEqualS(buffer, "k10temp") || // AMD
        ffStrbufEqualS(buffer, "coretemp") // Intel
    ) return value / 1000.;

    return false;
}

double ffDetectCPUTemp(void)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&baseDir, "/sys/class/hwmon/");

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    uint32_t baseDirLength = baseDir.length;

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
        return 0.0/0.0;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(&baseDir, entry->d_name);
        ffStrbufAppendC(&baseDir, '/');

        double result = parseHwmonDir(&baseDir, &buffer);
        if (result == result)
            return result;

        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    return 0.0/0.0;
}
