#include "sound.h"

#include "detection/displayserver/displayserver.h"
#include "common/android/binder.h"
#include "common/android/dex.h"
#include "common/debug.h"
#include "common/strutil.h"

#include <string.h>

// Android has no desktop sound stack. PulseAudio is not part of the platform, but a Termux user can
// run one, and that is what a session sounds through whenever the display server is a real desktop
// rather than SurfaceFlinger -- so this file answers for SurfaceFlinger alone and hands the rest to
// sound_linux.c, which speaks PulseAudio. ALSA and OSS are not there either: /dev/snd, /proc/asound
// and /sys/class/sound are all EACCES from an app's SELinux domain, `dumpsys audio` needs
// android.permission.DUMP, and `settings get system volume_music` needs INTERACT_ACROSS_USERS.
// `cmd audio` is worse than useless: `cmd` hands the arguments to the service over binder and the
// command body runs inside system_server, so from an app it hangs or answers ENOSYS.
//
// What is left is the service those all sit on: `audio`, i.e. android.media.IAudioService, which
// lives in system_server. This file speaks to it over /dev/binder, the same way the battery, media
// and wifi modules do. Two things about it are worth stating up front, because they are what makes
// this module work at all:
//
//   * Nothing here needs a permission. The methods used below are the ones `AudioManager` itself
//     calls on behalf of an ordinary app, and the service checks no permission for them. The one
//     that does the real work is deliberately named for it -- see getDevicesForAttributesUnprotected
//     below. The protected twin, `getDevicesForAttributes`, is refused with
//     "Missing MODIFY_AUDIO_ROUTING or QUERY_AUDIO_STATE permissions", and
//     `getLastAudibleStreamVolume` with "requires android.permission.QUERY_AUDIO_STATE"; neither is
//     used. So unlike the wifi module, this one works on a device where nothing has been granted.
//
//   * Every transaction code is resolved from the device's own jar at run time, never written down.
//     `IAudioService$Stub` declares 283 methods in AOSP and 324 on the test device -- a vendor fork
//     inserts methods freely, and an insertion ahead of the one being called shifts it. Reading the
//     constants out of the dex is the only thing that survives that; see common/android/dex.h. The
//     class is in the *second* dex of framework.jar, which is why ffDexStaticInt walks the entries.
//
// The reply is not uniform, because the two kinds of service are not: IAudioService is Java, so a
// failure arrives as a parcel exception code (-1 for a SecurityException, -4 for a null argument),
// while the native services answer with a TF_STATUS_CODE carrying a raw status_t. Only the former
// can happen here, but the check is cheap and the two are easy to confuse.

#define FF_SOUND_ANDROID_SERVICE "audio"
#define FF_SOUND_ANDROID_DESCRIPTOR "android.media.IAudioService"

// IAudioService has always been part of framework.jar -- it is not one of the frameworks that moved
// into an APEX -- so there is one path and no fallback. The stub is in classes2.dex there.
#define FF_SOUND_ANDROID_JAR "/system/framework/framework.jar"
#define FF_SOUND_ANDROID_STUB "Landroid/media/IAudioService$Stub;"

// The methods this module calls, by the name of the constant the dex carries for each. Spelling the
// names out rather than the numbers is the whole point: the lookup turns them into whatever this
// build uses, and a build that does not declare one says so instead of reaching another method.
#define FF_SOUND_ANDROID_GET_STREAM_VOLUME "TRANSACTION_getStreamVolume"
#define FF_SOUND_ANDROID_GET_STREAM_MIN_VOLUME "TRANSACTION_getStreamMinVolume"
#define FF_SOUND_ANDROID_GET_STREAM_MAX_VOLUME "TRANSACTION_getStreamMaxVolume"
#define FF_SOUND_ANDROID_IS_STREAM_MUTE "TRANSACTION_isStreamMute"
#define FF_SOUND_ANDROID_IS_MUSIC_ACTIVE "TRANSACTION_isMusicActive"
#define FF_SOUND_ANDROID_GET_DEVICES_FOR_ATTRIBUTES "TRANSACTION_getDevicesForAttributesUnprotected"
#define FF_SOUND_ANDROID_GET_BT_ACTIVE_DEVICE_NAME "TRANSACTION_getBtActiveDeviceName"

// `AudioManager.STREAM_MUSIC`. The one stream a phone's volume rocker moves, and the one the module
// reports; the other eleven are internal (SYSTEM_ENFORCED, DTMF, ...) or belong to a call.
typedef enum FFSoundAndroidStream : int32_t {
    FF_SOUND_ANDROID_STREAM_MUSIC = 3,
} FFSoundAndroidStream;

// `AudioAttributes.USAGE_MEDIA`. Asking the policy what it routes for media is what makes the answer
// "where the phone's sound is going"; other usages answer differently on purpose (a call goes to the
// earpiece, an emergency call to the telephony port).
typedef enum FFSoundAndroidUsage : int32_t {
    FF_SOUND_ANDROID_USAGE_MEDIA = 1,
} FFSoundAndroidUsage;

