#include "wifi.h"

#include "common/android/api.h"
#include "common/android/binder.h"
#include "common/android/dex.h"
#include "common/debug.h"

#include <ifaddrs.h>
#include <net/if.h>
#include <unistd.h>

// Android gives an app two ways to see the current connection, and both are weaker than the desktop
// ones. `/system/bin/cmd wifi status` prints everything fastfetch wants, but it does not run in the
// calling process: `cmd` hands the arguments to the service over binder and the command body runs
// inside system_server, where it reads a locally built WifiInfo. An app can therefore not ask for
// it -- the call comes back as a silent exit status 255 with nothing on either stream, which is
// what makes it look like it worked. `dumpsys wifi` needs android.permission.DUMP, and
// /proc/net/wireless, /proc/net/dev and /sys/class/net/wlan0 are all EACCES. That leaves
// `IWifiManager.getConnectionInfo`, which needs only ACCESS_WIFI_STATE -- a normal permission,
// granted without asking. There is no Termux:API fallback: it returned less than this does, and
// needed a separate app installed.
//
// The transaction code is read out of the device's own jar rather than carried as a table, so the
// module is not tied to a list of releases. It does need the jar to exist, and Android 11 is the
// release that moved the Wi-Fi framework into an APEX -- before that the class sat in framework.jar,
// which is not read here. So Android 11 (API 30) is the floor, and an older release reports an error
// rather than a guess.
//
// The reply layout itself has been checked against the AOSP source as far back as Android 9, which
// is why nothing about it is release-gated: the head grew a link speed in Android 10 and a Wi-Fi
// standard in Android 11, and both are found by shape.
//
// Whether the radio is on is a second call, `IWifiManager.getWifiEnabledState`, which takes no
// arguments and answers with a `WifiManager.WIFI_STATE_*`. It is needed because the interface
// cannot answer for itself: bionic's `getifaddrs` lists only the interfaces that hold an address,
// so the flags that would say whether `wlan0` is up leave the list exactly when there is no
// association -- and nothing else in the reply separates a Wi-Fi that is off from one that is on
// with nothing behind it. Both answer with an empty `WifiInfo`, the same at every offset, down to
// the `02:00:00:00:00:00` placeholder it writes in place of a BSSID. The call is made only when the
// interface was not in the list, so a connected device -- where `IFF_UP` is both closer to hand and
// more accurate -- pays nothing for it.
//
// Three things about the call are positional rather than negotiated, and all three were measured
// on a vivo V2505A (Android 16) and a Redmi 9A (Android 11):
//
//   * The transaction code is a build-time constant of the `.aidl` (`getConnectionInfo` is 29 on
//     Android 11 and 41 on Android 16) -- see common/android/dex.h.
//   * The reply layout drifts, and not only by release. Android 12 dropped the duplicate length word
//     that Android 11 and 10 wrote in front of the SSID octets, and the vivo's Android 16 build
//     fills an int between the transmit and the receive speed that the Redmi's Android 11 build
//     leaves out -- while AOSP's own Android 16 writes the head exactly the Android 11 way, so that
//     one int is a vendor addition rather than a release difference. Frequency is the only channel
//     centre in the head, so it is located by value, with the whole result validated before it is
//     used. A wrong guess shows up as a rejected layout, not as a plausible wrong number.
//   * The tail of the parcel, which holds the Wi-Fi standard, is not laid out the same way by every
//     vendor: the Android 11 build on the Redmi carries three words there that AOSP's own Android 11
//     does not. It is located by shape for that reason.
//
// BSSID and MAC are the one place the binder route beats `cmd wifi status`: MAC addresses are
// redacted for a caller targeting a recent SDK, and the shell targets the current one, so `cmd wifi
// status` prints 24:**:**:**:70:2c where this reads 24:a4:87:3c:70:2c.
//
// What is deliberately left empty:
//
//   * `security`. `WifiInfo` carries no security type on Android 11 at all, and on Android 16 the
//     value the shell command prints is not in the parcel either -- `cmd wifi status` shows it only
//     because it reads the object in system_server. No other call an app can make returns it, so
//     the field stays empty rather than being guessed.
//   * The IPv4 address. It is in the parcel, but FFWifiConnection has no field for it and
//     `inf.description` is the interface name, as on every other platform. The LocalIP module
//     reports addresses.
//   * `inf.description` while the interface is not in the address list. Its name is not knowable
//     from anywhere else -- sysfs and /proc/net are EACCES -- so the field stays empty rather than
//     naming an interface that was guessed at. `inf.status` is still reported, from the radio.

#define FF_WIFI_ANDROID_SERVICE "wifi"
#define FF_WIFI_ANDROID_DESCRIPTOR "android.net.wifi.IWifiManager"

