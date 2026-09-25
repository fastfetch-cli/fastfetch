#include "fastfetch.h"
#include "battery.h"
#include "common/android/api.h"
#include "common/android/binder.h"
#include "common/debug.h"
#include "common/processing.h"
#include "common/properties.h"

#include <inttypes.h>

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

// One `getProperty` reply is [exception code][return value][value present][int64 mValueLong], and it
// measures 24 bytes on the device: API 35 appended [string8 mValueString] to `BatteryProperty`, and a
// reply carries a word for it even when it is empty. A dump of the first 32 covers every word.
#define FF_BATTERY_ANDROID_DEBUG_DUMP_SIZE 32

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

#ifndef NDEBUG
// Every word of the reply is read at a fixed offset, so a reply whose fields do not line up is
// otherwise undiagnosable: the bytes are the only evidence. Each line carries the words next to their
// bytes, because what the reader is looking for -- the exception code, the return value, the long --
// is an int, and the long straddles the last word of the first line and the first of the second.
static void debugDumpReply(const uint8_t* data, size_t size) {
    const size_t limit = size < FF_BATTERY_ANDROID_DEBUG_DUMP_SIZE ? size : FF_BATTERY_ANDROID_DEBUG_DUMP_SIZE;
    for (size_t offset = 0; offset < limit; offset += 16) {
        char hex[16 * 3 + 1];
        size_t used = 0;
        for (size_t i = 0; i < 16 && offset + i < limit; ++i) {
            used += (size_t) snprintf(hex + used, sizeof(hex) - used, "%02x ", data[offset + i]);
        }
        FF_DEBUG("  +0x%02zx  %-47s | %d %d %d %d", offset, hex,
            ffBinderReadI32(data, size, offset), ffBinderReadI32(data, size, offset + 4),
            ffBinderReadI32(data, size, offset + 8), ffBinderReadI32(data, size, offset + 12));
    }
}
    #define FF_BATTERY_ANDROID_DEBUG_DUMP(data, size) debugDumpReply(data, size)
#else
    #define FF_BATTERY_ANDROID_DEBUG_DUMP(data, size) ((void) 0)
#endif

static const char* getProperty(FFBinder* binder, uint32_t handle, uint32_t property, uint64_t* value) {
    // `getProperty` is the third method declared in `IBatteryPropertiesRegistrar` up to Android 9 and
    // the first one from Android 10 on. Written as an `if` because clang rejects `__builtin_available`
    // -- which is what FF_ANDROID_API_AT_LEAST expands to -- in any other position.
    uint32_t transaction = 3u;
    if (FF_ANDROID_API_AT_LEAST(29)) {
        transaction = 1u;
    }
    FF_DEBUG("Property %u goes out as transaction %u (3 up to Android 9, 1 from Android 10)", property, transaction);

    uint8_t parcelBuffer[128];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_BATTERY_ANDROID_DESCRIPTOR);
    ffBinderParcelPutU32(&parcel, property);

    uint8_t replyBuffer[128];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    const char* error = ffBinderTransact(binder, handle, transaction, 0, &parcel, &reply);
    if (error != nullptr) {
        FF_DEBUG("Property %u could not be transacted: %s", property, error);
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("Property %u came back as a status reply of %d instead of a parcel",
            property, (int) ffBinderReadI32(reply.data, reply.size, 0));
        return "Battery service rejected the request";
    }

    // The three words AIDL writes in front of the value, named here rather than read inline so that
    // the trace below can print them: the exception code, the method's own `int` return value (which
    // is what the service sets when it does not know the property) and the non-null marker of the out
    // parameter. A marker of 0 means the value behind it was never written.
    const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
    const int32_t returnValue = ffBinderReadI32(reply.data, reply.size, 4);
    FF_DEBUG("Property %u: reply is %zu bytes, exception %d, return value %d, value present %d",
        property, reply.size, exception, returnValue, ffBinderReadI32(reply.data, reply.size, 8));
    if (exception != 0) {
        return "Battery service raised an exception";
    }
    if (returnValue != 0) {
        FF_DEBUG("Property %u is not one this HAL implements, so the words below are not a value:",
            property);
        FF_BATTERY_ANDROID_DEBUG_DUMP(reply.data, reply.size);
        return "Battery service does not report this property";
    }

    const uint64_t result = ffBinderReadU64(reply.data, reply.size, FF_BATTERY_ANDROID_VALUE_OFFSET);
    FF_DEBUG("Property %u is %" PRIu64 " (0x%" PRIx64 ") at +0x%02x",
        property, result, result, FF_BATTERY_ANDROID_VALUE_OFFSET);
    if (result == FF_BATTERY_ANDROID_VALUE_UNSET) {
        FF_DEBUG("Property %u is Long.MIN_VALUE, which is what the service leaves behind when it never "
                 "fills the field in, so there is nothing to report:",
            property);
        FF_BATTERY_ANDROID_DEBUG_DUMP(reply.data, reply.size);
        return "Battery service left this property empty";
    }

    *value = result;
    return nullptr;
}

static const char* parseBinder(FFlist* results) {
    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    const char* error = ffBinderOpen(&binder);
    if (error != nullptr) {
        FF_DEBUG("Opening /dev/binder failed: %s", error);
        return error;
    }

    // Released on every path out of this function, including the ones below that return early.
    [[gnu::cleanup(ffBinderServiceHandleRelease)]] FFBinderServiceHandle service = { .binder = &binder };
    error = ffBinderLookupService(&binder, FF_BATTERY_ANDROID_SERVICE, FF_BINDER_SM_GET_SERVICE, &service.handle);
    if (error != nullptr) {
        FF_DEBUG("Looking up the \"%s\" service failed: %s", FF_BATTERY_ANDROID_SERVICE, error);
        return error;
    }
    FF_DEBUG("The \"%s\" service is handle %u", FF_BATTERY_ANDROID_SERVICE, service.handle);

    uint64_t capacity = 0;
    error = getProperty(&binder, service.handle, FF_BATTERY_ANDROID_PROPERTY_CAPACITY, &capacity);
    if (error != nullptr) {
        FF_DEBUG("The capacity is what failed, so no battery is reported at all");
        return error;
    }

    uint64_t status = 0;
    error = getProperty(&binder, service.handle, FF_BATTERY_ANDROID_PROPERTY_STATUS, &status);
    if (error != nullptr) {
        FF_DEBUG("The status is what failed, so no battery is reported at all");
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
    // `BatteryStatus` has five values and only two of them say something the powered bits do not, so
    // the trace spells the mapping out: anything that is not 2 or 3 leaves the bits clear, which is
    // what UNKNOWN (1), NOT_CHARGING (4) and FULL (5) all come to.
    FF_DEBUG("Reporting one battery: capacity %.0f%%, status %" PRIu64 " -> bits 0x%x "
             "(2 charging, 3 discharging; 1/4/5 leave them clear). The registrar answers no "
             "temperature and no technology, so both stay unset",
        battery->capacity, status, (unsigned) battery->status);
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