// `AudioAttributes.SOURCE_*`, and the one value that matters here. A request whose source is
// anything but AUDIO_SOURCE_INVALID takes the *capture* branch of
// AudioPolicyManager::getDevicesForAttributes and answers with microphones, whatever the usage says:
//
//     if (attr.source != AUDIO_SOURCE_INVALID) return getInputDevicesForAttributes(attr, devices);
//
// AUDIO_SOURCE_DEFAULT is 0, so leaving the field at zero -- which is what a zeroed parcel does --
// silently reports the built-in microphone as the output device. It has to be written out as -1.
#define FF_SOUND_ANDROID_SOURCE_INVALID (-1)

// `AudioAttributes.ATTR_PARCEL_IS_NULL_BUNDLE`. The tags and the bundle are the only variable-length
// parts of the request; an empty tag list and no bundle is the smallest well-formed one.
#define FF_SOUND_ANDROID_ATTR_NULL_BUNDLE (-1977)

// The interface token is 12 bytes of header plus a padded string16, and the largest request here adds
// one AudioAttributes to it. The replies are one int, or one list of AudioDeviceAttributes. The
// device reply is sized well past the 48 bytes a single device takes, because a reply that does not
// fit is reported as a buffer error and would hide the exception message the service sent instead.
#define FF_SOUND_ANDROID_SCALAR_PARCEL_SIZE 96
#define FF_SOUND_ANDROID_SCALAR_REPLY_SIZE 32
#define FF_SOUND_ANDROID_DEVICE_PARCEL_SIZE 128
#define FF_SOUND_ANDROID_DEVICE_REPLY_SIZE 1024

// The reply to the Bluetooth device name request is one string16, and a device name is chosen by
// whoever owns the headset: 248 bytes is the longest a Bluetooth name can be, so 512 bytes of reply
// hold the count, the code units, the terminator and the padding with room to spare. A reply that
// does not fit is reported as a buffer error, so sizing it generously is what keeps a long name
// from looking like a failure.
#define FF_SOUND_ANDROID_NAME_REPLY_SIZE 512

// `AudioDeviceInfo.TYPE_*`, as the test device's own dex declares them (read out of
// `Landroid/media/AudioDeviceInfo;`). The value is what `AudioDeviceAttributes.type` carries in the
// reply; the names are the AOSP constant names without the prefix, which is what goes into
// `identifier`, and a spelled out name, which is what the module prints. An unrecognised value is
// reported rather than guessed at: a vendor is free to add types, and a wrong name is worse than
// "Unknown Device".
typedef enum FFSoundAndroidDeviceType : int32_t {
    FF_SOUND_ANDROID_TYPE_UNKNOWN = 0,
    FF_SOUND_ANDROID_TYPE_BUILTIN_EARPIECE = 1,
    FF_SOUND_ANDROID_TYPE_BUILTIN_SPEAKER = 2,
    FF_SOUND_ANDROID_TYPE_WIRED_HEADSET = 3,
    FF_SOUND_ANDROID_TYPE_WIRED_HEADPHONES = 4,
    FF_SOUND_ANDROID_TYPE_LINE_ANALOG = 5,
    FF_SOUND_ANDROID_TYPE_LINE_DIGITAL = 6,
    FF_SOUND_ANDROID_TYPE_BLUETOOTH_SCO = 7,
    FF_SOUND_ANDROID_TYPE_BLUETOOTH_A2DP = 8,
    FF_SOUND_ANDROID_TYPE_HDMI = 9,
    FF_SOUND_ANDROID_TYPE_HDMI_ARC = 10,
    FF_SOUND_ANDROID_TYPE_USB_DEVICE = 11,
    FF_SOUND_ANDROID_TYPE_USB_ACCESSORY = 12,
    FF_SOUND_ANDROID_TYPE_DOCK = 13,
    FF_SOUND_ANDROID_TYPE_FM = 14,
    FF_SOUND_ANDROID_TYPE_BUILTIN_MIC = 15,
    FF_SOUND_ANDROID_TYPE_FM_TUNER = 16,
    FF_SOUND_ANDROID_TYPE_TV_TUNER = 17,
    FF_SOUND_ANDROID_TYPE_TELEPHONY = 18,
    FF_SOUND_ANDROID_TYPE_AUX_LINE = 19,
    FF_SOUND_ANDROID_TYPE_IP = 20,
    FF_SOUND_ANDROID_TYPE_BUS = 21,
    FF_SOUND_ANDROID_TYPE_USB_HEADSET = 22,
    FF_SOUND_ANDROID_TYPE_HEARING_AID = 23,
    FF_SOUND_ANDROID_TYPE_BUILTIN_SPEAKER_SAFE = 24,
    FF_SOUND_ANDROID_TYPE_REMOTE_SUBMIX = 25,
    FF_SOUND_ANDROID_TYPE_BLE_HEADSET = 26,
    FF_SOUND_ANDROID_TYPE_BLE_SPEAKER = 27,
    FF_SOUND_ANDROID_TYPE_ECHO_REFERENCE = 28,
    FF_SOUND_ANDROID_TYPE_HDMI_EARC = 29,
    FF_SOUND_ANDROID_TYPE_BLE_BROADCAST = 30,
    FF_SOUND_ANDROID_TYPE_DOCK_ANALOG = 31,
    FF_SOUND_ANDROID_TYPE_MULTICHANNEL_GROUP = 32,
} FFSoundAndroidDeviceType;

