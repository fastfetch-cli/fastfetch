#include "fastfetch.h"
#include "battery.h"
#include "common/android/api.h"
#include "common/android/binder.h"
#include "common/android/dex.h"
#include "common/debug.h"
#include "common/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "common/settings.h"
#include "common/strutil.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>

// Android exposes the battery through BatteryManager, which the NDK does not wrap and whose sysfs
// backing is unreadable from an app UID -- /sys/class/power_supply is closed to untrusted_app
// outright, directory and all. What does work without any permission is
// android.os.IBatteryPropertiesRegistrar in system_server, over /dev/binder (see
// common/android/binder.h).
//
// `dumpsys battery` is the other route, tried first for shell and root because it is the richer of
// the two: a flat `key: value` list read by name, so it carries what neither the registrar nor a
// property has -- the technology and the critical capacity level -- and neither a vendor that prints
// extra keys in the middle nor a release that appends a field can break it. The binder reply is
// positional and breaks silently instead. What it costs is a fork/exec, which is why it is only
// reached where it can succeed; see the UID gate in ffDetectBattery.
//
// An app UID gets none of that, and not because Android withholds it. `ACTION_BATTERY_CHANGED` is a
// sticky broadcast, and a sticky broadcast is read without a permission: it carries the technology,
// and a cycle count since Android 14 -- the device's own BatteryManager declares `EXTRA_CYCLE_COUNT`,
// spelled "android.os.extra.CYCLE_COUNT". What is out of reach is the call that fetches it. A sticky
// broadcast is read by `registerReceiver`, the NDK does not wrap it, and a native binary has no Java
// side to make that call from, so every route to it is a Java process. termux-api is one, and it is
// not the answer: on the device this was written on it hung on six of six runs, each killed at 30
// seconds having written nothing, after exec'ing `am broadcast` and waiting on a socket that has no
// timeout of its own.
//
// What is left within reach is a pair of system properties BatteryService mirrors out of every update
// it processes: `debug.tracing.plug_type` and `debug.tracing.battery_status`. A system property is
// world readable, which is the whole point of them here -- the charger bits used to need `dumpsys
// battery`, and so the shell UID, while the status used to cost a second transaction. Both are now
// free, to any UID; see the two defines below.
//
// The temperature comes from neither service. The kernel publishes it as a thermal zone, and
// /sys/class/thermal is readable from an app UID -- unlike /sys/class/power_supply, which is closed
// outright, directory and all -- so it is read there whenever `temp` asks for it. The dump has a
// `temperature` line of its own and that one wins where the dump runs; see `detectTemperature` below
// for why the zone is looked up by name rather than addressed by number.
//
// The registrar answers nothing else. `BatteryProperty` has no temperature, no technology and no
// cycle count, and manufacturer and model name are not battery data at all. Of the properties that
// do exist, ids 7 to 12 -- the ones Android 15 added -- are behind BATTERY_STATS, and ids 1 to 6 are
// not. The one reading the module wants that is not a property either, how long the charge will
// last, comes from a different service; see the `batterystats` block below.

// The reply layout is positional and is not negotiated with the service, so it only stays correct as
// long as AOSP keeps the order it already has. It has, because `BatteryProperty` has only ever grown
// at the end -- API 35 appended [string8 mValueString] to it -- so the field read below is still the
// first one written.
//
// The transaction code is positional for the same reason, and it has *not* stayed: the interface
// still declared registerListener / unregisterListener / getProperty up to Android 9, and Android 10
// dropped the two listener methods, which moved `getProperty` from the third position to the first.
// The number is therefore read out of the device's own jar rather than guessed from the API level,
// because a vendor fork is free to insert a method ahead of it -- and a wrong code reaches a
// different method, or none. See common/android/dex.h.
//
// Getting either wrong is quiet rather than loud: `ffBinderReadU64` returns 0 for a read past the
// end of the reply, so a drift shows up as a wrong capacity, not as a failed call.
#define FF_BATTERY_ANDROID_SERVICE "batteryproperties"
#define FF_BATTERY_ANDROID_DESCRIPTOR "android.os.IBatteryPropertiesRegistrar"