// Android 11 is the release that moved the Wi-Fi framework out of framework.jar and into an APEX,
// which is where this jar is. Nothing older carries it, so there is no second path to try: the
// class is either in the APEX or the release is too old.
#define FF_WIFI_ANDROID_JAR "/apex/com.android.wifi/javalib/framework-wifi.jar"
#define FF_WIFI_ANDROID_STUB "Landroid/net/wifi/IWifiManager$Stub;"
#define FF_WIFI_ANDROID_GET_CONNECTION_INFO "TRANSACTION_getConnectionInfo"
// Whether the radio is on is the one thing the connection does not say. An empty `WifiInfo` is what
// the service answers with both while Wi-Fi is off and while it is on with nothing associated, so
// the two states are told apart by asking the service directly. The method takes no arguments and
// answers with a `WifiManager.WIFI_STATE_*`.
#define FF_WIFI_ANDROID_GET_WIFI_ENABLED_STATE "TRANSACTION_getWifiEnabledState"
#define FF_WIFI_ANDROID_WIFI_STATE_DISABLED 1
#define FF_WIFI_ANDROID_WIFI_STATE_UNKNOWN 4

// The other argument is the caller's package name. The shell UID owns exactly one package and the
// service accepts that name, which is what `cmd` and `dumpsys` pass. See getOwnPackage().
#define FF_WIFI_ANDROID_SHELL_UID 2000
#define FF_WIFI_ANDROID_SHELL_PACKAGE "com.android.shell"

// An AIDL reply opens with the exception code and, for a Parcelable return, a non-null marker;
// WifiInfo itself then starts at 8. The network id, the RSSI and the link speed have held those
// offsets from Android 9 to 16, and the transmit speed joined them in Android 10 without moving
// since, so all four are read at fixed offsets. Everything after them is located by shape.
#define FF_WIFI_ANDROID_OFF_NET_ID 8
#define FF_WIFI_ANDROID_OFF_RSSI 12
#define FF_WIFI_ANDROID_OFF_LINK_SPEED 16
#define FF_WIFI_ANDROID_OFF_TX_LINK_SPEED 20

// A `WifiInfo` with no connection to describe carries the sentinels for the two fields that are
// always at a fixed offset: -1 for a network id it does not have and -127 for a signal it cannot
// measure. Both were measured on the vivo's Android 16 reply, which is what the service answers
// while Wi-Fi is on with nothing associated -- the head holds no frequency and no SSID at all, only
// the "02:00:00:00:00:00" placeholder that `WifiInfo` writes in place of a BSSID.
#define FF_WIFI_ANDROID_NET_ID_NONE (-1)
#define FF_WIFI_ANDROID_RSSI_NONE (-127)

// Frequency is looked for in the head, after the link speeds and before the address. See the note
// above for why it is not at a fixed offset.
#define FF_WIFI_ANDROID_FREQ_SCAN_FROM 0x14
#define FF_WIFI_ANDROID_FREQ_SCAN_TO 0x34

// Distances from Frequency. AOSP writes an address as a flag byte followed by a byte array, and
// omits the array entirely when there is no IPv4 address -- an IPv6-only network does that -- which
// shifts everything after it by two words. The base of the SSID section is therefore derived from
// that flag rather than fixed, and only the SSID's own offset is left as a pair: Android 11 wrote
// the length twice in front of the octets and Android 12 dropped the duplicate.
//
// The SSID is the one field WifiInfo writes as raw bytes rather than as a UTF-16 string, so its
// length is a byte count and a NUL among the bytes means the offset was wrong.
#define FF_WIFI_ANDROID_FREQ_HAS_IP 0x04
#define FF_WIFI_ANDROID_FREQ_IP_LENGTH 0x08
#define FF_WIFI_ANDROID_FREQ_SSID_BASE 0x10
#define FF_WIFI_ANDROID_FREQ_SSID_BASE_NO_IP 0x08
#define FF_WIFI_ANDROID_SSID_LENGTH 0x04

// The receive speed is the word in front of Frequency; the transmit speed is the word after the link
// speed. The vivo writes one extra word between those two that AOSP does not -- it repeats the
// transmit speed -- so the receive speed can only be reached from Frequency, and Frequency cannot be
// reached by stepping over the two link speeds. Both were checked against `cmd wifi status` at the
// same moment: 131/219 on the vivo, 86/-1 on the Redmi.
#define FF_WIFI_ANDROID_FREQ_RX_LINK_SPEED 0x04
#define FF_WIFI_ANDROID_SSID 0x08        // Android 12 and later
#define FF_WIFI_ANDROID_SSID_LEGACY 0x0c // Android 11 and older
#define FF_WIFI_ANDROID_SSID_MAX_LENGTH 32