typedef struct FFSoundAndroidDeviceName {
    FFSoundAndroidDeviceType type;
    const char* identifier;
    const char* name;
} FFSoundAndroidDeviceName;

static const FFSoundAndroidDeviceName FF_SOUND_ANDROID_DEVICE_NAMES[] = {
    { FF_SOUND_ANDROID_TYPE_UNKNOWN, "UNKNOWN", "Unknown Device" },
    { FF_SOUND_ANDROID_TYPE_BUILTIN_EARPIECE, "BUILTIN_EARPIECE", "Built-in Earpiece" },
    { FF_SOUND_ANDROID_TYPE_BUILTIN_SPEAKER, "BUILTIN_SPEAKER", "Built-in Speaker" },
    { FF_SOUND_ANDROID_TYPE_WIRED_HEADSET, "WIRED_HEADSET", "Wired Headset" },
    { FF_SOUND_ANDROID_TYPE_WIRED_HEADPHONES, "WIRED_HEADPHONES", "Wired Headphones" },
    { FF_SOUND_ANDROID_TYPE_LINE_ANALOG, "LINE_ANALOG", "Line (Analog)" },
    { FF_SOUND_ANDROID_TYPE_LINE_DIGITAL, "LINE_DIGITAL", "Line (Digital)" },
    { FF_SOUND_ANDROID_TYPE_BLUETOOTH_SCO, "BLUETOOTH_SCO", "Bluetooth SCO" },
    { FF_SOUND_ANDROID_TYPE_BLUETOOTH_A2DP, "BLUETOOTH_A2DP", "Bluetooth A2DP" },
    { FF_SOUND_ANDROID_TYPE_HDMI, "HDMI", "HDMI" },
    { FF_SOUND_ANDROID_TYPE_HDMI_ARC, "HDMI_ARC", "HDMI ARC" },
    { FF_SOUND_ANDROID_TYPE_USB_DEVICE, "USB_DEVICE", "USB Device" },
    { FF_SOUND_ANDROID_TYPE_USB_ACCESSORY, "USB_ACCESSORY", "USB Accessory" },
    { FF_SOUND_ANDROID_TYPE_DOCK, "DOCK", "Dock" },
    { FF_SOUND_ANDROID_TYPE_FM, "FM", "FM" },
    { FF_SOUND_ANDROID_TYPE_BUILTIN_MIC, "BUILTIN_MIC", "Built-in Microphone" },
    { FF_SOUND_ANDROID_TYPE_FM_TUNER, "FM_TUNER", "FM Tuner" },
    { FF_SOUND_ANDROID_TYPE_TV_TUNER, "TV_TUNER", "TV Tuner" },
    { FF_SOUND_ANDROID_TYPE_TELEPHONY, "TELEPHONY", "Telephony" },
    { FF_SOUND_ANDROID_TYPE_AUX_LINE, "AUX_LINE", "Aux Line" },
    { FF_SOUND_ANDROID_TYPE_IP, "IP", "IP" },
    { FF_SOUND_ANDROID_TYPE_BUS, "BUS", "Bus" },
    { FF_SOUND_ANDROID_TYPE_USB_HEADSET, "USB_HEADSET", "USB Headset" },
    { FF_SOUND_ANDROID_TYPE_HEARING_AID, "HEARING_AID", "Hearing Aid" },
    { FF_SOUND_ANDROID_TYPE_BUILTIN_SPEAKER_SAFE, "BUILTIN_SPEAKER_SAFE", "Built-in Speaker (Safe)" },
    { FF_SOUND_ANDROID_TYPE_REMOTE_SUBMIX, "REMOTE_SUBMIX", "Remote Submix" },
    { FF_SOUND_ANDROID_TYPE_BLE_HEADSET, "BLE_HEADSET", "Bluetooth LE Headset" },
    { FF_SOUND_ANDROID_TYPE_BLE_SPEAKER, "BLE_SPEAKER", "Bluetooth LE Speaker" },
    { FF_SOUND_ANDROID_TYPE_ECHO_REFERENCE, "ECHO_REFERENCE", "Echo Reference" },
    { FF_SOUND_ANDROID_TYPE_HDMI_EARC, "HDMI_EARC", "HDMI eARC" },
    { FF_SOUND_ANDROID_TYPE_BLE_BROADCAST, "BLE_BROADCAST", "Bluetooth LE Broadcast" },
    { FF_SOUND_ANDROID_TYPE_DOCK_ANALOG, "DOCK_ANALOG", "Dock (Analog)" },
    { FF_SOUND_ANDROID_TYPE_MULTICHANNEL_GROUP, "MULTICHANNEL_GROUP", "Multichannel Group" },
};

