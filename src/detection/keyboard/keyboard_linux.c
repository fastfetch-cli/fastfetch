#include "keyboard.h"
#include "common/io.h"
#include "common/stringUtils.h"

static void addDevice(FFlist* devices, uint64_t* flags, uint32_t index, FFstrbuf* path)
{
    if (index >= 64 || (*flags & (1UL << index)))
        return;
    *flags |= (1UL << index);

    ffStrbufSetF(path, "/sys/class/input/event%u/device/name", (unsigned) index);

    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
    if (ffAppendFileBuffer(path->chars, &name))
    {
        ffStrbufTrimRightSpace(&name);
        ffStrbufSubstrBefore(path, path->length - (uint32_t) strlen("name"));

        FFKeyboardDevice* device = (FFKeyboardDevice*) ffListAdd(devices);
        ffStrbufInitMove(&device->name, &name);
        ffStrbufInit(&device->serial);

        ffStrbufAppendS(path, "uniq");
        if (ffAppendFileBuffer(path->chars, &device->serial))
            ffStrbufTrimRightSpace(&device->serial);
    }
}

static bool detectFromByPath(FFlist* devices, uint64_t* flags, FFstrbuf* path)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/dev/input/by-path/");
    if (dirp == NULL)
        return false;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (!ffStrEndsWith(entry->d_name, "-event-kbd"))
            continue;

        char buffer[32]; // `../eventXX`
        ssize_t len = readlinkat(dirfd(dirp), entry->d_name, buffer, ARRAY_SIZE(buffer) - 1);
        if (len <= (ssize_t) strlen("../event") || !ffStrStartsWith(buffer, "../event")) continue;
        buffer[len] = 0;

        char* pend = NULL;
        uint32_t index = (uint32_t) strtoul(buffer + strlen("../event"), &pend, 10);
        if (pend == buffer + strlen("../event")) continue;

        addDevice(devices, flags, index, path);
    }

    return true;
}

static bool hasAutoRepeat(uint32_t index, FFstrbuf* path)
{
    // Filter out pseudo-keyboards (Power Button, PC Speaker) by checking for
    // EV_REP (auto-repeat, bit 20) in the device's event capabilities.
    // Real keyboards and macro pads support auto-repeat; system buttons don't.
    ffStrbufSetF(path, "/sys/class/input/event%u/device/capabilities/ev", (unsigned) index);

    FF_STRBUF_AUTO_DESTROY caps = ffStrbufCreate();
    if (!ffReadFileBuffer(path->chars, &caps))
        return false;

    ffStrbufTrimRightSpace(&caps);
    unsigned long val = strtoul(caps.chars, NULL, 16);
    return (val & (1UL << 20)) != 0; // EV_REP
}

static bool detectFromProc(FFlist* devices, uint64_t* flags, FFstrbuf* path)
{
    // Bluetooth and other virtual keyboards may not appear in /dev/input/by-path/.
    // Parse /proc/bus/input/devices to find any keyboard with a "kbd" handler.
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!ffAppendFileBuffer("/proc/bus/input/devices", &content))
        return false;

    const char* line = content.chars;
    while (line && *line)
    {
        if (ffStrStartsWith(line, "H: Handlers="))
        {
            const char* handlers = line + strlen("H: Handlers=");
            bool hasKbd = false;
            uint32_t eventIndex = UINT32_MAX;

            // Parse space-separated handler names
            const char* p = handlers;
            while (*p && *p != '\n')
            {
                while (*p == ' ') p++;
                if (*p == '\n' || *p == '\0') break;

                const char* wordStart = p;
                while (*p && *p != ' ' && *p != '\n') p++;
                uint32_t wordLen = (uint32_t)(p - wordStart);

                if (wordLen == 3 && memcmp(wordStart, "kbd", 3) == 0)
                    hasKbd = true;
                else if (wordLen > strlen("event") && memcmp(wordStart, "event", strlen("event")) == 0)
                {
                    char* pend = NULL;
                    eventIndex = (uint32_t) strtoul(wordStart + 5, &pend, 10);
                    if (pend == wordStart + 5) eventIndex = UINT32_MAX;
                }
            }

            if (hasKbd && eventIndex != UINT32_MAX && hasAutoRepeat(eventIndex, path))
                addDevice(devices, flags, eventIndex, path);
        }

        const char* next = strchr(line, '\n');
        line = next ? next + 1 : NULL;
    }

    return true;
}

const char* ffDetectKeyboard(FFlist* devices /* List of FFKeyboardDevice */)
{
    uint64_t flags = 0;
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();

    // Prefer /dev/input/by-path/ for wired/USB keyboards
    bool byPathOk = detectFromByPath(devices, &flags, &path);

    // Fall back to /proc/bus/input/devices for Bluetooth and other keyboards
    // that don't appear in by-path. The flags bitmap prevents duplicates.
    bool procOk = detectFromProc(devices, &flags, &path);

    if (!byPathOk && !procOk)
        return "Failed to read both /dev/input/by-path/ and /proc/bus/input/devices";

    return NULL;
}