// `WifiInfo` writes the Wi-Fi standard as one of the `ScanResult.WIFI_STANDARD_*` values. It is
// followed by the two maximum-supported link speeds, which is what makes it recognisable: an int
// naming a standard, then two plausible rates. The band is checked against it as well, so a value
// that only fits another band is rejected. A layout that is not recognised leaves `protocol` empty
// instead of reporting the wrong standard.
#define FF_WIFI_ANDROID_STANDARD_LEGACY 1
#define FF_WIFI_ANDROID_STANDARD_11A 2
#define FF_WIFI_ANDROID_STANDARD_11B 3
#define FF_WIFI_ANDROID_STANDARD_11N 4
#define FF_WIFI_ANDROID_STANDARD_11AC 5
#define FF_WIFI_ANDROID_STANDARD_11AX 6
#define FF_WIFI_ANDROID_STANDARD_11AD 7
#define FF_WIFI_ANDROID_STANDARD_11BE 8
#define FF_WIFI_ANDROID_RATE_MAX 20000

// The whole WifiInfo parcel measured 1340 bytes on Android 16, most of it the MLO and ANQP tail.
#define FF_WIFI_ANDROID_REPLY_SIZE 4096

// The layout is found by shape rather than by offset, so a rejected reply leaves nothing to go on.
// The debug dump covers the head -- where the frequency, the address flag and the SSID sit -- and
// stops before the tail, which is the long MLO and ANQP list and takes no part in locating
// anything. Read with `fastfetch -s Wifi` under a debug build.
#define FF_WIFI_ANDROID_DEBUG_DUMP_SIZE 0x60

// ---------------------------------------------------------------------------------------------
// Binder
// ---------------------------------------------------------------------------------------------

// `getConnectionInfo` takes the caller's package name, and whether it is checked depends on the
// release: the vivo answered "Package com.termux does not belong to 2000" for a name the shell does
// not own, while the Redmi accepted the same name. Passing the real one is what works on both. There
// is no way for a process to ask for its own package name -- it is not in /proc/self/status, and an
// app cannot list /data/data -- but the executable path carries it: an app's binaries live under
// /data/data/<package>/ or /data/user/<user>/<package>/, and /proc/self/exe resolves there.
//
// A binary outside those directories has no package of its own, and that is not an edge case: a
// static build pushed to /data/local/tmp is how this runs on a device without Termux, and the Redmi
// reported "Cannot determine the package name of this process" for exactly that. The shell UID can
// be answered instead, because it has one name the service accepts. No other UID reaches here: an
// app's own binaries are always under its data directory, so a failure there is a real failure.
static bool getOwnPackage(char* buffer, size_t capacity) {
    char path[4096];
    const ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (length <= 0) {
        FF_DEBUG("Cannot read /proc/self/exe, so the calling package cannot be derived");
        return false;
    }
    path[length] = '\0';

    const char* rest = nullptr;
    const char* const dataPrefix = "/data/data/";
    const char* const userPrefix = "/data/user/";
    if (strncmp(path, dataPrefix, strlen(dataPrefix)) == 0) {
        rest = path + strlen(dataPrefix);
    } else if (strncmp(path, userPrefix, strlen(userPrefix)) == 0) {
        rest = strchr(path + strlen(userPrefix), '/');
        if (rest != nullptr) {
            rest += 1;
        }
    }
    if (rest == nullptr) {
        FF_DEBUG("The executable is not in an app data directory (\"%s\"), uid %u", path, (unsigned) getuid());
        if (getuid() != FF_WIFI_ANDROID_SHELL_UID) {
            return false;
        }
        static const char shellPackage[] = FF_WIFI_ANDROID_SHELL_PACKAGE;
        if (sizeof(shellPackage) > capacity) {
            return false;
        }
        memcpy(buffer, shellPackage, sizeof(shellPackage));
        FF_DEBUG("The calling package is \"%s\" (the shell owns it)", buffer);
        return true;
    }

    const char* end = strchr(rest, '/');
    const size_t nameLength = end != nullptr ? (size_t) (end - rest) : strlen(rest);
    if (nameLength == 0 || nameLength >= capacity) {
        return false;
    }
    memcpy(buffer, rest, nameLength);
    buffer[nameLength] = '\0';
    FF_DEBUG("The calling package is \"%s\"", buffer);
    return true;
}

