#include "fastfetch.h"
#include "battery.h"
#include "common/android/api.h"
#include "common/android/binder.h"
#include "common/debug.h"
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
typedef enum FFBatteryAndroidProperty : uint32_t {
    FF_BATTERY_ANDROID_PROPERTY_CAPACITY = 4,
    FF_BATTERY_ANDROID_PROPERTY_STATUS = 6,
} FFBatteryAndroidProperty;

// BatteryManager.BATTERY_STATUS_*, which the health HAL's `BatteryStatus` mirrors value for value.
// Only the three the module has something to say about are carried: NOT_CHARGING (4) and FULL (5)
// both mean a charger is attached, which the powered bits already say.
typedef enum FFBatteryAndroidStatus : uint32_t {
    FF_BATTERY_ANDROID_STATUS_UNKNOWN = 1,
    FF_BATTERY_ANDROID_STATUS_CHARGING = 2,
    FF_BATTERY_ANDROID_STATUS_DISCHARGING = 3,
} FFBatteryAndroidStatus;

// BatteryCapacityLevel.BATTERY_CAPACITY_LEVEL_CRITICAL, the level the framework itself shuts the
// device down on. This is the health HAL's numbering, not the one the older
// `BatteryManager.BATTERY_CAPACITY_LEVEL_*` constants used, which had CRITICAL at 4. The rest of the
// enum -- -1 UNSUPPORTED, 0 UNKNOWN, 2 LOW, 3 NORMAL, 4 HIGH, 5 FULL -- only restates the
// percentage, so only this value is carried.
typedef enum FFBatteryAndroidCapacityLevel : int32_t {
    FF_BATTERY_ANDROID_CAPACITY_LEVEL_CRITICAL = 1,
} FFBatteryAndroidCapacityLevel;

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
    // -- which is what FF_ANDROID_API_AT_LEAST expands to -- in any other position.
    uint32_t transaction = 3u;
    if (FF_ANDROID_API_AT_LEAST(29)) {
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

// `dumpsys battery` prints what `BatteryService.dumpInternal()` prints: the whole of the health HAL's
// `HealthInfo`, plus the timestamps the service keeps for its own broadcast rate limiter. It is a flat
// `key: value` list read by name, which is what makes it the sturdier of the two routes -- a vendor
// that prints extra keys (some vendors do: `engine`, `soc decimal`, `adapter power`, `board temp status`,
// `low bat status`, `reverse wl chg status`, `reverse wl chg exception`, `chg shut vbat`,
// `last mode flag` and `last mode keep time`, in the middle of the list) only adds keys nobody asks
// for, and a release that appends a field costs nothing. The binder reply is positional and breaks
// silently instead.
//
// What each key carries, per `android.hardware.health.HealthInfo`:
//
//   AC / USB / Wireless / Dock powered -- `charger*Online`, the `plugged` bits of the
//     ACTION_BATTERY_CHANGED broadcast. Only the first three have a `FF_BATTERY_STATUS_*`
//     counterpart; `Dock powered` (`BATTERY_PLUGGED_DOCK`) has none and is not read.
//   Max charging current / Max charging voltage -- `maxChargingCurrentMicroamps` (µA) and
//     `maxChargingVoltageMicrovolts` (µV): what the charger offers, not what the battery is drawing.
//   Charge counter -- `batteryChargeCounterUah` (µAh), what is left in the pack. The unit is not
//     honoured everywhere: MIUI reports the same number as its own dump and three orders of
//     magnitude below what the registrar answers for the same battery.
//   status -- `batteryStatus`, i.e. `BatteryStatus`: 1 UNKNOWN, 2 CHARGING, 3 DISCHARGING,
//     4 NOT_CHARGING (a charger is attached and the battery is deliberately not taking current),
//     5 FULL.
//   health -- `batteryHealth`, i.e. `BatteryHealth`: 1 UNKNOWN, 2 GOOD, 3 OVERHEAT, 4 DEAD,
//     5 OVER_VOLTAGE, 6 UNSPECIFIED_FAILURE, 7 COLD, and, in newer releases, 8 FAIR, 11 NOT_AVAILABLE
//     and 12 INCONSISTENT.
//   present -- `batteryPresent`, whether the pack is in the device at all.
//   level / scale -- `batteryLevel`, the remaining capacity in percent, and `BATTERY_SCALE`, the value
//     100% is expressed in. The scale has been 100 on every release, but it is printed rather than
//     assumed, so it is read.
//   voltage -- `batteryVoltageMillivolts`. The unit in the name is the recent one: the HAL 1.0 field
//     was `batteryVoltage` in microvolts, and the AIDL notes that implementations had always filled it
//     in millivolts, so the rename only wrote down the de-facto unit.
//   temperature -- `batteryTemperatureTenthsCelsius`, tenths of a degree Celsius.
//   technology -- `batteryTechnology`, e.g. "Li-poly". The registrar answers no such property, so
//     together with the temperature this is what the route is here for.
//   Charging state -- `chargingState`, i.e. `BatteryChargingState`: 0 INVALID, 1 NORMAL, 2 TOO_COLD,
//     3 TOO_HOT, 4 LONG_LIFE, 5 ADAPTIVE.
//   Charging policy -- `chargingPolicy`, i.e. `BatteryChargingPolicy`: 0 INVALID, 1 DEFAULT,
//     2 LONG_LIFE, 3 ADAPTIVE. 0 in either means the HAL did not report it.
//   Capacity level -- `batteryCapacityLevel`, i.e. `BatteryCapacityLevel`: -1 UNSUPPORTED,
//     0 UNKNOWN, 1 CRITICAL, 2 LOW, 3 NORMAL, 4 HIGH, 5 FULL.
//
// Three keys are not battery readings at all and are skipped: the two `Time when the latest updated
// value of the ... was sent via battery changed broadcast` lines and `The last voltage value sent via
// the battery changed broadcast` are what `BatteryService` keeps to rate limit the broadcast.
//
// `FFBatteryResult` has a place for the status enum, the level pair, the temperature and the
// technology, and for the three powered booleans and the capacity level in the form of status bits.
// `voltage`, `health`, `Charge counter` and the two maximums are read by nothing -- there is no field
// to put them in -- and are described above so that the next reader does not have to go back to the
// service to find out what they are.
static const char* parseDumpsys(FFBatteryOptions* options, FFlist* results) {
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buf, (char*[]) {
                                        "/system/bin/dumpsys",
                                        "battery",
                                        nullptr,
                                    }) != nullptr ||
        buf.length == 0) {
        return "Executing `/system/bin/dumpsys battery` failed";
    }

    if (!ffStrbufStartsWithS(&buf, "Current Battery Service state:\n")) {
        // A refusal, which ffDetectBattery has already ruled out by looking at the UID: an app UID gets
        // `Can't find service: battery` on Android 16, and the permission denial some other
        // release would print lands here too. Both exit 0, so this is the only place that notices.
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

    // NOT_CHARGING and FULL both mean a charger is attached, which the powered bits already say, and
    // neither is charging or discharging in the sense `FF_BATTERY_STATUS` means. UNKNOWN is mapped
    // because it is a different statement from the service not answering at all.
    if (ffParsePropLines(start, "status: ", &temp)) {
        switch (ffStrbufToUInt(&temp, 0)) {
            case FF_BATTERY_ANDROID_STATUS_CHARGING:
                battery->status |= FF_BATTERY_STATUS_CHARGING;
                break;
            case FF_BATTERY_ANDROID_STATUS_DISCHARGING:
                battery->status |= FF_BATTERY_STATUS_DISCHARGING;
                break;
            case FF_BATTERY_ANDROID_STATUS_UNKNOWN:
                battery->status |= FF_BATTERY_STATUS_UNKNOWN;
                break;
            default:
                break;
        }
    }
    ffStrbufClear(&temp);

    // CRITICAL is the one capacity level that says something the percentage does not. -1 (UNSUPPORTED)
    // and 0 (UNKNOWN) are what a HAL that does not implement the property reports, and 2 to 5 only
    // restate the level.
    if (ffParsePropLines(start, "Capacity level: ", &temp) &&
        ffStrbufToSInt(&temp, -1) == FF_BATTERY_ANDROID_CAPACITY_LEVEL_CRITICAL) {
        battery->status |= FF_BATTERY_STATUS_CRITICAL;
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
    // `dumpsys battery` needs android.permission.DUMP, which only the shell UID and root hold: an app
    // UID is answered with `Can't find service: battery` on stdout and a zero exit status, so forking
    // it there costs a child process and cannot succeed. It is also the richer of the two routes --
    // the registrar answers neither temperature nor technology, and the dump is what spells out the
    // status enum -- so it is tried first wherever it is allowed to run, and the binder route covers
    // everything else at ~0.3 ms. termux-api was dropped: it returns nothing on this device and can
    // hang for minutes, the same reason its camera path was removed.
    if (ffAndroidIsRootOrShell(instance.state.platform.uid)) {
        const char* error = parseDumpsys(options, results);
        if (error == nullptr) {
            return nullptr;
        }
        FF_DEBUG("`/system/bin/dumpsys battery` failed: %s. Using the binder route instead", error);
    }

    return parseBinder(results);
}