static const FFSoundAndroidDeviceName* soundAndroidDeviceName(int32_t type) {
    for (size_t i = 0; i < ARRAY_SIZE(FF_SOUND_ANDROID_DEVICE_NAMES); ++i) {
        if ((int32_t) FF_SOUND_ANDROID_DEVICE_NAMES[i].type == type) {
            return &FF_SOUND_ANDROID_DEVICE_NAMES[i];
        }
    }
    return nullptr;
}

// The types that stand for a Bluetooth device. They are the ones whose own name is worth asking the
// service for: everything else is described well enough by its type, and the request is one more
// round trip to system_server. A hearing aid is on the list because it is a Bluetooth LE link with
// a name of its own, the same as a headset.
static bool soundAndroidIsBluetooth(int32_t type) {
    switch ((FFSoundAndroidDeviceType) type) {
        case FF_SOUND_ANDROID_TYPE_BLUETOOTH_SCO:
        case FF_SOUND_ANDROID_TYPE_BLUETOOTH_A2DP:
        case FF_SOUND_ANDROID_TYPE_HEARING_AID:
        case FF_SOUND_ANDROID_TYPE_BLE_HEADSET:
        case FF_SOUND_ANDROID_TYPE_BLE_SPEAKER:
        case FF_SOUND_ANDROID_TYPE_BLE_BROADCAST:
            return true;
        default:
            return false;
    }
}

// Every transaction code this module sends, resolved from the device's own jar in one walk. They are
// asked for together because the walk is dominated by the dex's type table and all seven sit in the
// same class of the same entry -- see common/android/dex.h.
typedef struct FFSoundAndroidCodes {
    int32_t getStreamVolume;
    int32_t getStreamMinVolume;
    int32_t getStreamMaxVolume;
    int32_t isStreamMute;
    int32_t getDevicesForAttributes;
    int32_t isMusicActive;
    int32_t getBtActiveDeviceName;
} FFSoundAndroidCodes;

// The five codes the module cannot answer without, against the two that only refine the answer. A
// build that does not declare one of these five has nothing to report, which is what the module did
// when each code was resolved on its own.
static bool soundAndroidCodesIncomplete(const FFSoundAndroidCodes* codes) {
    return codes->getStreamVolume == FF_DEX_STATIC_INT_UNRESOLVED
        || codes->getStreamMinVolume == FF_DEX_STATIC_INT_UNRESOLVED
        || codes->getStreamMaxVolume == FF_DEX_STATIC_INT_UNRESOLVED
        || codes->isStreamMute == FF_DEX_STATIC_INT_UNRESOLVED
        || codes->getDevicesForAttributes == FF_DEX_STATIC_INT_UNRESOLVED;
}

// Resolves all seven in one call, and then makes the judgement the reader leaves to its caller. The
// two that only refine the answer are allowed to come back as the sentinel, because a build may
// declare one of them and not the other -- Mi 10 has `getDevicesForAttributesUnprotected` and no
// `getBtActiveDeviceName` -- and a code the module can live without must not take it down with it.
static const char* soundAndroidResolveCodes(FFSoundAndroidCodes* codes) {
    const FFDexStaticIntRequest requests[] = {
        { FF_SOUND_ANDROID_STUB, FF_SOUND_ANDROID_GET_STREAM_VOLUME, &codes->getStreamVolume },
        { FF_SOUND_ANDROID_STUB, FF_SOUND_ANDROID_GET_STREAM_MIN_VOLUME, &codes->getStreamMinVolume },
        { FF_SOUND_ANDROID_STUB, FF_SOUND_ANDROID_GET_STREAM_MAX_VOLUME, &codes->getStreamMaxVolume },
        { FF_SOUND_ANDROID_STUB, FF_SOUND_ANDROID_IS_STREAM_MUTE, &codes->isStreamMute },
        { FF_SOUND_ANDROID_STUB, FF_SOUND_ANDROID_GET_DEVICES_FOR_ATTRIBUTES, &codes->getDevicesForAttributes },
        { FF_SOUND_ANDROID_STUB, FF_SOUND_ANDROID_IS_MUSIC_ACTIVE, &codes->isMusicActive },
        { FF_SOUND_ANDROID_STUB, FF_SOUND_ANDROID_GET_BT_ACTIVE_DEVICE_NAME, &codes->getBtActiveDeviceName },
    };

    const char* error = ffDexStaticInts(FF_SOUND_ANDROID_JAR, requests, ARRAY_SIZE(requests));
    if (error != nullptr) {
        FF_DEBUG("Reading the transaction codes from \"%s\" failed: %s", FF_SOUND_ANDROID_JAR, error);
        return error;
    }
    if (soundAndroidCodesIncomplete(codes)) {
        return "The audio service does not declare every method this module calls";
    }
    return nullptr;
}

