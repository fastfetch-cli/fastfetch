#include "wifi.h"
#include "common/dbus.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "util/stringUtils.h"

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
    FFDBusData dbus;
    const char* error = ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus);
    if(error)
        return error;

    {
        DBusMessage* device = ffDBusGetMethodReply(&dbus, "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "GetDeviceByIpIface", item->inf.description.chars);
        if(!device)
            return "Failed to call GetDeviceByIpIface";

        ffStrbufClear(buffer);
        DBusMessageIter rootIter;
        if(!dbus.lib->ffdbus_message_iter_init(device, &rootIter) || !ffDBusGetString(&dbus, &rootIter, buffer))
        {
            dbus.lib->ffdbus_message_unref(device);
            return "Failed to get device path";
        }
        dbus.lib->ffdbus_message_unref(device);
    }

    if (item->conn.txRate != item->conn.txRate)
    {
        uint32_t bitrate;
        if (ffDBusGetPropertyUint(&dbus, "org.freedesktop.NetworkManager", buffer->chars, "org.freedesktop.NetworkManager.Device.Wireless", "Bitrate", &bitrate))
            item->conn.txRate = bitrate / 1000.;
    }

    FF_STRBUF_AUTO_DESTROY apPath = ffStrbufCreate();
    if (!ffDBusGetPropertyString(&dbus, "org.freedesktop.NetworkManager", buffer->chars, "org.freedesktop.NetworkManager.Device.Wireless", "ActiveAccessPoint", &apPath))
        return "Failed to get active access point path";

    if (!item->conn.status.length)
        ffStrbufSetStatic(&item->conn.status, "connected");

    DBusMessage* reply = ffDBusGetAllProperties(&dbus, "org.freedesktop.NetworkManager", apPath.chars, "org.freedesktop.NetworkManager.AccessPoint");
    if(reply == NULL)
        return "Failed to get access point properties";

    DBusMessageIter rootIterator;
    if(!dbus.lib->ffdbus_message_iter_init(reply, &rootIterator) &&
        dbus.lib->ffdbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_ARRAY)
    {
        dbus.lib->ffdbus_message_unref(reply);
        return "Invalid type of access point properties";
    }

    DBusMessageIter arrayIterator;
    dbus.lib->ffdbus_message_iter_recurse(&rootIterator, &arrayIterator);

    NM80211ApFlags flags;
    NM80211ApSecurityFlags wpaFlags, rsnFlags;
    int flagCount = 0;

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
                ffDBusGetString(&dbus, &dictIterator, &item->conn.ssid);
        }
        else if (ffStrEquals(key, "HwAddress"))
        {
            if (!item->conn.bssid.length)
                ffDBusGetString(&dbus, &dictIterator, &item->conn.bssid);
        }
        else if (ffStrEquals(key, "Strength"))
        {
            if (item->conn.signalQuality != item->conn.signalQuality)
            {
                uint32_t strengthPercent;
                if (ffDBusGetUint(&dbus, &dictIterator, &strengthPercent))
                    item->conn.signalQuality = strengthPercent;
            }
        }
        else if (ffStrEquals(key, "Frequency"))
        {
            if (item->conn.frequency == 0)
            {
                uint32_t frequency;
                if (ffDBusGetUint(&dbus, &dictIterator, &frequency))
                {
                    item->conn.frequency = (uint16_t) frequency;
                    if (item->conn.channel == 0)
                        item->conn.channel = ffWifiFreqToChannel(item->conn.frequency);
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
        if ((flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpaFlags == NM_802_11_AP_SEC_NONE)
            && (rsnFlags == NM_802_11_AP_SEC_NONE))
            ffStrbufAppendS(&item->conn.security, "WEP/");
        if (wpaFlags != NM_802_11_AP_SEC_NONE)
            ffStrbufAppendS(&item->conn.security, "WPA/");
        if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
            ffStrbufAppendS(&item->conn.security, "WPA2/");
        }
        if (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_SAE) {
            ffStrbufAppendS(&item->conn.security, "WPA3/");
        }
        if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)) {
            ffStrbufAppendS(&item->conn.security, "OWE/");
        }
        if ((wpaFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
            ffStrbufAppendS(&item->conn.security, "802.1X/");
        }
        if (!item->conn.security.length)
            ffStrbufAppendS(&item->conn.security, "Insecure");
        else
            ffStrbufTrimRight(&item->conn.security, '/');
    }

    return NULL;
}
#endif // FF_HAVE_DBUS

