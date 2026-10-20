#include "wayland.h"

#if defined(FF_HAVE_WAYLAND) && defined(FF_HAVE_DBUS) && defined(__linux__)

    #include "common/arrutil.h"
    #include "common/dbus.h"
    #include "common/debug.h"
    #include "common/edidHelper.h"
    #include "common/strutil.h"

    #include <string.h>

// Read the display configuration of GNOME / Mutter and Cinnamon / Muffin, including the raw EDID
// data, instead of guessing the DRM connector by matching the wl_output name (#...).
// https://gitlab.gnome.org/GNOME/mutter/-/blob/main/data/dbus-interfaces/org.gnome.Mutter.DisplayConfig.xml

    // MetaMonitorColorMode
    // https://gitlab.gnome.org/GNOME/mutter/-/blob/main/src/backends/meta-monitor-manager-private.h
    #define FF_MUTTER_COLOR_MODE_BT2100 1

// Muffin (Cinnamon) exposes the very same interface as Mutter (GNOME), only the names differ
typedef struct FFMutterDisplayConfig {
    const char* busName;
    const char* objectPath;
    const char* interface;
} FFMutterDisplayConfig;

static const FFMutterDisplayConfig mutterDisplayConfigs[] = {
    {
        .busName = "org.gnome.Mutter.DisplayConfig",
        .objectPath = "/org/gnome/Mutter/DisplayConfig",
        .interface = "org.gnome.Mutter.DisplayConfig",
    },
    {
        .busName = "org.cinnamon.Muffin.DisplayConfig",
        .objectPath = "/org/cinnamon/Muffin/DisplayConfig",
        .interface = "org.cinnamon.Muffin.DisplayConfig",
    },
};

static inline DBusMessage* mutterCall(FFDBusData* dbus, const FFMutterDisplayConfig* config, const char* method) {
    return ffDBusGetMethodReply(dbus, config->busName, config->objectPath, config->interface, method, nullptr, nullptr);
}

// Returns the first display configuration interface that responds, or nullptr
static const FFMutterDisplayConfig* mutterFindConfig(FFDBusData* dbus) {
    for (uint32_t i = 0; i < ARRAY_SIZE(mutterDisplayConfigs); ++i) {
        DBusMessage* reply = mutterCall(dbus, &mutterDisplayConfigs[i], "GetCurrentState");
        if (reply != nullptr) {
            dbus->lib->ffdbus_message_unref(reply);
            return &mutterDisplayConfigs[i];
        }
    }

    return nullptr;
}

static FFDisplayResult* mutterFindDisplay(FFDisplayServerResult* result, const char* connector) {
    if (connector == nullptr || *connector == '\0') {
        return nullptr;
    }

    const uint64_t id = ffWaylandGenerateIdFromName(connector);
    FF_LIST_FOR_EACH (FFDisplayResult, display, result->displays) {
        if (display->id == id) {
            display->platformApi = "wayland-mutter";
            return display;
        }
    }

    return nullptr;
}

// Point `value` to the variant stored under `key` in the `a{sv}` dictionary `dict`
static bool mutterDictGet(FFDBusData* dbus, DBusMessageIter* dict, const char* key, DBusMessageIter* value) {
    if (dbus->lib->ffdbus_message_iter_get_arg_type(dict) != DBUS_TYPE_ARRAY) {
        return false;
    }

    DBusMessageIter entries;
    dbus->lib->ffdbus_message_iter_recurse(dict, &entries);

    while (dbus->lib->ffdbus_message_iter_get_arg_type(&entries) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter entry;
        dbus->lib->ffdbus_message_iter_recurse(&entries, &entry);

        if (dbus->lib->ffdbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_STRING) {
            const char* entryKey = nullptr;
            dbus->lib->ffdbus_message_iter_get_basic(&entry, &entryKey);
            if (entryKey && ffStrEquals(entryKey, key) && dbus->lib->ffdbus_message_iter_next(&entry)) {
                *value = entry;
                return true;
            }
        }

        if (!dbus->lib->ffdbus_message_iter_next(&entries)) {
            break;
        }
    }

    return false;
}

