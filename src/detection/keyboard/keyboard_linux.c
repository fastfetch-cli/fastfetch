#include "keyboard.h"
#include "common/io.h"
#include "common/stringUtils.h"

#include <linux/input-event-codes.h>

const char* ffDetectKeyboard(FFlist* devices /* List of FFKeyboardDevice */) {
    // Parse /proc/bus/input/devices to find keyboards with a "kbd" handler.
    // This detects both wired and Bluetooth keyboards uniformly.
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!ffAppendFileBuffer("/proc/bus/input/devices", &content)) {
        return "ffAppendFileBuffer(\"/proc/bus/input/devices\") == NULL";
    }

    FFstrbuf kbd = ffStrbufCreateStatic("kbd");

    FFKeyboardDevice device = {
        .name = ffStrbufCreate(),
        .serial = ffStrbufCreate()};

    char* line = NULL;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, &content)) {
        switch (line[0]) {
            case 'N': {
                const uint32_t prefixLen = strlen("N: Name=");
                if (__builtin_expect(len <= prefixLen, false)) {
                    continue;
                }
                const char* name = line + prefixLen;
                const uint32_t nameLen = (uint32_t) len - prefixLen;
                ffStrbufSetNS(&device.name, nameLen, name);
                ffStrbufTrim(&device.name, '"');
                continue;
            }
            case 'H': {
                const uint32_t prefixLen = strlen("H: Handlers=");
                if (__builtin_expect(len <= prefixLen, false)) {
                    continue;
                }
                const char* handlers = line + prefixLen;
                const uint32_t handlersLen = (uint32_t) len - prefixLen;
                if (!ffStrbufMatchSeparatedNS(&kbd, handlersLen, handlers, ' ')) {
                    goto skipDevice;
                }
                continue;
            }
            case 'B': {
                const char* bits = line + strlen("B: ");
                if (ffStrStartsWith(bits, "EV=")) {
                    // Check EV_REP (auto-repeat, bit 20) to filter pseudo-keyboards (Power Button, PC Speaker).
                    const char* evBits = bits + strlen("EV=");
                    uint64_t val = strtoull(evBits, NULL, 16);
                    if (!(val & (1ULL << EV_REP))) {
                        goto skipDevice;
                    }
                } else if (ffStrStartsWith(bits, "KEY=")) {
                    // Check KEY_A (bit 30) to filter media remotes and headset controls.
                    // The key capability bitmap is space-separated hex longs, MSB first;
                    // KEY_A falls in the last (least significant) word on all architectures.
                    const char* keyBits = bits + strlen("KEY=");
                    const char* lastWord = memrchr(keyBits, ' ', len - (size_t) (keyBits - line));
                    lastWord = lastWord ? lastWord + 1 : keyBits;

                    uint64_t val = strtoull(lastWord, NULL, 16);
                    if (!(val & (1ULL << KEY_A))) {
                        goto skipDevice;
                    }
                }
                continue;
            }
            case 'U': {
                const uint32_t prefixLen = strlen("U: Uniq=");
                if (__builtin_expect(len <= prefixLen, false)) {
                    continue;
                }
                const char* uniq = line + prefixLen;
                const uint32_t uniqLen = (uint32_t) len - prefixLen;
                ffStrbufSetNS(&device.serial, uniqLen, uniq);
                continue;
            }
            case '\0':
                // End of device entry; add to list if it has a name.
                if (device.name.length > 0) {
                    FFKeyboardDevice* added = (FFKeyboardDevice*) ffListAdd(devices);
                    ffStrbufInitMove(&added->name, &device.name);
                    ffStrbufInitMove(&added->serial, &device.serial);
                }
                continue;
            default:
                continue;
        }

    skipDevice:
        // Skip to the end of the current device entry.
        while (line[0] != '\0' && ffStrbufGetline(&line, &len, &content));
        // Despite the fn name, it resets the string buffer to initial state
        ffStrbufDestroy(&device.name);
        ffStrbufDestroy(&device.serial);
    }

    return NULL;
}