// IBatteryPropertiesRegistrar has always been part of framework.jar -- it is not one of the
// frameworks that moved into an APEX -- so there is one path and no fallback. The method this module
// calls is named by the constant the dex carries for it, which is what the lookup turns into whatever
// number this build uses.
#define FF_BATTERY_ANDROID_JAR "/system/framework/framework.jar"
#define FF_BATTERY_ANDROID_STUB "Landroid/os/IBatteryPropertiesRegistrar$Stub;"
#define FF_BATTERY_ANDROID_GET_PROPERTY "TRANSACTION_getProperty"

// How long the charge will last is not a property of the pack: it is a forecast BatteryStatsService
// makes from the discharge history, so it lives on a second service. That service is `batterystats`,
// whose interface is com.android.internal.app.IBatteryStats, and it is also in framework.jar --
// com.android.internal.app is not one of the namespaces that moved into an APEX -- so the same read
// resolves this code as the one above.
//
// The interface has 120 methods and the dex cannot say which of them an app UID may call: neither
// IBatteryStats nor its $Stub carries an `_enforcePermission` wrapper, so the wall is inside
// BatteryStatsService, which lives in services.jar and is not available to read. It was therefore
// measured, as uid 10478 on an Android 16 device. The three this module needs answer with no
// permission at all -- and so do `getAllWakeLocks` (117) and `getAllWakeLocksEx` (118), which is why
// the claim is not "only three of 120". Every read-only statistic getter tried (`getAwakeTimeBattery`,
// `getAwakeTimePlugged`, and the cellular / wifi / gps / bluetooth ones) came back with
// "Access denied, requires: android.permission.BATTERY_STATS", or an `anyOf` of that and
// UPDATE_DEVICE_STATS.
//
// The code is read out of the jar for the same reason as `getProperty`, and the numbers make the
// point better than the argument does: this method is transaction 19 on that Android 16 device, 22
// on a Mi 10 (Android 14) and 20 on a Mi 9 (Android 10). A hardcoded 19 would reach a different
// method on two of the three.
#define FF_BATTERY_ANDROID_STATS_SERVICE "batterystats"
#define FF_BATTERY_ANDROID_STATS_DESCRIPTOR "com.android.internal.app.IBatteryStats"
#define FF_BATTERY_ANDROID_STATS_STUB "Lcom/android/internal/app/IBatteryStats$Stub;"
#define FF_BATTERY_ANDROID_COMPUTE_BATTERY_TIME_REMAINING "TRANSACTION_computeBatteryTimeRemaining"

// The temperature is a third source again, and not a service this time: the kernel publishes it as a
// thermal zone, and the zone Android's own thermal HAL names `battery` is the one to read. The name
// is what is matched -- not the number, and not a prefix -- for the reasons spelled out in
// detectTemperature below.
#define FF_BATTERY_ANDROID_THERMAL_DIR "/sys/class/thermal/"
#define FF_BATTERY_ANDROID_THERMAL_ZONE_PREFIX "thermal_zone"
#define FF_BATTERY_ANDROID_THERMAL_BATTERY_TYPE "battery"

// BatteryManager.BATTERY_PROPERTY_*
typedef enum FFBatteryAndroidProperty : uint32_t {
    FF_BATTERY_ANDROID_PROPERTY_CAPACITY = 4,
    FF_BATTERY_ANDROID_PROPERTY_STATUS = 6,
} FFBatteryAndroidProperty;

// BatteryManager.BATTERY_STATUS_*, which the health HAL's `BatteryStatus` mirrors value for value.
// The module has something to say about only three of the five: NOT_CHARGING (4) and FULL (5) both
// mean a charger is attached, and it answers nothing about the charger to put that on, while the dump
// route maps UNKNOWN (1) as well, because there it is a different statement from the service not
// answering at all. The two are named anyway, because the property route validates the whole range
// before it trusts a value it did not get from the service.
typedef enum FFBatteryAndroidStatus : uint32_t {
    FF_BATTERY_ANDROID_STATUS_UNKNOWN = 1,
    FF_BATTERY_ANDROID_STATUS_CHARGING = 2,
    FF_BATTERY_ANDROID_STATUS_DISCHARGING = 3,
    FF_BATTERY_ANDROID_STATUS_NOT_CHARGING = 4,
    FF_BATTERY_ANDROID_STATUS_FULL = 5,
} FFBatteryAndroidStatus;

