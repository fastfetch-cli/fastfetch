#include "keyboard.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

const char* ffDetectKeyboard(FFlist* devices /* List of FFKeyboardDevice */)
{
    // There is no /sys/class/input/kbd* on Linux
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/dev/input/by-path/");
    if (dirp == NULL)
        return "opendir(\"/dev/input/by-path/\") == NULL";

    uint64_t flags = 0;

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (!ffStrEndsWith(entry->d_name, "-event-kbd"))
            continue;

        char buffer[32]; // `../eventX`
        ssize_t len = readlinkat(dirfd(dirp), entry->d_name, buffer, ARRAY_SIZE(buffer) - 1);
        if (len != strlen("../eventX") || !ffStrStartsWith(buffer, "../event")) continue;
        buffer[len] = 0;

        const char* eventid = buffer + strlen("../event");

        char* pend = NULL;
        uint32_t index = (uint32_t) strtoul(eventid, &pend, 10);
        if (pend == eventid) continue;

        // Ignore duplicate entries
        if (flags & (1 << index))
            continue;
        flags |= (1 << index);

        ffStrbufSetF(&path, "/sys/class/input/event%s/device/name", eventid);

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
        if (ffAppendFileBuffer(path.chars, &name))
        {
            ffStrbufTrimRightSpace(&name);
            ffStrbufSubstrBefore(&path, path.length - 4);

            FFKeyboardDevice* device = (FFKeyboardDevice*) ffListAdd(devices);
            ffStrbufInitMove(&device->name, &name);
            ffStrbufInit(&device->serial);

            ffStrbufAppendS(&path, "uniq");
            if (ffAppendFileBuffer(path.chars, &device->serial))
                ffStrbufTrimRightSpace(&device->serial);
        }
    }

    return NULL;
}
