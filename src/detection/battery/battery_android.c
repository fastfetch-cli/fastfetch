#include "fastfetch.h"
#include "battery.h"
#include "common/androidApi.h"
#include "common/android/binder.h"
#include "common/processing.h"
#include "common/properties.h"

// Android exposes the battery through BatteryManager, which the NDK does not wrap and whose sysfs
// backing is unreadable from an app UID. What does work without any permission is
// android.os.IBatteryPropertiesRegistrar in system_server, reachable either over /dev/binder (see
// common/android/binder.h) or through a `dumpsys battery` subprocess. The binder route is the cheap
// one: ~0.3 ms for both properties, and no child process.
//
// The registrar only answers properties that do not need BATTERY_STATS, and that rules out
// everything the module would otherwise like to show. Manufacturer, model name, technology, serial
// number, manufacture date, temperature and cycle count all sit behind that permission.

// The transaction code and the reply layout are both positional, and neither is negotiated with the
// service, so each only stays correct as long as AOSP keeps the order it already has. The code has
// not: `IBatteryPropertiesRegistrar` still declared registerListener / unregisterListener /
// getProperty up to Android 9, and Android 10 dropped the two listener methods, which moved
// `getProperty` from the third position to the first. The layout has, because `BatteryProperty` has
// only ever grown at the end -- API 35 appended [string8 mValueString] to it -- so the field read
// below is still the first one written.
//
// Getting either wrong is quiet rather than loud: `ffBinderReadU64` returns 0 for a read past the
// end of the reply, so a drift shows up as a wrong capacity, not as a failed call.
#define FF_BATTERY_ANDROID_SERVICE "batteryproperties"
#define FF_BATTERY_ANDROID_DESCRIPTOR "android.os.IBatteryPropertiesRegistrar"

// BatteryManager.BATTERY_PROPERTY_*
#define FF_BATTERY_ANDROID_PROPERTY_CAPACITY 4u
#define FF_BATTERY_ANDROID_PROPERTY_STATUS 6u

// BatteryManager.BATTERY_STATUS_*
#define FF_BATTERY_ANDROID_STATUS_CHARGING 2u
#define FF_BATTERY_ANDROID_STATUS_DISCHARGING 3u

// BatteryProperty starts with [int64 mValueLong], behind the usual [exception code][return value]
// [out-param non-null marker] prefix of an AIDL reply. API 35 appended [string8 mValueString] after
// the long for the one string valued property (`BATTERY_PROPERTY_SERIAL_NUMBER`), which does not
// move the long, so one offset covers every release.
#define FF_BATTERY_ANDROID_VALUE_OFFSET 12
#define FF_BATTERY_ANDROID_VALUE_UNSET 0x8000000000000000ULL // Long.MIN_VALUE, i.e. never filled in

static void initResult(FFBatteryResult* battery) {
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    battery->cycleCount = 0;
    battery->timeRemaining = -1;
    battery->capacity = 0;
    battery->status = FF_BATTERY_STATUS_NONE;
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->technology);
    ffStrbufInit(&battery->serial);
    ffStrbufInit(&battery->manufactureDate);
}

static const char* getProperty(FFBinder* binder, uint32_t handle, uint32_t property, uint64_t* value) {
    // `getProperty` is the third method declared in `IBatteryPropertiesRegistrar` up to Android 9 and
    // the first one from Android 10 on. Written as an `if` because clang rejects `__builtin_available`
    // -- which is what FF_API_AT_LEAST expands to -- in any other position.
    uint32_t transaction = 3u;
    if (FF_API_AT_LEAST(29)) {
        transaction = 1u;
    }

    uint8_t parcelBuffer[128];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_BATTERY_ANDROID_DESCRIPTOR);
    ffBinderParcelPutU32(&parcel, property);

    uint8_t replyBuffer[128];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    const char* error = ffBinderTransact(binder, handle, transaction, &parcel, &reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        return "Battery service rejected the request";
    }
    if (ffBinderReadI32(reply.data, reply.size, 0) != 0) {
        return "Battery service raised an exception";
    }
    if (ffBinderReadI32(reply.data, reply.size, 4) != 0) {
        return "Battery service does not report this property";
    }

    const uint64_t result = ffBinderReadU64(reply.data, reply.size, FF_BATTERY_ANDROID_VALUE_OFFSET);
    if (result == FF_BATTERY_ANDROID_VALUE_UNSET) {
        return "Battery service left this property empty";
    }

    *value = result;
    return nullptr;
}