// The pair of system properties that stand in for two of the readings above. Both are published by
// `BatteryService.processValuesLocked` -- the two strings sit next to each other in the constant pool
// of classes.dex in this device's services.jar, which is what a pair of `SystemProperties.set` calls
// looks like after the fact -- and neither is a `BatteryProperty`, so nothing about the registrar's
// positional reply applies to them.
//
//   debug.tracing.plug_type      -- BatteryManager.BATTERY_PLUGGED_*: 1 AC, 2 USB, 4 wireless,
//                                  8 dock, and 0 when nothing is attached. The dock bit has no
//                                  `FF_BATTERY_STATUS_*` counterpart and is masked off below.
//   debug.tracing.battery_status -- BatteryManager.BATTERY_STATUS_*, the value
//                                  `getProperty(BATTERY_PROPERTY_STATUS)` answers with as well:
//                                  1 UNKNOWN, 2 CHARGING, 3 DISCHARGING, 4 NOT_CHARGING, 5 FULL.
#define FF_BATTERY_ANDROID_PROP_PLUG_TYPE "debug.tracing.plug_type"
#define FF_BATTERY_ANDROID_PROP_BATTERY_STATUS "debug.tracing.battery_status"

// The encoding of plug_type, read out of the device's own framework.jar rather than assumed: the
// values are bits of one field and not a sequence, so `value != 0` is not a test for "wired", and a
// device reporting the dock would be told it is on neither AC nor USB nor wireless.
typedef enum FFBatteryAndroidPlugged : uint32_t {
    FF_BATTERY_ANDROID_PLUGGED_AC = 1,
    FF_BATTERY_ANDROID_PLUGGED_USB = 2,
    FF_BATTERY_ANDROID_PLUGGED_WIRELESS = 4,
} FFBatteryAndroidPlugged;

// BatteryCapacityLevel.BATTERY_CAPACITY_LEVEL_CRITICAL, the level the framework itself shuts the
// device down on. This is the health HAL's numbering, not the one the older
// `BatteryManager.BATTERY_CAPACITY_LEVEL_*` constants used, which had CRITICAL at 4. The rest of the
// enum is skipped: -1 (UNSUPPORTED) and 0 (UNKNOWN) are what a HAL that does not implement the
// property reports, and 2 to 5 only restate the percentage. Only the dump carries this -- it is not a
// `BatteryProperty`, so the binder route never sees it.
typedef enum FFBatteryAndroidCapacityLevel : int32_t {
    FF_BATTERY_ANDROID_CAPACITY_LEVEL_CRITICAL = 1,
} FFBatteryAndroidCapacityLevel;

// BatteryProperty starts with [int64 mValueLong], behind the usual [exception code][return value]
// [out-param non-null marker] prefix of an AIDL reply. API 35 appended [string8 mValueString] after
// the long for the one string valued property (`BATTERY_PROPERTY_SERIAL_NUMBER`), which does not
// move the long, so one offset covers every release.
#define FF_BATTERY_ANDROID_VALUE_OFFSET 12
#define FF_BATTERY_ANDROID_VALUE_UNSET 0x8000000000000000ULL // Long.MIN_VALUE, i.e. never filled in

// A method that answers with a bare `long` writes [int32 exception][int64 value] and nothing else:
// there is no out parameter, so no non-null marker sits in front of the value the way it does in a
// `getProperty` reply. Twelve bytes in all on the device.
#define FF_BATTERY_ANDROID_LONG_VALUE_OFFSET 4

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