static bool isMacAddress(const char* value, uint32_t length) {
    if (length != 17) {
        return false;
    }
    for (uint32_t i = 0; i < length; ++i) {
        const char c = value[i];
        if (i % 3 == 2) {
            if (c != ':') {
                return false;
            }
        } else if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    return true;
}

// SupplicantState, in the order AOSP declares it.
static const char* const FF_WIFI_ANDROID_STATES[] = {
    "DISCONNECTED", "INTERFACE_DISABLED", "INACTIVE", "SCANNING", "AUTHENTICATING", "ASSOCIATING",
    "ASSOCIATED", "FOUR_WAY_HANDSHAKE", "GROUP_HANDSHAKE", "COMPLETED", "DORMANT", "UNINITIALIZED", "INVALID",
};

static bool isSupplicantState(const char* value, uint32_t length) {
    for (uint32_t i = 0; i < ARRAY_SIZE(FF_WIFI_ANDROID_STATES); ++i) {
        if (strlen(FF_WIFI_ANDROID_STATES[i]) == length && memcmp(FF_WIFI_ANDROID_STATES[i], value, length) == 0) {
            return true;
        }
    }
    return false;
}

typedef struct FFWifiAndroidConnection {
    char interface[IF_NAMESIZE + 1];
    char state[24];
    char ssid[FF_WIFI_ANDROID_SSID_MAX_LENGTH + 1];
    char bssid[18];
    int32_t rssi;
    int32_t linkSpeed;
    int32_t txLinkSpeed;
    int32_t rxLinkSpeed;
    int32_t standard;
    size_t stateEnd;
    uint16_t frequency;
    bool up;
    bool upKnown; // whether the state of the interface was established at all
    bool connected;
} FFWifiAndroidConnection;

// The Wi-Fi interface is the one the HAL names `wlan*`, and it is looked up rather than assumed so
// that a second one (`wlan1` on a dual-STA device) is still reported by name. The flags describe the
// interface, so the first entry for a name is as good as any other.
//
// What bionic's `getifaddrs` will not do is list an interface that has no address: the list is
// built out of the addresses, so `wlan0` leaves it as soon as nothing is associated -- measured on
// the vivo, and the Redmi behaves the same. That is exactly the state its flags are wanted in, so
// when the name is missing the radio state stands in for them. See detectWithBinder().
static void detectInterface(FFWifiAndroidConnection* connection) {
    struct ifaddrs* addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        FF_DEBUG("getifaddrs failed, so neither the interface name nor its state is known");
        return;
    }

    for (const struct ifaddrs* ifa = addrs; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_name == nullptr || strncmp(ifa->ifa_name, "wlan", 4) != 0) {
            continue;
        }
        snprintf(connection->interface, sizeof(connection->interface), "%s", ifa->ifa_name);
        connection->up = (ifa->ifa_flags & IFF_UP) != 0;
        connection->upKnown = true;
        FF_DEBUG("Interface \"%s\" is %s (IFF_UP %s)", connection->interface, connection->up ? "up" : "down", connection->up ? "set" : "clear");
        break;
    }
    freeifaddrs(addrs);
    if (connection->interface[0] == '\0') {
        FF_DEBUG("No interface named wlan* is present");
    }
}

static const char* wifiStandardName(int32_t standard) {
    switch (standard) {
        case FF_WIFI_ANDROID_STANDARD_LEGACY: return "802.11";
        case FF_WIFI_ANDROID_STANDARD_11A: return "802.11a";
        case FF_WIFI_ANDROID_STANDARD_11B: return "802.11b";
        case FF_WIFI_ANDROID_STANDARD_11N: return "802.11n (Wi-Fi 4)";
        case FF_WIFI_ANDROID_STANDARD_11AC: return "802.11ac (Wi-Fi 5)";
        case FF_WIFI_ANDROID_STANDARD_11AX: return "802.11ax (Wi-Fi 6)";
        case FF_WIFI_ANDROID_STANDARD_11AD: return "802.11ad (WiGig)";
        case FF_WIFI_ANDROID_STANDARD_11BE: return "802.11be (Wi-Fi 7)";
        default: return nullptr;
    }
}

// 11ac and 11ad live in bands the connection is not in when it reports a 2.4 GHz frequency, so a
// candidate that disagrees with the band is a misread rather than a standard.
static bool isStandardPlausible(int32_t standard, uint16_t frequency) {
    if (frequency < 3000 && (standard == FF_WIFI_ANDROID_STANDARD_11AC || standard == FF_WIFI_ANDROID_STANDARD_11AD)) {
        return false;
    }
    if (frequency > 50000 && standard != FF_WIFI_ANDROID_STANDARD_11AD) {
        return false;
    }
    return true;
}