// Calls a method that takes no argument or one int, and answers with one int. `transaction` comes out
// of the jar, and the sentinel the reader leaves for a code it could not find is caught here, so that
// a build which does not declare the method is a message rather than a transaction that means
// something else. `transactionField` is carried only to name the method in the debug output, which a
// release build compiles out -- hence the attribute.
static const char* soundAndroidCallInt(FFBinder* binder, uint32_t handle, [[maybe_unused]] const char* transactionField, int32_t transaction, bool hasArgument, int32_t argument, int32_t* result) {
    if (transaction == FF_DEX_STATIC_INT_UNRESOLVED) {
        return "The audio service does not declare that method";
    }

    uint8_t parcelBuffer[FF_SOUND_ANDROID_SCALAR_PARCEL_SIZE];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_SOUND_ANDROID_DESCRIPTOR);
    if (hasArgument) {
        ffBinderParcelPutI32(&parcel, argument);
    }

    uint8_t replyBuffer[FF_SOUND_ANDROID_SCALAR_REPLY_SIZE];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    const char* error = ffBinderTransact(binder, handle, (uint32_t) transaction, 0, &parcel, &reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("\"%s\" resolved to transaction %d, which this build does not answer", transactionField, transaction);
        return "The audio service does not answer that request";
    }
    const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
    if (exception != 0) {
        FF_DEBUG("\"%s\" is transaction %d and raised exception %d", transactionField, transaction, exception);
        return "The audio service raised an exception";
    }
    *result = ffBinderReadI32(reply.data, reply.size, 4);
    FF_DEBUG("\"%s\" is transaction %d and answered %d", transactionField, transaction, *result);
    return nullptr;
}

// Walks past a string16 and reports how long it was. The bytes themselves are not decoded: the only
// strings in the reply are the device address and name, both of which are empty for every built-in
// device and a partly anonymised MAC for a Bluetooth one -- `AudioService` blanks the address of
// anything it considers identifying before answering an unprivileged caller, so neither is an
// identifier worth carrying.
static bool soundAndroidSkipString16(const uint8_t* data, size_t size, size_t* offset) {
    const int32_t length = ffBinderReadI32(data, size, *offset);
    *offset += sizeof(int32_t);
    if (length < 0) {
        return true;
    }
    // The code units plus the NUL terminator, padded to four bytes.
    const size_t payload = ((size_t) length * 2 + 2 + 3) & ~(size_t) 3;
    if (*offset + payload > size) {
        return false;
    }
    *offset += payload;
    return true;
}

// Reads a string16 out of a reply and appends it to `result`.
//
// The code units are UTF-16LE, and a device name is the first string here that is not guaranteed to
// be ASCII -- a headset can be named in any script, and `vivo TWS A4` is only the one that happened
// to be paired -- so they are converted rather than copied. A count of -1 is how the same field
// spells null, which is what the service answers when there is no name to give; that is not a
// failure, and the caller falls back to the type name.
static bool soundAndroidReadString16(const uint8_t* data, size_t size, size_t offset, FFstrbuf* result) {
    if (offset + sizeof(int32_t) > size) {
        return false;
    }
    const int32_t length = ffBinderReadI32(data, size, offset);
    offset += sizeof(int32_t);
    if (length < 0) {
        return true;
    }
    // The code units plus the NUL terminator. The padding behind them is not part of the string, so
    // there is nothing to step over here.
    if ((size_t) length * 2 + 2 > size - offset) {
        return false;
    }

    for (int32_t i = 0; i < length; ++i) {
        uint32_t codePoint = (uint32_t) data[offset + (size_t) i * 2] | ((uint32_t) data[offset + (size_t) i * 2 + 1] << 8);
        if (codePoint >= 0xD800 && codePoint <= 0xDBFF && i + 1 < length) {
            // Anything outside the basic plane arrives as a surrogate pair, and the low half is what
            // turns it back into one code point. A high surrogate without one is malformed and is
            // passed through as it stands rather than guessed at.
            const uint32_t low = (uint32_t) data[offset + (size_t) (i + 1) * 2] | ((uint32_t) data[offset + (size_t) (i + 1) * 2 + 1] << 8);
            if (low >= 0xDC00 && low <= 0xDFFF) {
                codePoint = 0x10000u + ((codePoint - 0xD800u) << 10) + (low - 0xDC00u);
                ++i;
            }
        }
        ffStrbufAppendUtf32CodePoint(result, codePoint);
    }
    return true;
}

