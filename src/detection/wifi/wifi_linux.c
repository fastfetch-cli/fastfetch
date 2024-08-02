#include "wifi.h"
#include "common/processing.h"
#include "common/properties.h"
#include "util/stringUtils.h"

#include <stdio.h>
#include <stdlib.h>

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

        if(ffStrbufContainS(buffer, " HE-MCS "))
            ffStrbufSetStatic(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
        else if(ffStrbufContainS(buffer, " VHT-MCS "))
            ffStrbufSetStatic(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
        else if(ffStrbufContainS(buffer, " MCS "))
            ffStrbufSetStatic(&item->conn.protocol, "802.11n (Wi-Fi 4)");
    }

    return NULL;
}

#if FF_HAVE_LINUX_WIRELESS
#define FF_DETECT_WIFI_WITH_IOCTLS

#include "common/io/io.h"

#include <net/if.h>
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
    strncpy(iwr.ifr_name, item->inf.description.chars, IFNAMSIZ);
    ffStrbufEnsureFree(&item->conn.ssid, IW_ESSID_MAX_SIZE);
    iwr.u.essid.pointer = (caddr_t) item->conn.ssid.chars;
    iwr.u.essid.length = IW_ESSID_MAX_SIZE + 1;
    iwr.u.essid.flags = 0;
    if(ioctl(sock, SIOCGIWESSID, &iwr) >= 0)
        ffStrbufRecalculateLength(&item->conn.ssid);

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
            ffStrbufAppendF(&item->conn.bssid, "%.2X-", (uint8_t) iwr.u.ap_addr.sa_data[i]);
        ffStrbufTrimRight(&item->conn.bssid, '-');
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

#endif

const char* ffDetectWifi(FF_MAYBE_UNUSED FFlist* result)
{
    struct if_nameindex* infs = if_nameindex();
    if(!infs)
        return "if_nameindex() failed";

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    const char* error = NULL;

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        ffStrbufSetF(&buffer, "/sys/class/net/%s/phy80211", i->if_name);
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

        ffStrbufSetF(&buffer, "/sys/class/net/%s/operstate", i->if_name);
        if (!ffAppendFileBuffer(buffer.chars, &item->inf.status))
            continue;

        ffStrbufTrimRightSpace(&item->inf.status);
        if (!ffStrbufEqualS(&item->inf.status, "up"))
            continue;

        if (detectWifiWithIw(item, &buffer) == NULL)
            continue;

        #ifdef FF_DETECT_WIFI_WITH_IOCTLS
            detectWifiWithIoctls(item);
            continue;
        #endif

        error = "`iw` failed and `linux/wireless.h` not found during compilation";
        break;
    }
    if_freenameindex(infs);

    return error;
}