// WifiInfo writes the BSSID, the MAC and the supplicant state as length-prefixed UTF-16 strings, and
// their distance from the head depends on how long the SSID is and on which release wrote the
// parcel. They are recognised by shape instead of by offset: a 17 character `hh:hh:hh:hh:hh:hh`, and
// one of the thirteen state names. The BSSID is written before the MAC, so the first match is the
// one wanted. `from` is the end of the SSID octets, which is where the BSSID follows.
static void findStrings(const uint8_t* data, size_t size, size_t from, FFWifiAndroidConnection* connection) {
    for (size_t offset = from; offset + 4 <= size; ++offset) {
        const uint32_t length = ffBinderReadU32(data, size, offset);
        if (length == 0 || length > 24 || offset + 4 + (size_t) length * 2 > size) {
            continue;
        }

        const uint8_t* chars = data + offset + 4;
        char buffer[25];
        bool ascii = true;
        for (uint32_t i = 0; i < length; ++i) {
            if (chars[i * 2 + 1] != 0 || chars[i * 2] < 0x20 || chars[i * 2] >= 0x7f) {
                ascii = false;
                break;
            }
            buffer[i] = (char) chars[i * 2];
        }
        if (!ascii) {
            continue;
        }
        buffer[length] = '\0';

        if (connection->bssid[0] == '\0' && isMacAddress(buffer, length)) {
            memcpy(connection->bssid, buffer, length + 1);
        } else if (connection->state[0] == '\0' && isSupplicantState(buffer, length)) {
            memcpy(connection->state, buffer, length + 1);
            // The state is the last string before the tail, so where it ends is where the tail
            // starts. A Parcel pads a string to the next word, terminator included.
            connection->stateEnd = offset + 4 + ((length * 2 + 2 + 3) & ~3u);
        }
        if (connection->bssid[0] != '\0' && connection->state[0] != '\0') {
            return;
        }
    }
}

// AOSP writes the Wi-Fi standard between three nullable strings and the two maximum-supported link
// speeds, all of them ints. Vendors insert fields around that run -- the Android 11 build on the
// Redmi carries three words before the standard that AOSP's own Android 11 does not -- so a fixed
// offset would be a guess. The run is recognised by shape instead, searching forward from the end of
// the supplicant state: an int naming a standard, followed by two rates, in a band the standard can
// live in.
static void findWifiStandard(const uint8_t* data, size_t size, FFWifiAndroidConnection* connection) {
    if (connection->stateEnd == 0) {
        return; // without the state string there is no trustworthy place to start from
    }

    for (size_t offset = connection->stateEnd; offset + 12 <= size; offset += 4) {
        const int32_t standard = ffBinderReadI32(data, size, offset);
        if (wifiStandardName(standard) == nullptr || !isStandardPlausible(standard, connection->frequency)) {
            continue;
        }
        const int32_t maxTx = ffBinderReadI32(data, size, offset + 4);
        const int32_t maxRx = ffBinderReadI32(data, size, offset + 8);
        if (maxTx < 0 || maxTx > FF_WIFI_ANDROID_RATE_MAX || maxRx < 0 || maxRx > FF_WIFI_ANDROID_RATE_MAX) {
            continue;
        }
        connection->standard = standard;
        FF_DEBUG("Wi-Fi standard %d at +0x%zx, maximum speeds %d/%d", standard, offset, maxTx, maxRx);
        return;
    }
    FF_DEBUG("No Wi-Fi standard matched after the supplicant state at +0x%zx", connection->stateEnd);
}