// Calls `getBtActiveDeviceName()`, which takes no argument and answers a string16. It is the only
// call here that carries a human readable device name.
//
// The policy reply has a `name` field of its own, but on the test device `AudioService` leaves it
// empty for every device -- a Bluetooth one included, whose address it blanks to
// `XX:XX:XX:XX:D8:61` while the name goes with it -- so the type is all that reply can describe a
// sink by. This call is what turns "Bluetooth A2DP" into the name of the headset.
//
// What it must not be used for is deciding *which* device is the output. It answers about the
// Bluetooth route as a whole and reports it even when the policy has selected the speaker, so it is
// only ever consulted after the device list has already picked a Bluetooth sink, and only its name
// is taken from it.
static const char* soundAndroidCallBtName(FFBinder* binder, uint32_t handle, int32_t transaction, FFstrbuf* result) {
    if (transaction == FF_DEX_STATIC_INT_UNRESOLVED) {
        return "The audio service does not declare getBtActiveDeviceName";
    }

    uint8_t parcelBuffer[FF_SOUND_ANDROID_SCALAR_PARCEL_SIZE];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_SOUND_ANDROID_DESCRIPTOR);

    uint8_t replyBuffer[FF_SOUND_ANDROID_NAME_REPLY_SIZE];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    const char* error = ffBinderTransact(binder, handle, (uint32_t) transaction, 0, &parcel, &reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("\"%s\" resolved to transaction %d, which this build does not answer", FF_SOUND_ANDROID_GET_BT_ACTIVE_DEVICE_NAME, transaction);
        return "The audio service does not answer that request";
    }
    const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
    if (exception != 0) {
        FF_DEBUG("\"%s\" is transaction %d and raised exception %d", FF_SOUND_ANDROID_GET_BT_ACTIVE_DEVICE_NAME, transaction, exception);
        return "The audio service raised an exception";
    }
    if (!soundAndroidReadString16(reply.data, reply.size, sizeof(int32_t), result)) {
        return "The audio service answered a truncated device name";
    }
    FF_DEBUG("\"%s\" is transaction %d and answered \"%s\"", FF_SOUND_ANDROID_GET_BT_ACTIVE_DEVICE_NAME, transaction,
        result->chars != nullptr ? result->chars : "");
    return nullptr;
}

// Reads one `AudioDeviceAttributes` out of the reply and advances `offset` past it.
//
// The layout is the one `AudioDeviceAttributes.writeToParcel` writes, and it was checked byte by
// byte against the device's replies:
//
//     int32 role, int32 type, string16 address, string16 name, int32 nativeType,
//     List<AudioProfile> profiles, List<AudioProfile> descriptors
//
// `role` is 1 for a source (an input) and 2 for a sink (an output). `type` is the
// AudioDeviceInfo.TYPE_* above; `nativeType` repeats it as the older AUDIO_DEVICE_* bit value and is
// not used. Neither profile list has carried an entry on any device seen so far, so they are walked
// only far enough to confirm that. A device that does carry profiles is still reported -- everything
// this module reads comes before them -- but nothing after it in the list can be reached, because
// walking a profile means decoding one, and the caller stops there rather than reading the next
// entry out of the middle of one.
static bool soundAndroidReadDevice(const uint8_t* data, size_t size, size_t* offset, int32_t* type, int32_t* role, bool* ended) {
    *ended = false;
    if (*offset + 3 * sizeof(int32_t) > size) {
        return false;
    }
    *role = ffBinderReadI32(data, size, *offset);
    *offset += sizeof(int32_t);
    *type = ffBinderReadI32(data, size, *offset);
    *offset += sizeof(int32_t);
    if (!soundAndroidSkipString16(data, size, offset) || !soundAndroidSkipString16(data, size, offset)) {
        return false;
    }
    *offset += sizeof(int32_t); // nativeType, which follows the two strings
    for (int list = 0; list < 2; ++list) {
        const int32_t count = ffBinderReadI32(data, size, *offset);
        *offset += sizeof(int32_t);
        if (count > 0) {
            // A null list is a -1 and occupies nothing beyond the count that was just read, so only
            // a list with entries in it ends the walk.
            *ended = true;
            return true;
        }
    }
    return true;
}

// Turns the three volume readings into the percentage the module prints. The service answers with
// the index it keeps for the stream, which is not a percentage and is not even on a fixed scale:
// `getStreamVolume` divides by ten before returning, so the numbers here are the ones an app sees,
// and the maximum differs per stream -- 150 for music on the test device, 15 for the system stream,
// 10 for a call. Music's minimum is 0, but that is a property of the stream rather than of the API:
// a call's minimum is 1. So the percentage comes out of the range the service reports, and a stream
// whose range is empty or inverted is reported as unknown rather than as 100%.
static uint8_t soundAndroidPercent(int32_t volume, int32_t minimum, int32_t maximum, int32_t mute) {
    if (mute != 0) {
        // A muted stream is reported as 0 rather than as its index: the service keeps the index and
        // the mute flag separately, and it is the mute flag that decides what comes out of the
        // speaker. This is also what the PulseAudio backend does with a muted sink.
        return 0;
    }
    if (maximum <= minimum) {
        return FF_SOUND_VOLUME_UNKNOWN;
    }
    if (volume <= minimum) {
        return 0;
    }
    if (volume >= maximum) {
        return 100;
    }
    return (uint8_t) (((volume - minimum) * 100 + (maximum - minimum) / 2) / (maximum - minimum));
}