static const char* getProperty(FFBinder* binder, uint32_t handle, uint32_t transaction, uint32_t property, uint64_t* value) {
    FF_DEBUG("Property %u goes out as transaction %u", property, transaction);

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

// Reads either of the two properties above as a number. Both are written by `Integer.toString`, so a
// buffer whose text is not a number is the property saying nothing rather than a 0 -- 0 is a plug type,
// and it is not a status.
static bool readPropertyUInt(const char* name, uint32_t* value) {
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (!ffSettingsGetAndroidProperty(name, &buffer)) {
        FF_DEBUG("The \"%s\" property is not set, so nothing is read from it", name);
        return false;
    }

    // `ffStrbufToUInt` answers the default for text that does not start with a digit, which is why the
    // default is a value neither property can hold rather than a 0.
    const uint64_t parsed = ffStrbufToUInt(&buffer, UINT64_MAX);
    if (parsed == UINT64_MAX) {
        FF_DEBUG("The \"%s\" property reads \"%s\", which is not a number", name, buffer.chars);
        return false;
    }

    *value = (uint32_t) parsed;
    return true;
}

// The charger type. This is what the property route has and the registrar does not: `getProperty`
// answers nothing about the charger, so the three bits used to come from `dumpsys battery`, i.e. from
// the shell UID and root alone. A device that does not publish the property leaves the bits clear,
// which is also what an unplugged charger looks like -- the module cannot tell those apart, and neither
// can the framework, whose sticky broadcast carries this same value as `EXTRA_PLUGGED`.
static void fillPlugged(FFBatteryResult* battery) {
    uint32_t plugged = 0;
    if (!readPropertyUInt(FF_BATTERY_ANDROID_PROP_PLUG_TYPE, &plugged)) {
        return;
    }

    // A mask rather than a comparison: the three are bits of one field, and it is the bits that have a
    // `FF_BATTERY_STATUS_*` counterpart. BATTERY_PLUGGED_DOCK (8) is the one that does not, so a device
    // on a dock reports no powered bit at all -- and `plugged != 0` would have called that "wired".
    if (plugged & FF_BATTERY_ANDROID_PLUGGED_AC) {
        battery->status |= FF_BATTERY_STATUS_AC_CONNECTED;
    }
    if (plugged & FF_BATTERY_ANDROID_PLUGGED_USB) {
        battery->status |= FF_BATTERY_STATUS_USB_CONNECTED;
    }
    if (plugged & FF_BATTERY_ANDROID_PLUGGED_WIRELESS) {
        battery->status |= FF_BATTERY_STATUS_WIRELESS_CONNECTED;
    }

    FF_DEBUG("The \"%s\" property is %u, which becomes status bits 0x%x (1 AC, 2 USB, 4 wireless; 0 is "
             "what both an unplugged charger and a device that does not publish the property look like)",
        FF_BATTERY_ANDROID_PROP_PLUG_TYPE, plugged, (unsigned) battery->status);
}

// The status, read the cheap way first. The property carries the same `mHealthInfo.batteryStatus` the
// transaction would fetch, so the two can only differ inside the window between the health HAL reporting
// a value and the service storing it, while what the property costs is a read of a shared page instead
// of a round trip through /dev/binder -- a round trip measured at about 0.04 ms on the device this was
// written on, and what it saves is a positional reply that has to be decoded as well.
//
// It stays a preference rather than a replacement: a property that is not set (an early boot, or a
// build whose BatteryService does not publish it) or that does not carry one of the five values sends
// the caller to the service, which is the one answer that cannot be stale.
static bool detectStatusFromProperty(uint64_t* status) {
    uint32_t value = 0;
    if (!readPropertyUInt(FF_BATTERY_ANDROID_PROP_BATTERY_STATUS, &value)) {
        return false;
    }
    if (value < FF_BATTERY_ANDROID_STATUS_UNKNOWN || value > FF_BATTERY_ANDROID_STATUS_FULL) {
        FF_DEBUG("The \"%s\" property is %u, which is not a battery status, so the service is asked",
            FF_BATTERY_ANDROID_PROP_BATTERY_STATUS, value);
        return false;
    }

    *status = value;
    return true;
}

// Calls a method that takes no argument and answers with one `long`, which is how
// IBatteryStats.computeBatteryTimeRemaining() is declared. `transaction` comes out of the jar, and the
// sentinel the reader leaves for a code it could not find is caught here, so that a build which does
// not declare the method is a message rather than a transaction that means something else.
//
// A negative answer is not an error: it is how the interface spells "no estimate", so it is passed
// back as it came and the caller decides what that is worth. The unit is milliseconds.
static const char* getTimeRemaining(FFBinder* binder, uint32_t handle, int32_t transaction, int64_t* milliseconds) {
    if (transaction == FF_DEX_STATIC_INT_UNRESOLVED) {
        return "The battery stats service does not declare computeBatteryTimeRemaining";
    }

    uint8_t parcelBuffer[128];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_BATTERY_ANDROID_STATS_DESCRIPTOR);

    uint8_t replyBuffer[128];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    const char* error = ffBinderTransact(binder, handle, (uint32_t) transaction, 0, &parcel, &reply);
    if (error != nullptr) {
        FF_DEBUG("computeBatteryTimeRemaining could not be transacted: %s", error);
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("computeBatteryTimeRemaining came back as a status reply of %d instead of a parcel",
            (int) ffBinderReadI32(reply.data, reply.size, 0));
        return "The battery stats service rejected the request";
    }

    // The exception word is read first and the value only once it is known to be a value: a denied
    // call answers [exception -1][string16 message], so the eight bytes at the value's offset are
    // then the length and the first characters of the message rather than a number.
    const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
    if (exception != 0) {
        FF_DEBUG("computeBatteryTimeRemaining is transaction %d, reply %zu bytes, exception %d",
            transaction, reply.size, exception);
        return "The battery stats service raised an exception";
    }

    *milliseconds = (int64_t) ffBinderReadU64(reply.data, reply.size, FF_BATTERY_ANDROID_LONG_VALUE_OFFSET);
    FF_DEBUG("computeBatteryTimeRemaining is transaction %d: reply is %zu bytes, value %" PRId64 " ms",
        transaction, reply.size, *milliseconds);
    return nullptr;
}