static const char* detectWifiWithIw(FFWifiResult* item, FFstrbuf* buffer)
{
    const char* error = NULL;
    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();
    if((error = ffProcessAppendStdOut(&output, (char* const[]){
        "iw",
        "dev",
        item->inf.description.chars,
        "link",
        NULL
    })))
        return error;

    if(output.length == 0)
        return "iw command execution failed";

    if(!ffParsePropLines(output.chars, "Connected to ", &item->conn.bssid))
    {
        ffStrbufAppendS(&item->conn.status, "disconnected");
        return NULL;
    }

    ffStrbufAppendS(&item->conn.status, "connected");
    ffStrbufSubstrBeforeFirstC(&item->conn.bssid, ' ');
    ffStrbufUpperCase(&item->conn.bssid);

    ffParsePropLines(output.chars, "SSID: ", &item->conn.ssid);

    ffStrbufClear(buffer);
    if(ffParsePropLines(output.chars, "signal: ", buffer))
    {
        int level = (int) ffStrbufToSInt(buffer, INT_MAX);
        if (level != INT_MAX)
            item->conn.signalQuality = level >= -50 ? 100 : level <= -100 ? 0 : (level + 100) * 2;
    }

    ffStrbufClear(buffer);
    if(ffParsePropLines(output.chars, "rx bitrate: ", buffer))
        item->conn.rxRate = ffStrbufToDouble(buffer);

    ffStrbufClear(buffer);
    if(ffParsePropLines(output.chars, "tx bitrate: ", buffer))
    {
        item->conn.txRate = ffStrbufToDouble(buffer);

        if(ffStrbufContainS(buffer, " EHT-MCS "))
            ffStrbufSetStatic(&item->conn.protocol, "802.11be (Wi-Fi 7)");
        else if(ffStrbufContainS(buffer, " HE-MCS "))
            ffStrbufSetStatic(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
        else if(ffStrbufContainS(buffer, " VHT-MCS "))
            ffStrbufSetStatic(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
        else if(ffStrbufContainS(buffer, " MCS "))
            ffStrbufSetStatic(&item->conn.protocol, "802.11n (Wi-Fi 4)");
    }

    ffStrbufClear(buffer);
    if(ffParsePropLines(output.chars, "freq: ", buffer))
    {
        item->conn.frequency = (uint16_t) ffStrbufToUInt(buffer, 0);
        item->conn.channel = ffWifiFreqToChannel(item->conn.frequency);
    }

    return NULL;
}

#if FF_HAVE_LINUX_WIRELESS
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/wireless.h>

static const char* detectWifiWithIoctls(FFWifiResult* item)
{
    FF_AUTO_CLOSE_FD int sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if(sock < 0)
        return "socket() failed";

    struct iwreq iwr;
    ffStrCopy(iwr.ifr_name, item->inf.description.chars, IFNAMSIZ);
    ffStrbufEnsureFree(&item->conn.ssid, IW_ESSID_MAX_SIZE);
    iwr.u.essid.pointer = (caddr_t) item->conn.ssid.chars;
    iwr.u.essid.length = IW_ESSID_MAX_SIZE + 1;
    iwr.u.essid.flags = 0;
    if(ioctl(sock, SIOCGIWESSID, &iwr) >= 0)
    {
        ffStrbufSetStatic(&item->conn.status, "connected");
        ffStrbufRecalculateLength(&item->conn.ssid);
    }

    if(ioctl(sock, SIOCGIWNAME, &iwr) >= 0 && !ffStrEqualsIgnCase(iwr.u.name, "IEEE 802.11"))
    {
        if(ffStrStartsWithIgnCase(iwr.u.name, "IEEE "))
            ffStrbufSetS(&item->conn.protocol, iwr.u.name + strlen("IEEE "));
        else
            ffStrbufSetS(&item->conn.protocol, iwr.u.name);
    }

    if(ioctl(sock, SIOCGIWAP, &iwr) >= 0)
    {
        for(int i = 0; i < 6; ++i)
            ffStrbufAppendF(&item->conn.bssid, "%.2X:", (uint8_t) iwr.u.ap_addr.sa_data[i]);
        ffStrbufTrimRight(&item->conn.bssid, ':');
    }

    if(ioctl(sock, SIOCGIWRATE, &iwr) >= 0)
        item->conn.txRate = iwr.u.bitrate.value / 1000000.;

    if(ioctl(sock, SIOCGIWFREQ, &iwr) >= 0)
    {
        if (iwr.u.freq.e == 0 && iwr.u.freq.m <= 1000)
        {
            item->conn.channel = (uint16_t) iwr.u.freq.m;
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
        }
    }

    struct iw_statistics stats;
    iwr.u.data.pointer = &stats;
    iwr.u.data.length = sizeof(stats);
    iwr.u.data.flags = 0;

    if(ioctl(sock, SIOCGIWSTATS, &iwr) >= 0)
    {
        int8_t level = (int8_t) stats.qual.level; // https://stackoverflow.com/questions/18079771/wireless-h-how-do-i-print-out-the-signal-level
        item->conn.signalQuality = level >= -50 ? 100 : level <= -100 ? 0 : (level + 100) * 2;
    }

    //FIXME: doesn't work
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
                break;
            case IW_ENCODE_ALG_TKIP:
                ffStrbufAppendS(&item->conn.security, "TKIP");
                break;
            case IW_ENCODE_ALG_CCMP:
                ffStrbufAppendS(&item->conn.security, "CCMP");
                break;
            case IW_ENCODE_ALG_PMK:
                ffStrbufAppendS(&item->conn.security, "PMK");
                break;
            case IW_ENCODE_ALG_AES_CMAC:
                ffStrbufAppendS(&item->conn.security, "CMAC");
                break;
            default:
                ffStrbufAppendF(&item->conn.security, "Unknown (%d)", (int) iwe.alg);
                break;
        }
    }

    return NULL;
}
#endif // FF_HAVE_LINUX_WIRELESS