static const char* detectNative(FFSoundOptions* options, FFlist* devices) {
    // Resolved before anything else so that a build without the methods says so, rather than opening
    // binder and failing later with a message about the transport. All seven codes are asked for in
    // one call and judged here: the five the module cannot answer without decide whether it runs at
    // all, and the two it only uses when the build declares them are left as sentinels for their
    // call sites to cope with.
    FFSoundAndroidCodes codes = {};
    const char* error = soundAndroidResolveCodes(&codes);
    if (error != nullptr) {
        return error;
    }

    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    error = ffBinderOpen(&binder);
    if (error != nullptr) {
        return error;
    }

    // Released on every path out, including the early returns below.
    [[gnu::cleanup(ffBinderServiceHandleRelease)]] FFBinderServiceHandle service = { .binder = &binder };
    error = ffBinderLookupService(&binder, FF_SOUND_ANDROID_SERVICE, FF_BINDER_SM_GET_SERVICE, &service.handle);
    if (error != nullptr) {
        return error;
    }
    FF_DEBUG("The \"%s\" service is handle %u, getDevicesForAttributesUnprotected is transaction %d",
        FF_SOUND_ANDROID_SERVICE, service.handle, codes.getDevicesForAttributes);

    uint8_t parcelBuffer[FF_SOUND_ANDROID_DEVICE_PARCEL_SIZE];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_SOUND_ANDROID_DESCRIPTOR);
    // The non-null marker `Parcel.writeTypedObject` puts in front of a Parcelable. The service reads
    // the argument with `readTypedObject`, which reads that int first and hands back a null object
    // when it is not 1 -- so leaving it out does not shift the fields, it loses the argument, and
    // the service answers with an exception rather than a device list.
    ffBinderParcelPutI32(&parcel, 1);
    // `in AudioAttributes`, whose writeToParcel is: usage, contentType, source, flags, the parcel
    // flags, the tag array and the bundle marker. A null bundle is the only form built here.
    ffBinderParcelPutI32(&parcel, (int32_t) FF_SOUND_ANDROID_USAGE_MEDIA);
    ffBinderParcelPutI32(&parcel, 0); // contentType, unused by the policy when a usage is set
    ffBinderParcelPutI32(&parcel, FF_SOUND_ANDROID_SOURCE_INVALID);
    ffBinderParcelPutI32(&parcel, 0); // flags
    ffBinderParcelPutI32(&parcel, 0); // parcel flags: the tags are not flattened
    ffBinderParcelPutI32(&parcel, 0); // an empty tag array
    ffBinderParcelPutI32(&parcel, FF_SOUND_ANDROID_ATTR_NULL_BUNDLE);

    uint8_t replyBuffer[FF_SOUND_ANDROID_DEVICE_REPLY_SIZE];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    error = ffBinderTransact(&binder, service.handle, (uint32_t) codes.getDevicesForAttributes, 0, &parcel, &reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("The reply carries status %d instead of a parcel", (int) ffBinderReadI32(reply.data, reply.size, 0));
        return "The audio service rejected the request";
    }

    const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
    if (exception != 0) {
        FF_DEBUG("getDevicesForAttributesUnprotected raised exception %d", exception);
        return "The audio service raised an exception";
    }

    // The service answers with the devices the policy has selected for that usage, so there can be
    // more than one -- an alarm or a ringtone plays on the speaker and on a connected headset at the
    // same time -- and the first is the one the stream's volume belongs to.
    const int32_t count = ffBinderReadI32(reply.data, reply.size, 4);
    size_t offset = 2 * sizeof(int32_t);
    const FFSoundAndroidDeviceName* name = nullptr;
    int32_t deviceType = 0;
    for (int32_t index = 0; index < count; ++index) {
        const int32_t present = ffBinderReadI32(reply.data, reply.size, offset);
        offset += sizeof(int32_t);
        if (present != 1) {
            continue;
        }

        int32_t type = 0;
        int32_t role = 0;
        bool ended = false;
        if (!soundAndroidReadDevice(reply.data, reply.size, &offset, &type, &role, &ended)) {
            FF_DEBUG("The device list ended after %d of %d entries", index, count);
            break;
        }

        if (role == 2 && name == nullptr) {
            // ROLE_SINK. A source would be a microphone, which the policy only answers when the
            // request asks for one, so this is a guard rather than a case that happens.
            name = soundAndroidDeviceName(type);
            deviceType = type;
            FF_DEBUG("Media output device %d of %d is type %d, which is \"%s\"",
                index + 1, count, type, name != nullptr ? name->identifier : "an unrecognised type");
        }

        if (ended) {
            // A profile list with entries in it: the next entry cannot be reached without walking
            // one, and the first device is the one that matters.
            FF_DEBUG("Device %d carries audio profiles, so the rest of the list is not read", index);
            break;
        }
    }

    if (name == nullptr) {
        return "The audio service selected no output device";
    }

    int32_t volume = 0;
    int32_t minimum = 0;
    int32_t maximum = 0;
    int32_t mute = 0;
    error = soundAndroidCallInt(&binder, service.handle, FF_SOUND_ANDROID_GET_STREAM_VOLUME, codes.getStreamVolume, true, (int32_t) FF_SOUND_ANDROID_STREAM_MUSIC, &volume);
    if (error == nullptr) {
        error = soundAndroidCallInt(&binder, service.handle, FF_SOUND_ANDROID_GET_STREAM_MIN_VOLUME, codes.getStreamMinVolume, true, (int32_t) FF_SOUND_ANDROID_STREAM_MUSIC, &minimum);
    }
    if (error == nullptr) {
        error = soundAndroidCallInt(&binder, service.handle, FF_SOUND_ANDROID_GET_STREAM_MAX_VOLUME, codes.getStreamMaxVolume, true, (int32_t) FF_SOUND_ANDROID_STREAM_MUSIC, &maximum);
    }
    if (error == nullptr) {
        error = soundAndroidCallInt(&binder, service.handle, FF_SOUND_ANDROID_IS_STREAM_MUTE, codes.isStreamMute, true, (int32_t) FF_SOUND_ANDROID_STREAM_MUSIC, &mute);
    }
    if (error != nullptr) {
        return error;
    }

    uint8_t percent = soundAndroidPercent(volume, minimum, maximum, mute);
    FF_DEBUG("Stream %d is %d of %d..%d, muted %d, which is %s",
        (int) FF_SOUND_ANDROID_STREAM_MUSIC, volume, minimum, maximum, mute,
        percent != FF_SOUND_VOLUME_UNKNOWN ? "a percentage" : "unknown");

    // `isMusicActive` is what tells "this device is the output" apart from "this device is playing".
    // It answers about the media stream as a whole and not about one device, so it only ever adds
    // the ACTIVE bit to the device the policy already selected. A build that does not declare the
    // method, or a service that refuses it, leaves the bit clear rather than failing the module.
    int32_t active = 0;
    const char* activeError = soundAndroidCallInt(&binder, service.handle, FF_SOUND_ANDROID_IS_MUSIC_ACTIVE, codes.isMusicActive, true, 0, &active);
    if (activeError != nullptr) {
        FF_DEBUG("Whether anything is playing is not known: %s", activeError);
        active = 0;
    }

    const FFSoundType type = (FFSoundType) (FF_SOUND_TYPE_MAIN | (active != 0 ? FF_SOUND_TYPE_ACTIVE : FF_SOUND_TYPE_NONE));
    if ((options->soundType & FF_SOUND_TYPE_MAIN) && !(type & FF_SOUND_TYPE_MAIN)) {
        return nullptr;
    }
    if ((options->soundType & FF_SOUND_TYPE_ACTIVE) && !(type & FF_SOUND_TYPE_ACTIVE)) {
        return nullptr;
    }

    // A Bluetooth sink is reported by the name of the device behind it when the service can be
    // asked, and by its type otherwise. Falling back is not an error path: a build that does not
    // declare the method, a service that refuses it, and a service that has no name to give all end
    // in the type name, which is what this module printed before it asked at all.
    FF_STRBUF_AUTO_DESTROY bluetoothName = ffStrbufCreate();
    if (soundAndroidIsBluetooth(deviceType)) {
        const char* nameError = soundAndroidCallBtName(&binder, service.handle, codes.getBtActiveDeviceName, &bluetoothName);
        if (nameError != nullptr || bluetoothName.length == 0) {
            FF_DEBUG("The Bluetooth device is not named: %s",
                nameError != nullptr ? nameError : "the service answered an empty name");
        }
    }

    FFSoundDevice* device = FF_LIST_ADD(FFSoundDevice, *devices);
    // The identifier stays the type: it is the machine readable half of the pair, and the Bluetooth
    // address the reply carries is not usable as one -- `AudioService` blanks it to
    // `XX:XX:XX:XX:D8:61` for an unprivileged caller.
    ffStrbufInitS(&device->identifier, name->identifier);
    if (bluetoothName.length > 0) {
        ffStrbufInitMove(&device->name, &bluetoothName);
    } else {
        ffStrbufInitS(&device->name, name->name);
    }
    ffStrbufInitStatic(&device->platformApi, "AudioService");
    device->volume = percent;
    device->type = type;
    FF_DEBUG("Reporting \"%s\" (%s) at %u%%%s", device->name.chars, device->identifier.chars, device->volume,
        active != 0 ? ", active" : "");
    return nullptr;
}

const char* ffDetectSound(FFSoundOptions* options, FFlist* devices) {
    // The binder route below asks the policy about Android's own audio, which is the audio that is
    // audible only while SurfaceFlinger is the display server. A Termux:X11 or Wayland session runs
    // a real desktop on top of the device, and the sound there belongs to that desktop, not to
    // Android -- so that case is handed to the Linux implementation, which speaks PulseAudio. Sound
    // is only reported for one of the two, never merged.
    const FFDisplayServerResult* wm = ffConnectDisplayServer();
    if (!ffStrbufIgnCaseEqualS(&wm->wmProtocolName, FF_WM_PROTOCOL_SURFACEFLINGER)) {
        FF_DEBUG("The display server is \"%s\", so the Linux implementation answers", wm->wmProtocolName.chars);
        const char* ffDetectSoundLinux(FFSoundOptions* options, FFlist* devices);
        return ffDetectSoundLinux(options, devices);
    }

    return detectNative(options, devices);
}
