#include "wifi.h"
#include "common/dbus.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "util/stringUtils.h"
#include "util/debug.h"

#include <net/if.h>

#ifdef FF_HAVE_DBUS
// https://people.freedesktop.org/~lkundrak/nm-docs/nm-dbus-types.html#NM80211ApFlags
typedef enum {
    NM_802_11_AP_FLAGS_NONE    = 0x00000000,
    NM_802_11_AP_FLAGS_PRIVACY = 0x00000001,
    NM_802_11_AP_FLAGS_WPS     = 0x00000002,
    NM_802_11_AP_FLAGS_WPS_PBC = 0x00000004,
    NM_802_11_AP_FLAGS_WPS_PIN = 0x00000008,
} NM80211ApFlags;

// https://people.freedesktop.org/~lkundrak/nm-docs/nm-dbus-types.html#NM80211ApSecurityFlags
typedef enum {
    NM_802_11_AP_SEC_NONE                     = 0x00000000,
    NM_802_11_AP_SEC_PAIR_WEP40               = 0x00000001,
    NM_802_11_AP_SEC_PAIR_WEP104              = 0x00000002,
    NM_802_11_AP_SEC_PAIR_TKIP                = 0x00000004,
    NM_802_11_AP_SEC_PAIR_CCMP                = 0x00000008,
    NM_802_11_AP_SEC_GROUP_WEP40              = 0x00000010,
    NM_802_11_AP_SEC_GROUP_WEP104             = 0x00000020,
    NM_802_11_AP_SEC_GROUP_TKIP               = 0x00000040,
    NM_802_11_AP_SEC_GROUP_CCMP               = 0x00000080,
    NM_802_11_AP_SEC_KEY_MGMT_PSK             = 0x00000100,
    NM_802_11_AP_SEC_KEY_MGMT_802_1X          = 0x00000200,
    NM_802_11_AP_SEC_KEY_MGMT_SAE             = 0x00000400,
    NM_802_11_AP_SEC_KEY_MGMT_OWE             = 0x00000800,
    NM_802_11_AP_SEC_KEY_MGMT_OWE_TM          = 0x00001000,
    NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192 = 0x00002000,
} NM80211ApSecurityFlags;

#define FF_DBUS_ITER_CONTINUE(dbus, iterator) \
    { \
        if(!(dbus).lib->ffdbus_message_iter_next(iterator)) \
            break; \
        continue; \
    }

