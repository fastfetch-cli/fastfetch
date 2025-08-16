#include "wifi.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>
#include <unistd.h>

const char* ffDetectWifi(FFlist* result)
{
    struct if_nameindex* infs = if_nameindex();
    if(!infs) {
        return "if_nameindex() failed";
    }

    FF_AUTO_CLOSE_FD int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        return "socket() failed";
    }

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        if (!ffStrStartsWith(i->if_name, "iwm")) {
            continue;
        }

        FFWifiResult* item = (FFWifiResult*) ffListAdd(result);
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

        struct ieee80211_nodereq nr = {};
        strlcpy(nr.nr_ifname, i->if_name, sizeof(nr.nr_ifname));

        struct ifreq ifr = {};
        strlcpy(ifr.ifr_name, i->if_name, sizeof(ifr.ifr_name));
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
            ffStrbufSetStatic(&item->inf.status, "Unknown");
        } else {
            ffStrbufSetStatic(&item->inf.status, ifr.ifr_flags & IFF_UP ? "Up" : "Down");
        }

        if (ioctl(sock, SIOCG80211NODE, &nr) < 0) {
            ffStrbufSetStatic(&item->conn.status, "Not associated");
            continue;
        }

        if (nr.nr_nwid_len > 0) {
            ffStrbufSetStatic(&item->conn.status, "Associated");
            ffStrbufAppendNS(&item->conn.ssid, nr.nr_nwid_len, (char*)nr.nr_nwid);
        } else {
            ffStrbufSetStatic(&item->conn.status, "Not associated");
            continue;
        }

        ffStrbufSetF(&item->conn.bssid, "%02X:%02X:%02X:%02X:%02X:%02X",
                    nr.nr_bssid[0], nr.nr_bssid[1], nr.nr_bssid[2],
                    nr.nr_bssid[3], nr.nr_bssid[4], nr.nr_bssid[5]);

        item->conn.channel = nr.nr_channel;

        if (nr.nr_max_rssi) {
            item->conn.signalQuality = ((float)nr.nr_rssi / nr.nr_max_rssi) * 100.0;
        }

        if (nr.nr_flags & IEEE80211_NODEREQ_HT) {
            ffStrbufSetStatic(&item->conn.protocol, "802.11n (Wi-Fi 4)");
        } else if (nr.nr_flags & IEEE80211_NODEREQ_VHT) {
            ffStrbufSetStatic(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
        } else if (nr.nr_chan_flags & IEEE80211_CHANINFO_5GHZ) {
            ffStrbufSetStatic(&item->conn.protocol, "802.11a");
        } else if (nr.nr_chan_flags & IEEE80211_CHANINFO_2GHZ) {
            ffStrbufSetStatic(&item->conn.protocol, "802.11g");
        }

        struct ieee80211_wpaparams wpa = {};
        strlcpy(wpa.i_name, i->if_name, sizeof(wpa.i_name));

        if (ioctl(sock, SIOCG80211WPAPARMS, &wpa) >= 0 && wpa.i_enabled) {
            if (wpa.i_protos & IEEE80211_WPA_PROTO_WPA2)
                ffStrbufSetStatic(&item->conn.security, "WPA2");
            else if (wpa.i_protos & IEEE80211_WPA_PROTO_WPA1)
                ffStrbufSetStatic(&item->conn.security, "WPA");
        } else {
            struct ieee80211_nwkey nwkey = {};
            strlcpy(nwkey.i_name, i->if_name, sizeof(nwkey.i_name));

            if (ioctl(sock, SIOCG80211NWKEY, &nwkey) >= 0) {
                if (nwkey.i_wepon)
                    ffStrbufSetStatic(&item->conn.security, "WEP");
                else
                    ffStrbufSetStatic(&item->conn.security, "Open");
            }
        }
    }

    if_freenameindex(infs);
    return NULL;
}