// Whether the `au` array contained in the variant `variant` contains `needle`
static bool mutterVariantContainsUint32(FFDBusData* dbus, DBusMessageIter* variant, uint32_t needle) {
    if (dbus->lib->ffdbus_message_iter_get_arg_type(variant) != DBUS_TYPE_VARIANT) {
        return false;
    }

    DBusMessageIter array;
    dbus->lib->ffdbus_message_iter_recurse(variant, &array);
    if (dbus->lib->ffdbus_message_iter_get_arg_type(&array) != DBUS_TYPE_ARRAY) {
        return false;
    }

    DBusMessageIter items;
    dbus->lib->ffdbus_message_iter_recurse(&array, &items);

    while (dbus->lib->ffdbus_message_iter_get_arg_type(&items) == DBUS_TYPE_UINT32) {
        uint32_t item = 0;
        dbus->lib->ffdbus_message_iter_get_basic(&items, &item);
        if (item == needle) {
            return true;
        }
        if (!dbus->lib->ffdbus_message_iter_next(&items)) {
            break;
        }
    }

    return false;
}

// Copy the `ay` byte array contained in the variant `variant` into `data`
static bool mutterVariantGetBytes(FFDBusData* dbus, DBusMessageIter* variant, uint8_t* data, uint32_t capacity, uint32_t* length) {
    uint32_t count = 0;

    if (dbus->lib->ffdbus_message_iter_get_arg_type(variant) == DBUS_TYPE_VARIANT) {
        DBusMessageIter array;
        dbus->lib->ffdbus_message_iter_recurse(variant, &array);
        if (dbus->lib->ffdbus_message_iter_get_arg_type(&array) == DBUS_TYPE_ARRAY) {
            DBusMessageIter items;
            dbus->lib->ffdbus_message_iter_recurse(&array, &items);
            while (dbus->lib->ffdbus_message_iter_get_arg_type(&items) == DBUS_TYPE_BYTE) {
                uint8_t byte = 0;
                dbus->lib->ffdbus_message_iter_get_basic(&items, &byte);
                if (count < capacity) {
                    data[count] = byte;
                }
                ++count;
                if (!dbus->lib->ffdbus_message_iter_next(&items)) {
                    break;
                }
            }
        }
    }

    *length = count;
    return count > 0 && count <= capacity;
}

static void mutterApplyEdid(FFDisplayResult* display, const uint8_t* edid, uint32_t length) {
    FF_STRBUF_AUTO_DESTROY edidName = ffStrbufCreate();
    if (ffEdidGetName(edid, &edidName) && edidName.length > 0) {
        ffStrbufSet(&display->name, &edidName);
    }

    if (display->serial.length == 0) {
        ffEdidGetSerial(edid, &display->serial);
    }

    if (display->manufactureYear == 0) {
        ffEdidGetManufactureDate(edid, &display->manufactureYear, &display->manufactureWeek);
    }

    if (display->hdrStatus == FF_DISPLAY_HDR_STATUS_UNKNOWN) {
        display->hdrStatus = ffEdidGetHdrCompatible(edid, length)
            ? FF_DISPLAY_HDR_STATUS_SUPPORTED
            : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
    }

    if (display->preferredWidth == 0) {
        uint32_t width = 0, height = 0;
        double refreshRate = 0;
        ffEdidGetPreferredResolutionAndRefreshRate(edid, &width, &height, &refreshRate);
        display->preferredWidth = width;
        display->preferredHeight = height;
        display->preferredRefreshRate = refreshRate;
    }
}

static void mutterApplyMonitorProperties(FFDBusData* dbus, DBusMessageIter* properties, FFDisplayResult* display) {
    DBusMessageIter value;

    // "is-builtin" is authoritative, unlike ffdsGetDisplayType() which guesses from the connector name
    if (mutterDictGet(dbus, properties, "is-builtin", &value)) {
        bool builtin = false;
        if (ffDBusGetBool(dbus, &value, &builtin)) {
            display->type = builtin ? FF_DISPLAY_TYPE_BUILTIN : FF_DISPLAY_TYPE_EXTERNAL;
        }
    }

    bool hdrEnabled = false;
    if (mutterDictGet(dbus, properties, "color-mode", &value)) {
        uint64_t colorMode = 0;
        hdrEnabled = ffDBusGetUint(dbus, &value, &colorMode) && colorMode == FF_MUTTER_COLOR_MODE_BT2100;
    }

    bool hdrSupported = false;
    if (mutterDictGet(dbus, properties, "supported-color-modes", &value)) {
        hdrSupported = mutterVariantContainsUint32(dbus, &value, FF_MUTTER_COLOR_MODE_BT2100);
    }

    if (hdrEnabled) {
        display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
    } else if (hdrSupported && display->hdrStatus == FF_DISPLAY_HDR_STATUS_UNKNOWN) {
        display->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;
    }
}