#ifndef NDEBUG
// The reply is read by shape, so a reply that fails every shape leaves nothing behind to diagnose
// it with: the bytes are the only evidence. Each line carries the words next to their bytes,
// because what the reader is looking for -- a value that would pass for a frequency, an address
// flag, a string length -- is an int.
static void debugDumpReply(const uint8_t* data, size_t size) {
    const size_t limit = size < FF_WIFI_ANDROID_DEBUG_DUMP_SIZE ? size : FF_WIFI_ANDROID_DEBUG_DUMP_SIZE;
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
    #define FF_WIFI_ANDROID_DEBUG_DUMP(data, size) debugDumpReply(data, size)
#else
    #define FF_WIFI_ANDROID_DEBUG_DUMP(data, size) ((void) 0)
#endif

static const char* parseConnectionInfo(const uint8_t* data, size_t size, FFWifiAndroidConnection* connection) {
    if (size < FF_WIFI_ANDROID_OFF_TX_LINK_SPEED + 4) {
        FF_DEBUG("The reply is %zu bytes, too short to hold a WifiInfo", size);
        return "The reply is too short to hold a WifiInfo";
    }
    const int32_t exception = ffBinderReadI32(data, size, 0);
    if (exception != 0) {
        FF_DEBUG("The Wifi service raised exception %d", exception);
        return "The Wifi service raised an exception";
    }
    if (ffBinderReadU32(data, size, 4) == 0) {
        // A null WifiInfo is what the service returns when it has nothing to report, which is the
        // normal reply while Wi-Fi is off. The interface, when it is still there, is reported as
        // disconnected.
        FF_DEBUG("The reply is %zu bytes and carries no WifiInfo", size);
        return nullptr;
    }
    FF_DEBUG("Reply is %zu bytes: netId %d, rssi %d, link speed %d, tx link speed %d",
        size,
        ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_NET_ID),
        ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_RSSI),
        ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_LINK_SPEED),
        ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_TX_LINK_SPEED));

    // The two orders WifiSsid has written its length in: once before the octets on Android 11 and
    // older, not at all on Android 12 and later, which leaves the octets one word earlier. The one
    // that matches this release is tried first.
    uint32_t ssidOffsets[2] = { FF_WIFI_ANDROID_SSID_LEGACY, FF_WIFI_ANDROID_SSID };
    if (FF_API_AT_LEAST(31)) {
        ssidOffsets[0] = FF_WIFI_ANDROID_SSID;
        ssidOffsets[1] = FF_WIFI_ANDROID_SSID_LEGACY;
    }

    for (uint32_t base = FF_WIFI_ANDROID_FREQ_SCAN_FROM; base <= FF_WIFI_ANDROID_FREQ_SCAN_TO; base += 4) {
        const uint16_t frequency = (uint16_t) ffBinderReadU32(data, size, base);
        if (frequency == 0 || ffWifiFreqToChannel(frequency) == 0) {
            continue;
        }
        FF_DEBUG("+0x%02x holds %u, which is channel %u", base, frequency, ffWifiFreqToChannel(frequency));

        const int32_t hasIp = ffBinderReadI32(data, size, base + FF_WIFI_ANDROID_FREQ_HAS_IP);
        if (hasIp != 0 && hasIp != 1) {
            FF_DEBUG("  rejected: +0x%02x is not an address flag but %d", base + FF_WIFI_ANDROID_FREQ_HAS_IP, hasIp);
            continue;
        }
        const uint32_t ipLength = ffBinderReadU32(data, size, base + FF_WIFI_ANDROID_FREQ_IP_LENGTH);
        if (hasIp == 1 && ipLength != 4) {
            FF_DEBUG("  rejected: the address at +0x%02x is %u bytes long", base + FF_WIFI_ANDROID_FREQ_IP_LENGTH, ipLength);
            continue;
        }
        const uint32_t ssidBase = base + (hasIp == 1 ? FF_WIFI_ANDROID_FREQ_SSID_BASE : FF_WIFI_ANDROID_FREQ_SSID_BASE_NO_IP);
        const int32_t ssidPresent = ffBinderReadI32(data, size, ssidBase);
        if (ssidPresent != 1) {
            FF_DEBUG("  rejected: the SSID at +0x%02x is marked %d, not present", ssidBase, ssidPresent);
            continue; // the SSID of a reported connection is never null
        }

        const uint32_t ssidLength = ffBinderReadU32(data, size, ssidBase + FF_WIFI_ANDROID_SSID_LENGTH);
        if (ssidLength == 0 || ssidLength > FF_WIFI_ANDROID_SSID_MAX_LENGTH) {
            FF_DEBUG("  rejected: the SSID length at +0x%02x is %u", ssidBase + FF_WIFI_ANDROID_SSID_LENGTH, ssidLength);
            continue;
        }

        for (uint32_t i = 0; i < ARRAY_SIZE(ssidOffsets); ++i) {
            const size_t ssidOffset = ssidBase + ssidOffsets[i];
            if (ssidOffset + ssidLength > size) {
                continue;
            }

            // The SSID is a raw byte string, so a NUL in it means this was not the SSID after all.
            if (memchr(data + ssidOffset, '\0', ssidLength) != nullptr) {
                FF_DEBUG("  rejected: the %u SSID bytes at +0x%zx hold a NUL", ssidLength, ssidOffset);
                continue;
            }
            memcpy(connection->ssid, data + ssidOffset, ssidLength);
            connection->ssid[ssidLength] = '\0';

            connection->frequency = frequency;
            connection->rssi = ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_RSSI);
            connection->linkSpeed = ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_LINK_SPEED);
            connection->txLinkSpeed = ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_TX_LINK_SPEED);
            connection->rxLinkSpeed = ffBinderReadI32(data, size, base - FF_WIFI_ANDROID_FREQ_RX_LINK_SPEED);

            findStrings(data, size, ssidOffset + ssidLength, connection);
            FF_DEBUG("SSID \"%s\" at +0x%zx, BSSID \"%s\", supplicant state \"%s\"",
                connection->ssid, ssidOffset, connection->bssid, connection->state);
            // The supplicant state is the authoritative signal; the network id is the fallback for a
            // parcel whose state string was not recognised.
            connection->connected = connection->state[0] != '\0'
                ? strcmp(connection->state, "COMPLETED") == 0
                : ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_NET_ID) >= 0;
            findWifiStandard(data, size, connection);
            FF_DEBUG("Standard %d (%s), rx link speed %d, connected %s",
                connection->standard, wifiStandardName(connection->standard) ?: "unknown", connection->rxLinkSpeed,
                connection->connected ? "yes" : "no");
            return nullptr;
        }
    }

    // Nothing in the head matched, but an unassociated connection has nothing in the head to match:
    // no frequency, no SSID and -- unlike a connection -- the two sentinels `WifiInfo` fills in when
    // it has nothing to describe. That reply is the service saying there is nothing to report, which
    // is a state and not a failure. A reply carrying a network id of its own is a connection, so a
    // shape that still does not match one keeps being reported as a failure.
    if (ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_NET_ID) == FF_WIFI_ANDROID_NET_ID_NONE &&
        ffBinderReadI32(data, size, FF_WIFI_ANDROID_OFF_RSSI) == FF_WIFI_ANDROID_RSSI_NONE) {
        FF_DEBUG("No network id and no signal in the %zu byte reply: there is no connection to report", size);
        return nullptr;
    }

    FF_DEBUG("No frequency, no SSID and no supplicant state matched in this reply:");
    FF_WIFI_ANDROID_DEBUG_DUMP(data, size);
    return "The Wifi service returned a layout fastfetch does not understand";
}

