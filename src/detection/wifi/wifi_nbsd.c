#include "wifi.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#define COMPAT_FREEBSD_NET80211 1
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_media.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>

// ieee80211 header of NetBSD is full of mess. Add compatibility macros from FreeBSD
#undef IEEE80211_IS_CHAN_ANYG
#define IEEE80211_IS_CHAN_ANYG(x) (IEEE80211_IS_CHAN_PUREG(x) || IEEE80211_IS_CHAN_G(x))
#undef IEEE80211_IS_CHAN_HT
#define IEEE80211_IS_CHAN_HT(x) (((x)->ic_flags & IEEE80211_CHAN_HT) != 0)
#undef IEEE80211_IS_CHAN_VHT
#define IEEE80211_IS_CHAN_VHT(x) (((x)->ic_flags & IEEE80211_CHAN_VHT) != 0)

const char* ffDetectWifi(FFlist* result)
{
    struct if_nameindex* infs = if_nameindex();
    if(!infs) {
        return "if_nameindex() failed";
    }

    FF_AUTO_CLOSE_FD int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        if_freenameindex(infs);
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
        item->conn.signalQuality = 0.0/0.0;
        item->conn.rxRate = 0.0/0.0;
        item->conn.txRate = 0.0/0.0;
        item->conn.channel = 0;
        item->conn.frequency = 0;

        char ssid[IEEE80211_NWID_LEN + 1] = {};
        struct ieee80211req ireq = {};
        strlcpy(ireq.i_name, i->if_name, sizeof(ireq.i_name));
        ireq.i_type = IEEE80211_IOC_SSID;
        ireq.i_data = ssid;
        ireq.i_len = sizeof(ssid) - 1;

        if (ioctl(sock, SIOCG80211, &ireq) < 0 || ireq.i_len == 0) {
            struct ifreq ifr;
            strlcpy(ifr.ifr_name, i->if_name, sizeof(ifr.ifr_name));
            if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
                ffStrbufSetStatic(&item->inf.status, "Unknown");
            } else {
                ffStrbufSetStatic(&item->inf.status, ifr.ifr_flags & IFF_UP ? "Up" : "Down");
            }
            ffStrbufAppendS(&item->conn.status, "Not associated");
            continue;
        }

        ffStrbufSetStatic(&item->inf.status, "Up");
        ffStrbufSetStatic(&item->conn.status, "Associated");
        ffStrbufAppendNS(&item->conn.ssid, ireq.i_len, ssid);

        uint8_t bssid[IEEE80211_ADDR_LEN] = {};
        ireq.i_type = IEEE80211_IOC_BSSID;
        ireq.i_data = bssid;
        ireq.i_len = sizeof(bssid);

        if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
            ffStrbufSetF(&item->conn.bssid, "%02X:%02X:%02X:%02X:%02X:%02X",
                         bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
        }

        struct ieee80211_channel curchan = {};
        ireq.i_type = IEEE80211_IOC_CHANNEL;
        ireq.i_data = &curchan;
        ireq.i_len = sizeof(curchan);

        if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
            // item->conn.channel = curchan.ic_ieee; // No ic_ieee in NetBSD
            item->conn.channel = ffWifiFreqToChannel(curchan.ic_freq);
            item->conn.frequency = curchan.ic_freq;

            if (IEEE80211_IS_CHAN_FHSS(&curchan))
                ffStrbufSetStatic(&item->conn.protocol, "802.11 (FHSS)");
            if (IEEE80211_IS_CHAN_A(&curchan))
                ffStrbufSetStatic(&item->conn.protocol, "802.11a");
            if (IEEE80211_IS_CHAN_B(&curchan))
                ffStrbufSetStatic(&item->conn.protocol, "802.11b");
            if (IEEE80211_IS_CHAN_ANYG(&curchan))
                ffStrbufSetStatic(&item->conn.protocol, "802.11g");

            if (IEEE80211_IS_CHAN_HT(&curchan))
                ffStrbufSetStatic(&item->conn.protocol, "802.11n (Wi-Fi 4)");
            if (IEEE80211_IS_CHAN_VHT(&curchan))
                ffStrbufSetStatic(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
            #ifdef IEEE80211_IS_CHAN_HE
            if (IEEE80211_IS_CHAN_HE(&curchan))
                ffStrbufSetStatic(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
            #endif
        }

        union {
            struct ieee80211req_sta_req req;
            uint8_t buf[1024];
        } stareq = {};
        memcpy(stareq.req.is_u.macaddr, bssid, sizeof(bssid));
        ireq.i_type = IEEE80211_IOC_STA_INFO;
        ireq.i_data = &stareq;
        ireq.i_len = sizeof(stareq);

        if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
            struct ieee80211req_sta_info* sta = stareq.req.info;
            if (sta->isi_len != 0) {
                int8_t rssi = (int8_t) sta->isi_rssi; // This is strange
                item->conn.signalQuality = (rssi >= -50 ? 100 : rssi <= -100 ? 0 : (rssi + 100) * 2);

                if (sta->isi_txrate) {
                    item->conn.txRate = (double)sta->isi_txrate / 2.0;
                }
            }
        }

        ireq.i_type = IEEE80211_IOC_AUTHMODE;
        ireq.i_data = NULL;
        ireq.i_len = 0;
        if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
            switch (ireq.i_val) {
            case IEEE80211_AUTH_NONE:
                ffStrbufSetStatic(&item->conn.security, "Insecure");
                break;
            case IEEE80211_AUTH_OPEN:
                ffStrbufSetStatic(&item->conn.security, "Open");
                break;
            case IEEE80211_AUTH_SHARED:
                ffStrbufSetStatic(&item->conn.security, "Shared");
                break;
            case IEEE80211_AUTH_8021X:
                ffStrbufSetStatic(&item->conn.security, "8021X");
                break;
            case IEEE80211_AUTH_AUTO:
                ffStrbufSetStatic(&item->conn.security, "Auto");
                break;
            case IEEE80211_AUTH_WPA:
                ffStrbufSetStatic(&item->conn.security, "WPA");
                break;
            default:
                ffStrbufSetF(&item->conn.security, "Unknown (%d)", ireq.i_val);
                break;
            }
        }
    }

    if_freenameindex(infs);
    return NULL;
}