static const char* detectWifiWithNm(FFWifiResult* item, FFstrbuf* buffer)
{
    FF_DEBUG("Starting NetworkManager wifi detection for interface %s", item->inf.description.chars);
    FFDBusData dbus;
    const char* error = ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus);
    if(error)
    {
        FF_DEBUG("Failed to load DBus data: %s", error);
        return error;
    }

    {
        FF_DEBUG("Getting device by IP interface name");
        DBusMessage* device = ffDBusGetMethodReply(&dbus, "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "GetDeviceByIpIface", item->inf.description.chars, NULL);
        if(!device)
        {
            FF_DEBUG("GetDeviceByIpIface failed for interface %s", item->inf.description.chars);
            return "Failed to call GetDeviceByIpIface";
        }

        ffStrbufClear(buffer);
        DBusMessageIter rootIter;
        if(!dbus.lib->ffdbus_message_iter_init(device, &rootIter) || !ffDBusGetString(&dbus, &rootIter, buffer))
        {
            FF_DEBUG("Failed to initialize message iterator or get device path");
            dbus.lib->ffdbus_message_unref(device);
            return "Failed to get device path";
        }
        FF_DEBUG("Got device path: %s", buffer->chars);
        dbus.lib->ffdbus_message_unref(device);
    }

    if (item->conn.txRate == -DBL_MAX)
    {
        FF_DEBUG("Getting bitrate from NetworkManager");
        uint32_t bitrate;
        if (ffDBusGetPropertyUint(&dbus, "org.freedesktop.NetworkManager", buffer->chars, "org.freedesktop.NetworkManager.Device.Wireless", "Bitrate", &bitrate))
        {
            item->conn.txRate = bitrate / 1000.;
            FF_DEBUG("Got bitrate: %.2f Mbps", item->conn.txRate);
        }
        else
            FF_DEBUG("Failed to get bitrate");
    }

    FF_DEBUG("Getting active access point path");
    FF_STRBUF_AUTO_DESTROY apPath = ffStrbufCreate();
    if (!ffDBusGetPropertyString(&dbus, "org.freedesktop.NetworkManager", buffer->chars, "org.freedesktop.NetworkManager.Device.Wireless", "ActiveAccessPoint", &apPath))
    {
        FF_DEBUG("Failed to get active access point path");
        return "Failed to get active access point path";
    }
    FF_DEBUG("Got access point path: %s", apPath.chars);

    if (!item->conn.status.length)
    {
        ffStrbufSetStatic(&item->conn.status, "connected");
        FF_DEBUG("Setting connection status to 'connected'");
    }

    FF_DEBUG("Getting access point properties");
    DBusMessage* reply = ffDBusGetAllProperties(&dbus, "org.freedesktop.NetworkManager", apPath.chars, "org.freedesktop.NetworkManager.AccessPoint");
    if(reply == NULL)
    {
        FF_DEBUG("Failed to get access point properties");
        return "Failed to get access point properties";
    }

    DBusMessageIter rootIterator;
    if(!dbus.lib->ffdbus_message_iter_init(reply, &rootIterator) &&
        dbus.lib->ffdbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_ARRAY)
    {
        FF_DEBUG("Invalid type of access point properties");
        dbus.lib->ffdbus_message_unref(reply);
        return "Invalid type of access point properties";
    }

    DBusMessageIter arrayIterator;
    dbus.lib->ffdbus_message_iter_recurse(&rootIterator, &arrayIterator);

    NM80211ApFlags flags;
    NM80211ApSecurityFlags wpaFlags, rsnFlags;
    int flagCount = 0;

    FF_DEBUG("Parsing access point properties");
    while(true)
    {
        if(dbus.lib->ffdbus_message_iter_get_arg_type(&arrayIterator) != DBUS_TYPE_DICT_ENTRY)
            FF_DBUS_ITER_CONTINUE(dbus, &arrayIterator)

        DBusMessageIter dictIterator;
        dbus.lib->ffdbus_message_iter_recurse(&arrayIterator, &dictIterator);

        const char* key;
        dbus.lib->ffdbus_message_iter_get_basic(&dictIterator, &key);

        dbus.lib->ffdbus_message_iter_next(&dictIterator);

        if (ffStrEquals(key, "Ssid"))
        {
            if (!item->conn.ssid.length)
            {
                FF_DEBUG("Found SSID property");
                ffDBusGetString(&dbus, &dictIterator, &item->conn.ssid);
                FF_DEBUG("SSID: %s", item->conn.ssid.chars);
            }
        }
        else if (ffStrEquals(key, "HwAddress"))
        {
            if (!item->conn.bssid.length)
            {
                FF_DEBUG("Found HwAddress property");
                ffDBusGetString(&dbus, &dictIterator, &item->conn.bssid);
                FF_DEBUG("BSSID: %s", item->conn.bssid.chars);
            }
        }
        else if (ffStrEquals(key, "Strength"))
        {
            if (item->conn.signalQuality == -DBL_MAX)
            {
                FF_DEBUG("Found Strength property");
                uint32_t strengthPercent;
                if (ffDBusGetUint(&dbus, &dictIterator, &strengthPercent))
                {
                    item->conn.signalQuality = strengthPercent;
                    FF_DEBUG("Signal quality: %u%%", strengthPercent);
                }
            }
        }
        else if (ffStrEquals(key, "Frequency"))
        {
            if (item->conn.frequency == 0)
            {
                FF_DEBUG("Found Frequency property");
                uint32_t frequency;
                if (ffDBusGetUint(&dbus, &dictIterator, &frequency))
                {
                    item->conn.frequency = (uint16_t) frequency;
                    FF_DEBUG("Frequency: %u MHz", item->conn.frequency);
                    if (item->conn.channel == 0)
                    {
                        item->conn.channel = ffWifiFreqToChannel(item->conn.frequency);
                        FF_DEBUG("Calculated channel: %u", item->conn.channel);
                    }
                }
            }
        }
        else if ((ffStrEquals(key, "Flags") && ffDBusGetUint(&dbus, &dictIterator, &flags)) ||
            (ffStrEquals(key, "WpaFlags") && ffDBusGetUint(&dbus, &dictIterator, &wpaFlags)) ||
            (ffStrEquals(key, "RsnFlags") && ffDBusGetUint(&dbus, &dictIterator, &rsnFlags))
        )
            ++flagCount;

        FF_DBUS_ITER_CONTINUE(dbus, &arrayIterator)
    }

    if (flagCount == 3)
    {
        FF_DEBUG("Determining security type from flags (Flags: 0x%08x, WPA: 0x%08x, RSN: 0x%08x)",
                flags, wpaFlags, rsnFlags);
        if ((flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpaFlags == NM_802_11_AP_SEC_NONE)
            && (rsnFlags == NM_802_11_AP_SEC_NONE))
        {
            ffStrbufAppendS(&item->conn.security, "WEP/");
            FF_DEBUG("Adding security: WEP");
        }
        if (wpaFlags != NM_802_11_AP_SEC_NONE)
        {
            ffStrbufAppendS(&item->conn.security, "WPA/");
            FF_DEBUG("Adding security: WPA");
        }
        if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
            ffStrbufAppendS(&item->conn.security, "WPA2/");
            FF_DEBUG("Adding security: WPA2");
        }
        if (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_SAE) {
            ffStrbufAppendS(&item->conn.security, "WPA3/");
            FF_DEBUG("Adding security: WPA3");
        }
        if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)) {
            ffStrbufAppendS(&item->conn.security, "OWE/");
            FF_DEBUG("Adding security: OWE");
        }
        if ((wpaFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
            ffStrbufAppendS(&item->conn.security, "802.1X/");
            FF_DEBUG("Adding security: 802.1X");
        }
        if (!item->conn.security.length)
        {
            ffStrbufAppendS(&item->conn.security, "Insecure");
            FF_DEBUG("No security detected, marking as 'Insecure'");
        }
        else
        {
            ffStrbufTrimRight(&item->conn.security, '/');
            FF_DEBUG("Final security string: %s", item->conn.security.chars);
        }

        if (wpaFlags & NM_802_11_AP_SEC_PAIR_TKIP || rsnFlags & NM_802_11_AP_SEC_PAIR_TKIP) {
            FF_DEBUG("Detected TKIP encryption");
        }
        if (wpaFlags & NM_802_11_AP_SEC_PAIR_CCMP || rsnFlags & NM_802_11_AP_SEC_PAIR_CCMP) {
            FF_DEBUG("Detected CCMP/AES encryption");
        }
    }

    FF_DEBUG("NetworkManager wifi detection completed successfully");
    return NULL;
}
#endif // FF_HAVE_DBUS