static void mutterParseCurrentState(FFDBusData* dbus, DBusMessage* reply, FFDisplayServerResult* result) {
    DBusMessageIter root;
    if (!dbus->lib->ffdbus_message_iter_init(reply, &root)) {
        return;
    }

    // (u serial, a(monitors) monitors, a(logical_monitors) logical_monitors, a{sv} properties)
    if (!dbus->lib->ffdbus_message_iter_next(&root)) { // monitors
        return;
    }

    if (dbus->lib->ffdbus_message_iter_get_arg_type(&root) == DBUS_TYPE_ARRAY) {
        DBusMessageIter monitors;
        dbus->lib->ffdbus_message_iter_recurse(&root, &monitors);

        while (dbus->lib->ffdbus_message_iter_get_arg_type(&monitors) == DBUS_TYPE_STRUCT) {
            DBusMessageIter monitor;
            dbus->lib->ffdbus_message_iter_recurse(&monitors, &monitor);

            // ((ssss) monitor_specs, a(siiddada{sv}) modes, a{sv} properties)
            if (dbus->lib->ffdbus_message_iter_get_arg_type(&monitor) == DBUS_TYPE_STRUCT) {
                DBusMessageIter specs;
                dbus->lib->ffdbus_message_iter_recurse(&monitor, &specs);

                const char* connector = nullptr;
                if (dbus->lib->ffdbus_message_iter_get_arg_type(&specs) == DBUS_TYPE_STRING) {
                    dbus->lib->ffdbus_message_iter_get_basic(&specs, &connector);
                }

                FFDisplayResult* display = mutterFindDisplay(result, connector);
                if (display) {
                    // (connector, vendor, product, serial)
                    DBusMessageIter serialSpec = specs;
                    if (display->serial.length == 0 &&
                        dbus->lib->ffdbus_message_iter_next(&serialSpec) && // vendor
                        dbus->lib->ffdbus_message_iter_next(&serialSpec) && // product
                        dbus->lib->ffdbus_message_iter_next(&serialSpec) && // serial
                        dbus->lib->ffdbus_message_iter_get_arg_type(&serialSpec) == DBUS_TYPE_STRING) {
                        const char* serial = nullptr;
                        dbus->lib->ffdbus_message_iter_get_basic(&serialSpec, &serial);
                        if (serial && !ffStrEquals(serial, "unknown")) {
                            ffStrbufSetS(&display->serial, serial);
                        }
                    }

                    // Skip `modes`, then read `properties`
                    DBusMessageIter properties = monitor;
                    if (dbus->lib->ffdbus_message_iter_next(&properties) &&
                        dbus->lib->ffdbus_message_iter_next(&properties) &&
                        dbus->lib->ffdbus_message_iter_get_arg_type(&properties) == DBUS_TYPE_ARRAY) {
                        mutterApplyMonitorProperties(dbus, &properties, display);
                    }
                }
            }

            dbus->lib->ffdbus_message_iter_next(&monitors);
        }
    }

    // logical_monitors -> primary flag
    if (!dbus->lib->ffdbus_message_iter_next(&root) ||
        dbus->lib->ffdbus_message_iter_get_arg_type(&root) != DBUS_TYPE_ARRAY) {
        return;
    }

    DBusMessageIter logicalMonitors;
    dbus->lib->ffdbus_message_iter_recurse(&root, &logicalMonitors);

    while (dbus->lib->ffdbus_message_iter_get_arg_type(&logicalMonitors) == DBUS_TYPE_STRUCT) {
        DBusMessageIter logicalMonitor;
        dbus->lib->ffdbus_message_iter_recurse(&logicalMonitors, &logicalMonitor);

        // (i x, i y, d scale, u transform, b primary, a(ssss) monitors, a{sv} properties)
        int index = 0;
        bool primary = false;

        do {
            const int argType = dbus->lib->ffdbus_message_iter_get_arg_type(&logicalMonitor);

            if (index == 4 && argType == DBUS_TYPE_BOOLEAN) {
                dbus_bool_t value = 0;
                dbus->lib->ffdbus_message_iter_get_basic(&logicalMonitor, &value);
                primary = value != 0;
            } else if (index == 5 && primary && argType == DBUS_TYPE_ARRAY) {
                DBusMessageIter logicalMonitorMonitors;
                dbus->lib->ffdbus_message_iter_recurse(&logicalMonitor, &logicalMonitorMonitors);

                while (dbus->lib->ffdbus_message_iter_get_arg_type(&logicalMonitorMonitors) == DBUS_TYPE_STRUCT) {
                    DBusMessageIter specs;
                    dbus->lib->ffdbus_message_iter_recurse(&logicalMonitorMonitors, &specs);
                    if (dbus->lib->ffdbus_message_iter_get_arg_type(&specs) == DBUS_TYPE_STRING) {
                        const char* connector = nullptr;
                        dbus->lib->ffdbus_message_iter_get_basic(&specs, &connector);
                        FFDisplayResult* display = mutterFindDisplay(result, connector);
                        if (display) {
                            display->primary = true;
                        }
                    }
                    dbus->lib->ffdbus_message_iter_next(&logicalMonitorMonitors);
                }
            }

            ++index;
        } while (dbus->lib->ffdbus_message_iter_next(&logicalMonitor));

        dbus->lib->ffdbus_message_iter_next(&logicalMonitors);
    }
}