// Writes the time estimate onto the battery just reported. It is not one of the registrar's
// properties -- it is a forecast BatteryStatsService makes -- so it comes from a second service and
// is filled in here, after the capacity and the status are already in place.
//
// `binder` and `handle` are the caller's, already open on `batterystats`. Nothing here can fail the
// module: an estimate that is not available is what `FFBatteryResult` already spells with -1.
static void fillTimeRemaining(FFBinder* binder, uint32_t handle, int32_t transaction, FFlist* results) {
    if (results->length == 0) {
        FF_DEBUG("No time remaining is reported: no battery was reported to put it on");
        return;
    }

    int64_t milliseconds = 0;
    const char* error = getTimeRemaining(binder, handle, transaction, &milliseconds);
    if (error != nullptr) {
        FF_DEBUG("The time remaining could not be computed, so none is reported: %s", error);
        return;
    }
    if (milliseconds <= 0) {
        FF_DEBUG("The battery stats service answered %" PRId64 " ms, which is how it says it has no estimate",
            milliseconds);
        return;
    }

    // The interface answers in milliseconds and `FFBatteryResult` carries seconds, which is what the
    // other platform modules fill in. An answer under a second truncates to 0, which the module reads
    // as "unknown" as well, so there is nothing to guard against.
    FF_LIST_GET(FFBatteryResult, *results, 0)->timeRemaining = (int32_t) (milliseconds / 1000);
}

