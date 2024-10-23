#include "gamepad.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

static void detectGamepad(FFlist* devices, FFstrbuf* name, FFstrbuf* path)
{
    uint32_t baseLen = path->length;
    FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd(devices);
    ffStrbufInit(&device->serial);
    ffStrbufInitMove(&device->name, name);
    device->battery = 0;

    ffStrbufAppendS(path, "uniq");
    if (ffAppendFileBuffer(path->chars, &device->serial))
        ffStrbufTrimRightSpace(&device->serial);

    ffStrbufSubstrBefore(path, baseLen);
    ffStrbufAppendS(path, "device/power_supply/"); // /sys/class/input/jsX/device/device/power_supply

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(path->chars);
    if (dirp)
    {
        struct dirent* entry;
        while ((entry = readdir(dirp)) != NULL)
        {
            if (entry->d_name[0] == '.' || (entry->d_type != DT_DIR && entry->d_type != DT_UNKNOWN))
                continue;
            ffStrbufAppendS(path, entry->d_name);
            ffStrbufAppendS(path, "/capacity"); // /sys/class/input/jsX/device/device/power_supply/XXX/capacity
            char capacity[32];
            ssize_t nRead = ffReadFileData(path->chars, ARRAY_SIZE(capacity) - 1, capacity);
            if (nRead > 0) // Tested with a PS4 controller
            {
                capacity[nRead] = '\0';
                device->battery = (uint8_t) strtoul(capacity, NULL, 10);
                break;
            }

            ffStrbufAppendS(path, "_level");
            nRead = ffReadFileData(path->chars, ARRAY_SIZE(capacity) - 1, capacity);
            if (nRead > 0) // Tested with a NS Pro controller
            {
                // https://github.com/torvalds/linux/blob/52b1853b080a082ec3749c3a9577f6c71b1d4a90/drivers/power/supply/power_supply_sysfs.c#L124
                switch (capacity[0])
                {
                    case 'C': device->battery = 1; break; // Critical
                    case 'L': device->battery = 25; break; // Low
                    case 'N': device->battery = 50; break; // Normal
                    case 'H': device->battery = 75; break; // High
                    case 'F': device->battery = 100; break; // Full
                }
            }
        }
    }
}

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/input/");
    if (dirp == NULL)
        return "opendir(\"/sys/class/input/\") == NULL";

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/sys/class/input/");
    uint32_t baseLen = path.length;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (!ffStrStartsWith(entry->d_name, "js"))
            continue;
        if (!ffCharIsDigit(entry->d_name[strlen("js")]))
            continue;

        ffStrbufAppendS(&path, entry->d_name);
        ffStrbufAppendS(&path, "/device/name");

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
        if (ffAppendFileBuffer(path.chars, &name))
        {
            ffStrbufTrimRightSpace(&name);
            ffStrbufSubstrBefore(&path, path.length - 4);
            detectGamepad(devices, &name, &path);
        }

        ffStrbufSubstrBefore(&path, baseLen);
    }

    return NULL;
}