static const char* detectWifiWithIw(FFWifiResult* item, FFstrbuf* buffer)
{
    FF_DEBUG("Starting iw wifi detection for interface %s", item->inf.description.chars);
    const char* error = NULL;
    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();
    FF_DEBUG("Executing 'iw dev %s link'", item->inf.description.chars);
    if((error = ffProcessAppendStdOut(&output, (char* const[]){
        "iw",
        "dev",
        item->inf.description.chars,
        "link",
        NULL
    })))
    {
        FF_DEBUG("iw command execution failed: %s", error);
        return error;
    }

    if(output.length == 0)
    {
        FF_DEBUG("iw command output is empty");
        return "iw command execution failed";
    }

    if(!ffParsePropLines(output.chars, "Connected to ", &item->conn.bssid))
    {
        FF_DEBUG("Not connected to any access point");
        ffStrbufAppendS(&item->conn.status, "disconnected");
        return NULL;
    }

    FF_DEBUG("Connected to an access point");
    ffStrbufAppendS(&item->conn.status, "connected");
    ffStrbufSubstrBeforeFirstC(&item->conn.bssid, ' ');
    ffStrbufUpperCase(&item->conn.bssid);
    FF_DEBUG("BSSID: %s", item->conn.bssid.chars);

    if(ffParsePropLines(output.chars, "SSID: ", &item->conn.ssid))
        FF_DEBUG("SSID: %s", item->conn.ssid.chars);
    else
        FF_DEBUG("SSID not found in iw output");

    ffStrbufClear(buffer);
    if(ffParsePropLines(output.chars, "signal: ", buffer))
    {
        int level = (int) ffStrbufToSInt(buffer, INT_MAX);
        if (level != INT_MAX)
        {
            item->conn.signalQuality = level >= -50 ? 100 : level <= -100 ? 0 : (level + 100) * 2;
            FF_DEBUG("Signal level: %d dBm, quality: %.0f%%", level, item->conn.signalQuality);
        }
    }

    ffStrbufClear(buffer);
    if(ffParsePropLines(output.chars, "rx bitrate: ", buffer))
    {
        item->conn.rxRate = ffStrbufToDouble(buffer, -DBL_MAX);
        FF_DEBUG("RX bitrate: %.2f Mbps", item->conn.rxRate);
    }

    ffStrbufClear(buffer);
    if(ffParsePropLines(output.chars, "tx bitrate: ", buffer))
    {
        item->conn.txRate = ffStrbufToDouble(buffer, -DBL_MAX);
        FF_DEBUG("TX bitrate: %.2f Mbps (raw: %s)", item->conn.txRate, buffer->chars);

        if(ffStrbufContainS(buffer, " EHT-MCS "))
        {
            ffStrbufSetStatic(&item->conn.protocol, "802.11be (Wi-Fi 7)");
            FF_DEBUG("Detected protocol: Wi-Fi 7");
        }
        else if(ffStrbufContainS(buffer, " HE-MCS "))
        {
            ffStrbufSetStatic(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
            FF_DEBUG("Detected protocol: Wi-Fi 6");
        }
        else if(ffStrbufContainS(buffer, " VHT-MCS "))
        {
            ffStrbufSetStatic(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
            FF_DEBUG("Detected protocol: Wi-Fi 5");
        }
        else if(ffStrbufContainS(buffer, " MCS "))
        {
            ffStrbufSetStatic(&item->conn.protocol, "802.11n (Wi-Fi 4)");
            FF_DEBUG("Detected protocol: Wi-Fi 4");
        }
    }

    ffStrbufClear(buffer);
    if(ffParsePropLines(output.chars, "freq: ", buffer))
    {
        item->conn.frequency = (uint16_t) ffStrbufToUInt(buffer, 0);
        item->conn.channel = ffWifiFreqToChannel(item->conn.frequency);
        FF_DEBUG("Frequency: %u MHz, Channel: %u", item->conn.frequency, item->conn.channel);
    }

    FF_DEBUG("iw wifi detection completed successfully");
    return NULL;
}

#if FF_HAVE_LINUX_WIRELESS
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/wireless.h>

static const char* detectWifiWithIoctls(FFWifiResult* item)
{
    FF_DEBUG("Starting ioctl wifi detection for interface %s", item->inf.description.chars);
    FF_AUTO_CLOSE_FD int sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if(sock < 0)
    {
        FF_DEBUG("Failed to create socket: %m");
        return "socket() failed";
    }

    struct iwreq iwr;
    ffStrCopy(iwr.ifr_name, item->inf.description.chars, IFNAMSIZ);

    // Get SSID
    FF_DEBUG("Getting SSID via ioctl");
    ffStrbufEnsureFree(&item->conn.ssid, IW_ESSID_MAX_SIZE);
    iwr.u.essid.pointer = (caddr_t) item->conn.ssid.chars;
    iwr.u.essid.length = IW_ESSID_MAX_SIZE + 1;
    iwr.u.essid.flags = 0;
    if(ioctl(sock, SIOCGIWESSID, &iwr) >= 0)
    {
        ffStrbufSetStatic(&item->conn.status, "connected");
        ffStrbufRecalculateLength(&item->conn.ssid);
        FF_DEBUG("SSID: %s", item->conn.ssid.chars);
    }
    else
        FF_DEBUG("Failed to get SSID via ioctl: %m");

    // Get protocol name
    FF_DEBUG("Getting protocol name via ioctl");
    if(ioctl(sock, SIOCGIWNAME, &iwr) >= 0 && !ffStrEqualsIgnCase(iwr.u.name, "IEEE 802.11"))
    {
        if(ffStrStartsWithIgnCase(iwr.u.name, "IEEE "))
            ffStrbufSetS(&item->conn.protocol, iwr.u.name + strlen("IEEE "));
        else
            ffStrbufSetS(&item->conn.protocol, iwr.u.name);
        FF_DEBUG("Protocol: %s", item->conn.protocol.chars);
    }
    else
        FF_DEBUG("Failed to get protocol name via ioctl: %m");

    // Get BSSID
    FF_DEBUG("Getting BSSID via ioctl");
    if(ioctl(sock, SIOCGIWAP, &iwr) >= 0)
    {
        for(int i = 0; i < 6; ++i)
            ffStrbufAppendF(&item->conn.bssid, "%.2X:", (uint8_t) iwr.u.ap_addr.sa_data[i]);
        ffStrbufTrimRight(&item->conn.bssid, ':');
        FF_DEBUG("BSSID: %s", item->conn.bssid.chars);
    }
    else
        FF_DEBUG("Failed to get BSSID via ioctl: %m");

    // Get bitrate
    FF_DEBUG("Getting bitrate via ioctl");
    if(ioctl(sock, SIOCGIWRATE, &iwr) >= 0)
    {
        item->conn.txRate = iwr.u.bitrate.value / 1000000.;
        FF_DEBUG("TX bitrate: %.2f Mbps", item->conn.txRate);
    }
    else
        FF_DEBUG("Failed to get bitrate via ioctl: %m");

    // Get frequency/channel
    FF_DEBUG("Getting frequency via ioctl");
    if(ioctl(sock, SIOCGIWFREQ, &iwr) >= 0)
    {
        if (iwr.u.freq.e == 0 && iwr.u.freq.m <= 1000)
        {
            item->conn.channel = (uint16_t) iwr.u.freq.m;
            FF_DEBUG("Direct channel value: %u", item->conn.channel);
        }
        else
        {
            // convert it to MHz
            while (iwr.u.freq.e < 6)
            {
                iwr.u.freq.m /= 10;
                iwr.u.freq.e++;
            }
            while (iwr.u.freq.e > 6)
            {
                iwr.u.freq.m *= 10;
                iwr.u.freq.e--;
            }
            item->conn.frequency = (uint16_t) iwr.u.freq.m;
            item->conn.channel = ffWifiFreqToChannel(item->conn.frequency);
            FF_DEBUG("Frequency: %u MHz, Channel: %u", item->conn.frequency, item->conn.channel);
        }
    }
    else
        FF_DEBUG("Failed to get frequency via ioctl: %m");

    // Get signal strength
    FF_DEBUG("Getting signal stats via ioctl");
    struct iw_statistics stats;
    iwr.u.data.pointer = &stats;
    iwr.u.data.length = sizeof(stats);
    iwr.u.data.flags = 0;

    if(ioctl(sock, SIOCGIWSTATS, &iwr) >= 0)
    {
        int8_t level = (int8_t) stats.qual.level;
        item->conn.signalQuality = level >= -50 ? 100 : level <= -100 ? 0 : (level + 100) * 2;
        FF_DEBUG("Signal level: %d dBm, quality: %.0f%%", level, item->conn.signalQuality);
    }
    else
        FF_DEBUG("Failed to get signal stats via ioctl: %m");

    // Get security info
    FF_DEBUG("Getting security info via ioctl");
    struct iw_encode_ext iwe;
    iwr.u.data.pointer = &iwe;
    iwr.u.data.length = sizeof(iwe);
    iwr.u.data.flags = 0;
    if(ioctl(sock, SIOCGIWENCODEEXT, &iwr) >= 0)
    {
        switch(iwe.alg)
        {
            case IW_ENCODE_ALG_WEP:
                ffStrbufAppendS(&item->conn.security, "WEP");
                FF_DEBUG("Security: WEP");
                break;
            case IW_ENCODE_ALG_TKIP:
                ffStrbufAppendS(&item->conn.security, "TKIP");
                FF_DEBUG("Security: TKIP");
                break;
            case IW_ENCODE_ALG_CCMP:
                ffStrbufAppendS(&item->conn.security, "CCMP");
                FF_DEBUG("Security: CCMP");
                break;
            case IW_ENCODE_ALG_PMK:
                ffStrbufAppendS(&item->conn.security, "PMK");
                FF_DEBUG("Security: PMK");
                break;
            case IW_ENCODE_ALG_AES_CMAC:
                ffStrbufAppendS(&item->conn.security, "CMAC");
                FF_DEBUG("Security: CMAC");
                break;
            default:
                ffStrbufAppendF(&item->conn.security, "Unknown (%d)", (int) iwe.alg);
                FF_DEBUG("Security: Unknown (%d)", (int) iwe.alg);
                break;
        }
    }
    else
        FF_DEBUG("Failed to get security info via ioctl: %m");

    FF_DEBUG("ioctl wifi detection completed");
    return NULL;
}
#endif // FF_HAVE_LINUX_WIRELESS

const char* ffDetectWifi(FF_MAYBE_UNUSED FFlist* result)
{
    FF_DEBUG("Starting wifi detection");
    struct if_nameindex* infs = if_nameindex();
    if(!infs)
    {
        FF_DEBUG("if_nameindex() failed: %m");
        return "if_nameindex() failed";
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        FF_DEBUG("Checking interface: %s (index: %u)", i->if_name, i->if_index);
        ffStrbufSetF(&buffer, "/sys/class/net/%s/phy80211/", i->if_name);
        if(!ffPathExists(buffer.chars, FF_PATHTYPE_DIRECTORY))
        {
            FF_DEBUG("Not a wifi interface (no phy80211 directory)");
            continue;
        }

        FF_DEBUG("Found wifi interface: %s", i->if_name);
        FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
        ffStrbufInitS(&item->inf.description, i->if_name);
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
        item->conn.frequency = 0;

        char operstate;
        ffStrbufSetF(&buffer, "/sys/class/net/%s/operstate", i->if_name);
        if (!ffReadFileData(buffer.chars, 1, &operstate))
        {
            FF_DEBUG("Failed to read operstate file");
            continue;
        }

        FF_DEBUG("Connection status: %c", operstate);
        if (operstate != 'u')
        {
            FF_DEBUG("Skipping interface as it's not up");
            ffStrbufSetStatic(&item->conn.status, "disconnected");

            ffStrbufSetF(&buffer, "/sys/class/net/%s/flags", i->if_name);
            char flags[16];
            ssize_t len = ffReadFileData(buffer.chars, sizeof(flags), flags);
            if (len <= 0)
            {
                FF_DEBUG("Failed to read flags file");
                ffStrbufSetStatic(&item->inf.status, "unknown");
                continue;
            }
            flags[len] = '\0';
            FF_DEBUG("Interface flags: %s", flags);
            unsigned flagsVal = (unsigned) strtoul(flags, NULL, 16);
            if (flagsVal & IFF_UP)
            {
                ffStrbufSetStatic(&item->inf.status, "up");
                FF_DEBUG("Interface is up but not connected");
            }
            else
            {
                ffStrbufSetStatic(&item->inf.status, "down");
                FF_DEBUG("Interface is down");
            }

            continue;
        }

        ffStrbufSetStatic(&item->inf.status, "up");

        FF_DEBUG("Trying to detect wifi with iw");
        if (detectWifiWithIw(item, &buffer) != NULL)
        {
            FF_DEBUG("iw detection failed, trying fallback methods");
            #ifdef FF_HAVE_LINUX_WIRELESS
                FF_DEBUG("Trying to detect wifi with ioctls");
                detectWifiWithIoctls(item);
            #endif
        }

        #ifdef FF_HAVE_DBUS
            FF_DEBUG("Enhancing wifi info with NetworkManager");
            detectWifiWithNm(item, &buffer);
        #endif
    }
    if_freenameindex(infs);

    FF_DEBUG("Wifi detection completed, found %u wifi interfaces", result->length);
    return NULL;
}