// The battery temperature is neither a registrar property nor part of the health HAL: the kernel
// publishes it as a thermal zone, and the zone Android's thermal HAL names `battery` is the one to
// read. It is read from /sys/class/thermal, which an app UID may open -- the reason the module does
// not simply read /sys/class/power_supply/battery/temp is that that directory is closed to it.
//
// The zone is *found*, not addressed. Its number is a property of the probe order rather than of the
// hardware -- the battery is `thermal_zone67` on the device this was written on, and nothing promises
// the next one -- so the directory is walked and each entry is matched on the `type` it reports.
//
// That match is an exact string, and has to stay one. A prefix would be cheaper and wrong: the same
// directory holds hardware trip points (`cpu-hw-trip-0` reports 95000, a perfectly believable 95
// degrees), current and battery levels (`pmih010x-ibat-lvl0`, 0 or a small integer) and raw registers
// (`vbat`, in millivolts). None of those are a temperature, and every one of them would be printed as
// one. Reading a different zone is not a smaller error than reading none.
//
// A device that names its battery zone something else therefore reports no temperature rather than a
// wrong one, which is what FF_BATTERY_TEMP_UNSET already means.
static double detectTemperature(void) {
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(FF_BATTERY_ANDROID_THERMAL_DIR);
    if (dirp == nullptr) {
        FF_DEBUG("Opening \"%s\" failed, so no temperature is reported: %s", FF_BATTERY_ANDROID_THERMAL_DIR, strerror(errno));
        return FF_BATTERY_TEMP_UNSET;
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    struct dirent* entry;
    while ((entry = readdir(dirp)) != nullptr) {
        if (!ffStrStartsWith(entry->d_name, FF_BATTERY_ANDROID_THERMAL_ZONE_PREFIX)) {
            continue;
        }

        FF_AUTO_CLOSE_FD int zone = openat(dirfd(dirp), entry->d_name, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        if (zone < 0) {
            continue;
        }

        // The type file ends with a newline, which the comparison below would otherwise never survive.
        if (!ffReadFileBufferRelative(zone, "type", &buffer)) {
            continue;
        }
        ffStrbufTrimRightSpace(&buffer);
        if (!ffStrbufEqualS(&buffer, FF_BATTERY_ANDROID_THERMAL_BATTERY_TYPE)) {
            continue;
        }

        if (!ffReadFileBufferRelative(zone, "temp", &buffer)) {
            FF_DEBUG("The battery zone \"%s\" has no readable temperature", entry->d_name);
            return FF_BATTERY_TEMP_UNSET;
        }

        // sysfs thermal publishes millidegrees Celsius. That is the one thing every zone here agrees
        // on, even the ones whose reading is not a temperature at all.
        const double value = ffStrbufToDouble(&buffer, FF_BATTERY_TEMP_UNSET);
        if (value == FF_BATTERY_TEMP_UNSET) {
            FF_DEBUG("The battery zone \"%s\" reports \"%s\", which is not a number", entry->d_name, buffer.chars);
            return FF_BATTERY_TEMP_UNSET;
        }

        FF_DEBUG("The battery zone is \"%s\", reporting %.2f degrees", entry->d_name, value / 1000.);
        return value / 1000.;
    }

    FF_DEBUG("No zone in \"%s\" is named \"%s\", so no temperature is reported",
        FF_BATTERY_ANDROID_THERMAL_DIR, FF_BATTERY_ANDROID_THERMAL_BATTERY_TYPE);
    return FF_BATTERY_TEMP_UNSET;
}

// Temperature is opt-in on every platform, and on Android that is not only about the display: the
// walk above touches a directory with a hundred-odd entries on the device this was written on, and
// none of that is paid for unless `temp` asks for it.
static void fillTemperature(FFBatteryOptions* options, FFlist* results) {
    if (!options->temp) {
        FF_DEBUG("Temperature is not enabled, so the thermal zones are not walked");
        return;
    }
    if (results->length == 0) {
        return;
    }

    const double temperature = detectTemperature();
    if (temperature != FF_BATTERY_TEMP_UNSET) {
        FF_LIST_GET(FFBatteryResult, *results, 0)->temperature = temperature;
    }
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results) {
    // Resolved before the binder is opened, so that a jar which does not declare the method says so
    // rather than the module failing later with a message about the transport. Both codes are asked
    // for in one call, which is cheaper than two: the reader walks the jar's dex entries in order and
    // stops once every request has been answered, so a second request only costs the walk to the
    // entry that defines the second class. That walk is not free here, because the two classes are
    // not in the same entry -- `IBatteryPropertiesRegistrar$Stub` is in classes3.dex and
    // `IBatteryStats$Stub` in classes5.dex -- and on the device this was measured on it added 1.3 ms
    // to a module that had been 3.3 ms, while the service lookup and the transaction behind it stayed
    // inside the noise. The reader reports a code it cannot find by leaving the sentinel in the
    // result and says nothing more -- what a missing code means for this module is decided here, not
    // there.
    //
    // The two are not equally important. `getProperty` is what the module is made of, so a build
    // without it has nothing to report; `computeBatteryTimeRemaining` only adds the time estimate,
    // and a build without it still has the capacity and the status, so its sentinel is carried on to
    // where the estimate is asked for instead of failing the route here.
    int32_t transaction = FF_DEX_STATIC_INT_UNRESOLVED;
    int32_t timeRemainingTransaction = FF_DEX_STATIC_INT_UNRESOLVED;
    const FFDexStaticIntRequest requests[] = {
        { FF_BATTERY_ANDROID_STUB, FF_BATTERY_ANDROID_GET_PROPERTY, &transaction },
        { FF_BATTERY_ANDROID_STATS_STUB, FF_BATTERY_ANDROID_COMPUTE_BATTERY_TIME_REMAINING, &timeRemainingTransaction },
    };
    const char* error = ffDexStaticInts(FF_BATTERY_ANDROID_JAR, requests, ARRAY_SIZE(requests));
    if (error != nullptr) {
        FF_DEBUG("Reading the transaction codes from \"%s\" failed: %s", FF_BATTERY_ANDROID_JAR, error);
        return error;
    }
    if (transaction == FF_DEX_STATIC_INT_UNRESOLVED) {
        return "The battery service does not declare getProperty";
    }

    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    error = ffBinderOpen(&binder);
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
    FF_DEBUG("The \"%s\" service is handle %u, getProperty is transaction %d", FF_BATTERY_ANDROID_SERVICE, service.handle, transaction);

    uint64_t capacity = 0;
    error = getProperty(&binder, service.handle, (uint32_t) transaction, FF_BATTERY_ANDROID_PROPERTY_CAPACITY, &capacity);
    if (error != nullptr) {
        FF_DEBUG("The capacity is what failed, so no battery is reported at all");
        return error;
    }

    // The status is asked of the property first, because that is where the second transaction on the
    // service is saved. The service is still asked when the property has nothing to say, and a failure
    // there is still fatal: the capacity and the status are what this route is made of, and a battery
    // whose status is unknown is not a battery this route can describe.
    uint64_t status = 0;
    if (detectStatusFromProperty(&status)) {
        FF_DEBUG("The status came from the \"%s\" property and is %" PRIu64 ", so the service is not asked "
                 "for it",
            FF_BATTERY_ANDROID_PROP_BATTERY_STATUS, status);
    } else {
        error = getProperty(&binder, service.handle, (uint32_t) transaction, FF_BATTERY_ANDROID_PROPERTY_STATUS, &status);
        if (error != nullptr) {
            FF_DEBUG("The status is what failed, so no battery is reported at all");
            return error;
        }
    }

    FFBatteryResult* battery = FF_LIST_ADD(FFBatteryResult, *results);
    initResult(battery);
    battery->capacity = (double) capacity;
    if (status == FF_BATTERY_ANDROID_STATUS_CHARGING) {
        battery->status |= FF_BATTERY_STATUS_CHARGING;
    } else if (status == FF_BATTERY_ANDROID_STATUS_DISCHARGING) {
        battery->status |= FF_BATTERY_STATUS_DISCHARGING;
    }
    // The charger type is the one reading this route has that the registrar does not, and it is a
    // property as well: the three bits used to come from `dumpsys battery`, and so from the shell UID
    // and root alone.
    fillPlugged(battery);
    // The temperature comes from outside this route entirely, so it is asked for here rather than
    // inside it: nothing about the binder session above has any bearing on it, and a device without a
    // zone by that name simply leaves the field at the sentinel.
    fillTemperature(options, results);
    // The time estimate is the one part of this route that is allowed to be missing: a jar without
    // the code, a service that is not running and a service that answers -1 all leave the field at
    // the -1 the module starts it with, which is how `FFBatteryResult` spells "unknown". It is asked
    // for last for that reason -- a failure here cannot cost the capacity and the status that have
    // already been read.
    [[gnu::cleanup(ffBinderServiceHandleRelease)]] FFBinderServiceHandle stats = { .binder = &binder };
    error = ffBinderLookupService(&binder, FF_BATTERY_ANDROID_STATS_SERVICE, FF_BINDER_SM_GET_SERVICE, &stats.handle);
    if (error != nullptr) {
        FF_DEBUG("Looking up the \"%s\" service failed, so no time remaining is reported: %s",
            FF_BATTERY_ANDROID_STATS_SERVICE, error);
    } else {
        fillTimeRemaining(&binder, stats.handle, timeRemainingTransaction, results);
    }

    // `BatteryStatus` has five values and only two of them say something the powered bits do not, so
    // the trace spells the mapping out: anything that is not 2 or 3 leaves the charging / discharging
    // bits clear, which is what UNKNOWN (1), NOT_CHARGING (4) and FULL (5) all come to -- that a
    // charger is attached is stated by the bits the plug_type property fills in, not by this one. The
    // temperature is not repeated here: it was traced where it was read, and it is the field of the
    // three that can be missing without anything being wrong.
    FF_DEBUG("Reporting one battery: capacity %.0f%%, status %" PRIu64 " -> bits 0x%x "
             "(2 charging, 3 discharging; 1/4/5 leave them clear), %d s remaining (-1 when there is no "
             "estimate). The technology stays unset: it rides the sticky broadcast, which this route "
             "does not read",
        battery->capacity, status, (unsigned) battery->status, battery->timeRemaining);
    return nullptr;
}