const char* ffDetectWifi(FF_MAYBE_UNUSED FFlist* result)
{
    struct if_nameindex* infs = if_nameindex();
    if(!infs)
        return "if_nameindex() failed";

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        ffStrbufSetF(&buffer, "/sys/class/net/%s/phy80211/", i->if_name);
        if(!ffPathExists(buffer.chars, FF_PATHTYPE_DIRECTORY))
            continue;

        FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
        ffStrbufInitS(&item->inf.description, i->if_name);
        ffStrbufInit(&item->inf.status);
        ffStrbufInit(&item->conn.status);
        ffStrbufInit(&item->conn.ssid);
        ffStrbufInit(&item->conn.bssid);
        ffStrbufInit(&item->conn.protocol);
        ffStrbufInit(&item->conn.security);
        item->conn.signalQuality = 0.0/0.0;
        item->conn.rxRate = 0.0/0.0;
        item->conn.txRate = 0.0/0.0;
        item->conn.channel = 0;
        item->conn.frequency = 0;

        ffStrbufSetF(&buffer, "/sys/class/net/%s/operstate", i->if_name);
        if (!ffAppendFileBuffer(buffer.chars, &item->inf.status))
            continue;

        ffStrbufTrimRightSpace(&item->inf.status);
        if (!ffStrbufEqualS(&item->inf.status, "up"))
            continue;

        if (detectWifiWithIw(item, &buffer) != NULL)
        {
            #ifdef FF_HAVE_LINUX_WIRELESS
                detectWifiWithIoctls(item);
            #endif
        }

        #ifdef FF_HAVE_DBUS
            detectWifiWithNm(item, &buffer);
        #endif
    }
    if_freenameindex(infs);

    return NULL;
}