// Which of the speeds the service wrote becomes a rate. It writes a negative one when it does not
// know, which is what the Redmi does for the receive side, and a link speed of 0 when the link is
// down. `generic` is the fallback for a direction the service left unset: the transmit side has one
// -- the generic link speed is a real negotiated rate, just not direction-specific -- the way the
// Linux backend falls back to SIOCGIWRATE when the station info carries no bitrate. There is no
// generic receive speed, so 0 is passed there.
static double wifiAndroidRate(int32_t specific, int32_t generic) {
    const int32_t rate = specific > 0 ? specific : generic;
    return rate > 0 && rate <= FF_WIFI_ANDROID_RATE_MAX ? (double) rate : -DBL_MAX;
}

// Calls a method of the service that takes no arguments and answers with an int. The transaction
// code comes out of the jar the same way `getConnectionInfo`'s does, so a release that does not
// declare the method answers UNKNOWN_TRANSACTION instead of a number that means something else.
static const char* callIntMethod(FFBinder* binder, uint32_t handle, const char* transactionField, int32_t* result) {
    int32_t transaction = 0;
    const char* error = ffDexStaticInt(FF_WIFI_ANDROID_JAR, FF_WIFI_ANDROID_STUB, transactionField, &transaction);
    if (error != nullptr) {
        return error;
    }

    uint8_t parcelBuffer[256];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_WIFI_ANDROID_DESCRIPTOR);

    uint8_t replyBuffer[64];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    error = ffBinderTransact(binder, handle, (uint32_t) transaction, &parcel, &reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("\"%s\" resolved to transaction %d, which this build does not answer", transactionField, transaction);
        return "The Wifi service does not answer that request";
    }
    const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
    if (exception != 0) {
        FF_DEBUG("\"%s\" is transaction %d and raised exception %d", transactionField, transaction, exception);
        return "The Wifi service raised an exception";
    }
    *result = ffBinderReadI32(reply.data, reply.size, 4);
    FF_DEBUG("\"%s\" is transaction %d and answered %d", transactionField, transaction, *result);
    return nullptr;
}

