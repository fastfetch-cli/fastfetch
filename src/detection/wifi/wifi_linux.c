#include "wifi.h"

#include "common/io.h"

#include <net/if.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <unistd.h>

static const char* detectInf(char* ifName, FFWifiResult* wifi)
{
    int sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if(sock < 0)
        return "socket() failed";

    struct iwreq iwr;

    strncpy(iwr.ifr_name, ifName, IFNAMSIZ);
    ffStrbufEnsureFree(&wifi->conn.ssid, IW_ESSID_MAX_SIZE);
    iwr.u.essid.pointer = (caddr_t) wifi->conn.ssid.chars;
    iwr.u.essid.length = IW_ESSID_MAX_SIZE + 1;
    iwr.u.essid.flags = 0;
    if(ioctl(sock, SIOCGIWESSID, &iwr) >= 0)
        ffStrbufRecalculateLength(&wifi->conn.ssid);

    if(ioctl(sock, SIOCGIWNAME, &iwr) >= 0)
    {
        if(strncasecmp(iwr.u.name, "IEEE ", 5) == 0)
            ffStrbufSetS(&wifi->conn.phyType, iwr.u.name + 5);
        else
            ffStrbufSetS(&wifi->conn.phyType, iwr.u.name);
    }

    if(ioctl(sock, SIOCGIWAP, &iwr) >= 0)
    {
        for(int i = 0; i < 6; ++i)
            ffStrbufAppendF(&wifi->conn.macAddress, "%.2X-", (uint8_t) iwr.u.ap_addr.sa_data[i]);
        ffStrbufTrimRight(&wifi->conn.macAddress, '-');
    }

    //FIXME: doesn't work
    if(ioctl(sock, SIOCGIWSPY, &iwr) >= 0)
        wifi->conn.signalQuality = iwr.u.qual.level;

    //FIXME: doesn't work
    struct iw_encode_ext iwe;
    iwr.u.data.pointer = &iwe;
    iwr.u.data.length = sizeof(iwe);
    iwr.u.data.flags = 0;
    if(ioctl(sock, SIOCGIWENCODEEXT, &iwr) >= 0)
    {
        struct iw_encode_ext* iwe = iwr.u.data.pointer;
        if(iwe->alg == IW_ENCODE_ALG_NONE)
            wifi->security.type = FF_WIFI_SECURITY_DISABLED;
        else
        {
            wifi->security.type = FF_WIFI_SECURITY_ENABLED;
            switch(iwe->alg)
            {
                case IW_ENCODE_ALG_WEP:
                    ffStrbufAppendS(&wifi->security.algorithm, "WEP");
                    break;
                case IW_ENCODE_ALG_TKIP:
                    ffStrbufAppendS(&wifi->security.algorithm, "TKIP");
                    break;
                case IW_ENCODE_ALG_CCMP:
                    ffStrbufAppendS(&wifi->security.algorithm, "CCMP");
                    break;
                case IW_ENCODE_ALG_PMK:
                    ffStrbufAppendS(&wifi->security.algorithm, "PMK");
                    break;
                case IW_ENCODE_ALG_AES_CMAC:
                    ffStrbufAppendS(&wifi->security.algorithm, "CMAC");
                    break;
            }
        }
    }

    close(sock);

    return NULL;
}

const char* ffDetectWifi(const FFinstance* instance, FFlist* result)
{
    FF_UNUSED(instance);

    struct if_nameindex* infs = if_nameindex();
    if(!infs)
        return "if_nameindex() failed";

    FFstrbuf path;
    ffStrbufInit(&path);

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        ffStrbufSetF(&path, "/sys/class/net/%s/phy80211", i->if_name);
        if(!ffFileExists(path.chars, S_IFDIR))
            continue;

        FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
        ffStrbufInitS(&item->inf.description, i->if_name);
        ffStrbufInit(&item->inf.status);
        ffStrbufInit(&item->conn.status);
        ffStrbufInit(&item->conn.ssid);
        ffStrbufInit(&item->conn.macAddress);
        ffStrbufInit(&item->conn.phyType);
        item->conn.signalQuality = 0.0/0.0;
        item->conn.rxRate = 0.0/0.0;
        item->conn.txRate = 0.0/0.0;
        item->security.type = FF_WIFI_SECURITY_UNKNOWN;
        item->security.oneXEnabled = false;
        ffStrbufInit(&item->security.algorithm);

        ffStrbufSetF(&path, "/sys/class/net/%s/operstate", i->if_name);
        if(!ffAppendFileBuffer(path.chars, &item->inf.status) || !ffStrbufEqualS(&item->inf.status, "up"))
            continue;

        detectInf(i->if_name, item);
    }
    if_freenameindex(infs);
    ffStrbufDestroy(&path);

    if(result->length == 0)
        return "No wifi interfaces found";

    return NULL;
}