static void mutterParseResources(FFDBusData* dbus, DBusMessage* reply, FFDisplayServerResult* result) {
    DBusMessageIter root;
    if (!dbus->lib->ffdbus_message_iter_init(reply, &root)) {
        return;
    }

    // (u serial, a(crtcs) crtcs, a(outputs) outputs, a(modes) modes, i max_screen_width, i max_screen_height)
    DBusMessageIter outputs;
    if (!dbus->lib->ffdbus_message_iter_next(&root) || // crtcs
        !dbus->lib->ffdbus_message_iter_next(&root) || // outputs
        dbus->lib->ffdbus_message_iter_get_arg_type(&root) != DBUS_TYPE_ARRAY) {
        return;
    }
    dbus->lib->ffdbus_message_iter_recurse(&root, &outputs);

    while (dbus->lib->ffdbus_message_iter_get_arg_type(&outputs) == DBUS_TYPE_STRUCT) {
        DBusMessageIter output;
        dbus->lib->ffdbus_message_iter_recurse(&outputs, &output);

        // (u id, x output_id, i crtc_index, au crtcs, s name, au modes, au clones, a{sv} properties)
        FFDisplayResult* display = nullptr;
        int index = 0;

        do {
            const int argType = dbus->lib->ffdbus_message_iter_get_arg_type(&output);

            if (index == 4 && argType == DBUS_TYPE_STRING) {
                const char* connector = nullptr;
                dbus->lib->ffdbus_message_iter_get_basic(&output, &connector);
                display = mutterFindDisplay(result, connector);
            } else if (index == 7 && display != nullptr && argType == DBUS_TYPE_ARRAY) {
                DBusMessageIter value;
                if (mutterDictGet(dbus, &output, "edid", &value)) {
                    uint8_t edid[1024];
                    uint32_t length = 0;
                    if (mutterVariantGetBytes(dbus, &value, edid, ARRAY_SIZE(edid), &length) && ffEdidIsValid(edid, length)) {
                        mutterApplyEdid(display, edid, length);
                    }
                }
            }

            ++index;
        } while (dbus->lib->ffdbus_message_iter_next(&output));

        dbus->lib->ffdbus_message_iter_next(&outputs);
    }
}

// Whether org.gnome.Mutter.DisplayConfig (GNOME) or org.cinnamon.Muffin.DisplayConfig (Cinnamon)
// is reachable. Used to decide if the wl_output name based DRM connector matching
// (matchDrmConnector) can be skipped.
bool ffdsMutterAvailable(void) {
    FF_DBUS_AUTO_DESTROY_DATA FFDBusData dbus = {};
    if (ffDBusLoadData(DBUS_BUS_SESSION, &dbus) != nullptr) {
        return false;
    }

    return mutterFindConfig(&dbus) != nullptr;
}

// Enrich the displays detected from wl_output with the authoritative data reported by Mutter / Muffin.
// Returns true if the display configuration could be retrieved from D-Bus.
bool ffdsConnectMutter(FFDisplayServerResult* result) {
    FF_DBUS_AUTO_DESTROY_DATA FFDBusData dbus = {};
    const char* error = ffDBusLoadData(DBUS_BUS_SESSION, &dbus);
    if (error) {
        FF_DEBUG("ffDBusLoadData() failed: %s", error);
        return false;
    }

    const FFMutterDisplayConfig* config = mutterFindConfig(&dbus);
    if (config == nullptr) {
        FF_DEBUG("No Mutter / Muffin DisplayConfig interface found");
        return false;
    }

    DBusMessage* reply = mutterCall(&dbus, config, "GetCurrentState");
    if (reply == nullptr) {
        FF_DEBUG("GetCurrentState() failed");
        return false;
    }
    mutterParseCurrentState(&dbus, reply, result);
    dbus.lib->ffdbus_message_unref(reply);

    // GetCurrentState() does not expose the EDID data, so fall back to the legacy GetResources()
    reply = mutterCall(&dbus, config, "GetResources");
    if (reply != nullptr) {
        mutterParseResources(&dbus, reply, result);
        dbus.lib->ffdbus_message_unref(reply);
    } else {
        FF_DEBUG("GetResources() failed");
    }

    return true;
}

#endif
