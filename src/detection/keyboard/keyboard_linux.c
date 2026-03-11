#include "keyboard.h"
#include "common/io.h"
#include "common/stringUtils.h"

static bool isRealKeyboard(uint32_t index, FFstrbuf* path)
{
    // Check EV_REP (auto-repeat, bit 20) to filter pseudo-keyboards (Power Button, PC Speaker).
    ffStrbufSetF(path, "/sys/class/input/event%u/device/capabilities/ev", (unsigned) index);
    {
        FF_STRBUF_AUTO_DESTROY caps = ffStrbufCreate();
        if (!ffReadFileBuffer(path->chars, &caps))
            return false;

        ffStrbufTrimRightSpace(&caps);
        unsigned long val = strtoul(caps.chars, NULL, 16);
        if (!(val & (1UL << 20))) // EV_REP
            return false;
    }

    // Check KEY_A (bit 30) to filter media remotes and headset controls.
    // The key capability bitmap is space-separated hex longs, MSB first;
    // KEY_A falls in the last (least significant) word on all architectures.
    ffStrbufSetF(path, "/sys/class/input/event%u/device/capabilities/key", (unsigned) index);
    {
        FF_STRBUF_AUTO_DESTROY caps = ffStrbufCreate();
        if (!ffReadFileBuffer(path->chars, &caps))
            return false;

        ffStrbufTrimRightSpace(&caps);
        const char* lastWord = strrchr(caps.chars, ' ');
        lastWord = lastWord ? lastWord + 1 : caps.chars;

        unsigned long val = strtoul(lastWord, NULL, 16);
        if (!(val & (1UL << 30))) // KEY_A
            return false;
    }

    return true;
}

const char* ffDetectKeyboard(FFlist* devices /* List of FFKeyboardDevice */)
{
    // Parse /proc/bus/input/devices to find keyboards with a "kbd" handler.
    // This detects both wired and Bluetooth keyboards uniformly.
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!ffAppendFileBuffer("/proc/bus/input/devices", &content))
        return "ffAppendFileBuffer(\"/proc/bus/input/devices\") == NULL";

    uint64_t flags = 0;
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    FFstrbuf kbd = ffStrbufCreateStatic("kbd");

    char* line = NULL;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, &content))
    {
        if (!ffStrStartsWith(line, "H: Handlers="))
            continue;

        const char* handlers = line + strlen("H: Handlers=");

        if (!ffStrbufMatchSeparatedS(&kbd, handlers, ' '))
            continue;

        // Find "eventN" token and extract the index
        const char* eventStr = strstr(handlers, "event");
        if (!eventStr)
            continue;

        char* pend = NULL;
        uint32_t eventIndex = (uint32_t) strtoul(eventStr + strlen("event"), &pend, 10);
        if (pend == eventStr + strlen("event"))
            continue;

        // Skip duplicates (dedup bitmap covers indices 0-63; higher indices are not deduped)
        if (eventIndex < 64 && (flags & (1ULL << eventIndex)))
            continue;

        if (!isRealKeyboard(eventIndex, &path))
            continue;

        ffStrbufSetF(&path, "/sys/class/input/event%u/device/name", (unsigned) eventIndex);

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
        if (ffAppendFileBuffer(path.chars, &name))
        {
            if (eventIndex < 64)
                flags |= (1ULL << eventIndex);

            ffStrbufTrimRightSpace(&name);
            ffStrbufSubstrBefore(&path, path.length - (uint32_t) strlen("name"));

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