static const char* detectWithBinder(FFlist* result) {
    char package[128];
    if (!getOwnPackage(package, sizeof(package))) {
        return "Cannot determine the package name of this process";
    }

    int32_t transaction = 0;
    const char* error = ffDexStaticInt(FF_WIFI_ANDROID_JAR, FF_WIFI_ANDROID_STUB, FF_WIFI_ANDROID_GET_CONNECTION_INFO, &transaction);
    if (error != nullptr) {
        FF_DEBUG("Reading the transaction code from \"%s\" failed: %s", FF_WIFI_ANDROID_JAR, error);
        return error;
    }

    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    error = ffBinderOpen(&binder);
    if (error != nullptr) {
        return error;
    }

    uint32_t handle = 0;
    error = ffBinderLookupService(&binder, FF_WIFI_ANDROID_SERVICE, FF_BINDER_SM_GET_SERVICE, &handle);
    if (error != nullptr) {
        return error;
    }
    FF_DEBUG("The \"%s\" service is handle %u, %s is transaction %d",
        FF_WIFI_ANDROID_SERVICE, handle, FF_WIFI_ANDROID_GET_CONNECTION_INFO, transaction);

    uint8_t parcelBuffer[256];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_WIFI_ANDROID_DESCRIPTOR);
    ffBinderParcelPutString16(&parcel, package);
    // callingFeatureId is nullable, and a null String is a length of -1 rather than an empty string.
    ffBinderParcelPutI32(&parcel, -1);

    uint8_t replyBuffer[FF_WIFI_ANDROID_REPLY_SIZE];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    error = ffBinderTransact(&binder, handle, (uint32_t) transaction, &parcel, &reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("The reply carries status %d instead of a parcel", (int) ffBinderReadI32(reply.data, reply.size, 0));
        return "Wifi service rejected the request";
    }

    FFWifiAndroidConnection connection = {};
    detectInterface(&connection);
    error = parseConnectionInfo(reply.data, reply.size, &connection);
    if (error != nullptr) {
        return error;
    }

    if (!connection.upKnown) {
        // `getifaddrs` leaves the interface out while it has no address, so its flags are missing in
        // exactly the state they are wanted in. The radio answers for it: a Wi-Fi that is off is one
        // whose interface is down, and a Wi-Fi that is on with nothing associated keeps an interface
        // that is up. WIFI_STATE_DISABLING is a radio that is still up, so only the one value means
        // down, and WIFI_STATE_UNKNOWN is the service declining to say.
        int32_t state = 0;
        const char* stateError = callIntMethod(&binder, handle, FF_WIFI_ANDROID_GET_WIFI_ENABLED_STATE, &state);
        if (stateError == nullptr && state != FF_WIFI_ANDROID_WIFI_STATE_UNKNOWN) {
            connection.up = state != FF_WIFI_ANDROID_WIFI_STATE_DISABLED;
            connection.upKnown = true;
            FF_DEBUG("The radio is %s, which stands in for the interface flags", connection.up ? "on" : "off");
        } else {
            FF_DEBUG("The state of the radio is not known: %s",
                stateError != nullptr ? stateError : "the service answered WIFI_STATE_UNKNOWN");
        }
    }

    // Nothing was found to report on at all. That is a device without a Wi-Fi interface rather than
    // one with a Wi-Fi that is off, which has a state to print by now.
    if (!connection.upKnown && !connection.connected) {
        return "No Wi-Fi interface is present";
    }

    FFWifiResult* item = FF_LIST_ADD(FFWifiResult, *result);
    ffStrbufInit(&item->inf.description);
    ffStrbufInit(&item->inf.status);
    ffStrbufInit(&item->conn.status);
    ffStrbufInit(&item->conn.ssid);
    ffStrbufInit(&item->conn.bssid);
    ffStrbufInit(&item->conn.protocol);
    ffStrbufInit(&item->conn.security);
    item->conn.signalQuality = -DBL_MAX;
    item->conn.rxRate = -DBL_MAX;
    item->conn.txRate = -DBL_MAX;
    item->conn.channel = 0;
    item->conn.channelWidth = 0;
    item->conn.frequency = 0;

    ffStrbufSetS(&item->inf.description, connection.interface);
    // The name is empty when the interface was not in the address list, and the state is empty when
    // nothing established it: both are left empty rather than guessed at. The module prints this
    // state whenever there is no SSID to print instead, which is exactly the case where neither is
    // in the reply.
    if (connection.upKnown) {
        ffStrbufSetStatic(&item->inf.status, connection.up ? "Up" : "Down");
    }
    ffStrbufSetStatic(&item->conn.status, connection.connected ? "connected" : "disconnected");
    if (!connection.connected) {
        FF_DEBUG("Nothing is associated: interface \"%s\" is \"%s\", the connection is \"%s\"",
            connection.interface, item->inf.status.chars, item->conn.status.chars);
        return nullptr;
    }

    const char* protocol = wifiStandardName(connection.standard);
    if (protocol != nullptr) {
        ffStrbufSetStatic(&item->conn.protocol, protocol);
    }
    item->conn.signalQuality = connection.rssi >= -50 ? 100 : connection.rssi <= -100 ? 0
                                                                                     : (connection.rssi + 100) * 2;
    ffStrbufSetS(&item->conn.bssid, connection.bssid);
    ffStrbufSetS(&item->conn.ssid, connection.ssid);
    item->conn.frequency = connection.frequency;
    item->conn.txRate = wifiAndroidRate(connection.txLinkSpeed, connection.linkSpeed);
    item->conn.rxRate = wifiAndroidRate(connection.rxLinkSpeed, 0);
    item->conn.channel = ffWifiFreqToChannel(connection.frequency);
    FF_DEBUG("\"%s\" %s: \"%s\", \"%s\", %s, %u MHz (channel %u), signal %.0f, tx %d, rx %d",
        connection.interface, item->inf.status.chars, item->conn.ssid.chars, item->conn.bssid.chars,
        item->conn.protocol.length ? item->conn.protocol.chars : "(no standard)", connection.frequency,
        item->conn.channel, item->conn.signalQuality, connection.txLinkSpeed, connection.rxLinkSpeed);
    return nullptr;
}

const char* ffDetectWifi(FFlist* result) {
    return detectWithBinder(result);
}