static const char* parseBinder(FFlist* results) {
    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    const char* error = ffBinderOpen(&binder);
    if (error != nullptr) {
        return error;
    }

    uint32_t handle = 0;
    error = ffBinderLookupService(&binder, FF_BATTERY_ANDROID_SERVICE, FF_BINDER_SM_GET_SERVICE, &handle);
    if (error != nullptr) {
        return error;
    }

    uint64_t capacity = 0;
    error = getProperty(&binder, handle, FF_BATTERY_ANDROID_PROPERTY_CAPACITY, &capacity);
    if (error != nullptr) {
        return error;
    }

    uint64_t status = 0;
    error = getProperty(&binder, handle, FF_BATTERY_ANDROID_PROPERTY_STATUS, &status);
    if (error != nullptr) {
        return error;
    }

    FFBatteryResult* battery = FF_LIST_ADD(FFBatteryResult, *results);
    initResult(battery);
    battery->capacity = (double) capacity;
    if (status == FF_BATTERY_ANDROID_STATUS_CHARGING) {
        battery->status |= FF_BATTERY_STATUS_CHARGING;
    } else if (status == FF_BATTERY_ANDROID_STATUS_DISCHARGING) {
        battery->status |= FF_BATTERY_STATUS_DISCHARGING;
    }
    return nullptr;
}

static const char* parseDumpsys(FFBatteryOptions* options, FFlist* results) {
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buf, (char*[]) {
                                        "/system/bin/dumpsys",
                                        "battery",
                                        nullptr,
                                    }) != nullptr ||
        buf.length == 0) {
        return "Executing `/system/bin/dumpsys battery` failed"; // Only works in `adb shell`, or when rooted
    }

    if (!ffStrbufStartsWithS(&buf, "Current Battery Service state:\n")) {
        return "Invalid `/system/bin/dumpsys battery` result";
    }

    const char* start = buf.chars + strlen("Current Battery Service state:\n");

    FF_STRBUF_AUTO_DESTROY temp = ffStrbufCreate();
    if (!ffParsePropLines(start, "present: ", &temp) || !ffStrbufEqualS(&temp, "true")) {
        return nullptr;
    }
    ffStrbufClear(&temp);

    FFBatteryResult* battery = FF_LIST_ADD(FFBatteryResult, *results);
    initResult(battery);

    if (ffParsePropLines(start, "AC powered: ", &temp) && ffStrbufEqualS(&temp, "true")) {
        battery->status |= FF_BATTERY_STATUS_AC_CONNECTED;
    }
    ffStrbufClear(&temp);

    if (ffParsePropLines(start, "USB powered: ", &temp) && ffStrbufEqualS(&temp, "true")) {
        battery->status |= FF_BATTERY_STATUS_USB_CONNECTED;
    }
    ffStrbufClear(&temp);

    if (ffParsePropLines(start, "Wireless powered: ", &temp) && ffStrbufEqualS(&temp, "true")) {
        battery->status |= FF_BATTERY_STATUS_WIRELESS_CONNECTED;
    }
    ffStrbufClear(&temp);

    {
        double level = 0, scale = 0;
        if (ffParsePropLines(start, "level: ", &temp)) {
            level = ffStrbufToDouble(&temp, -DBL_MAX);
        }
        ffStrbufClear(&temp);

        if (ffParsePropLines(start, "scale: ", &temp)) {
            scale = ffStrbufToDouble(&temp, -DBL_MAX);
        }
        ffStrbufClear(&temp);

        if (level > 0 && scale > 0) {
            battery->capacity = level * 100 / scale;
        }
    }

    if (options->temp) {
        if (ffParsePropLines(start, "temperature: ", &temp)) {
            battery->temperature = ffStrbufToDouble(&temp, FF_BATTERY_TEMP_UNSET);
            if (battery->temperature != FF_BATTERY_TEMP_UNSET) {
                battery->temperature /= 10.0; // Android returns temperature in tenths of a degree
            }
        }
        ffStrbufClear(&temp);
    }

    ffParsePropLines(start, "technology: ", &battery->technology);

    return nullptr;
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results) {
    // The binder route needs no permission and costs ~0.3 ms. `dumpsys battery` is kept as a
    // fallback for `adb shell` and rooted environments, where the binder route also works but the
    // extra fields it reports are worth having. termux-api was dropped: it returns nothing on this
    // device and can hang for minutes, the same reason its camera path was removed.
    const char* error = parseBinder(results);
    if (error != nullptr && parseDumpsys(options, results) == nullptr) {
        return nullptr;
    }
    return error;
}
