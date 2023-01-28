#include "wifi.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef FF_HAVE_LIBNM

#include "common/processing.h"
#include "common/properties.h"
#include "common/library.h"

#include <glib.h>
#define NM_NO_INCLUDE_EXTRA_HEADERS 1
#include <NetworkManager.h>

static const char* detectWifiWithLibnm(const FFinstance* instance, FFlist* result)
{
    FF_LIBRARY_LOAD(nm, &instance->config.libnm, "dlopen libnm failed", "libnm" FF_LIBRARY_EXTENSION, 0);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_client_new);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_client_get_devices);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_device_get_iface);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_device_get_state);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_device_wifi_get_type);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_device_wifi_get_active_access_point);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_access_point_get_ssid);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_utils_ssid_to_utf8);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_access_point_get_bssid);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_access_point_get_strength);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_access_point_get_max_bitrate);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_access_point_get_flags);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_access_point_get_wpa_flags);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, nm_access_point_get_rsn_flags);

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, g_type_check_instance_is_a);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, g_bytes_get_size);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, g_bytes_get_data);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, g_free);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(nm, g_object_unref);

    NMClient* client = ffnm_client_new(NULL, NULL);
    if(!client)
    {
        dlclose(nm);
        return "Could not create NMClient";
    }

    /* Get all devices managed by NetworkManager */
    const GPtrArray* devices = ffnm_client_get_devices(client);

    /* Go through the array and process Wi-Fi devices */
    for (guint i = 0; i < devices->len; i++)
    {
        NMDevice *device = g_ptr_array_index(devices, i);
        if (!({
            GTypeInstance *__inst = (GTypeInstance*) device; GType __t = ffnm_device_wifi_get_type(); gboolean __r;
            if (!__inst)
                __r = FALSE;
            else if (__inst->g_class && __inst->g_class->g_type == __t)
                __r = TRUE;
            else
                __r = ffg_type_check_instance_is_a (__inst, __t);
            __r;
            }) //NM_IS_DEVICE_WIFI(device)
        )
            continue;

        FFWifiResult* item = (FFWifiResult*) ffListAdd(result);
        ffStrbufInit(&item->inf.description);
        ffStrbufInit(&item->inf.status);
        ffStrbufInit(&item->conn.status);
        ffStrbufInit(&item->conn.ssid);
        ffStrbufInit(&item->conn.macAddress);
        ffStrbufInit(&item->conn.protocol);
        ffStrbufInit(&item->conn.security);
        item->conn.signalQuality = 0.0/0.0;
        item->conn.rxRate = 0.0/0.0;
        item->conn.txRate = 0.0/0.0;

        ffStrbufAppendS(&item->inf.description, ffnm_device_get_iface(device));
        NMDeviceState state = ffnm_device_get_state(device);
        switch(state)
        {
            case NM_DEVICE_STATE_UNKNOWN:
                ffStrbufAppendS(&item->inf.status, "Unknown");
                break;
            case NM_DEVICE_STATE_UNMANAGED:
                ffStrbufAppendS(&item->inf.status, "Unmanaged");
                break;
            case NM_DEVICE_STATE_UNAVAILABLE:
                ffStrbufAppendS(&item->inf.status, "Unavailable");
                break;
            case NM_DEVICE_STATE_DISCONNECTED:
                ffStrbufAppendS(&item->inf.status, "Disconnected");
                break;
            case NM_DEVICE_STATE_PREPARE:
                ffStrbufAppendS(&item->inf.status, "Prepare");
                break;
            case NM_DEVICE_STATE_CONFIG:
                ffStrbufAppendS(&item->inf.status, "Config");
                break;
            case NM_DEVICE_STATE_NEED_AUTH:
                ffStrbufAppendS(&item->inf.status, "Need auth");
                break;
            case NM_DEVICE_STATE_IP_CONFIG:
                ffStrbufAppendS(&item->inf.status, "IP config");
                break;
            case NM_DEVICE_STATE_IP_CHECK:
                ffStrbufAppendS(&item->inf.status, "IP check");
                break;
            case NM_DEVICE_STATE_SECONDARIES:
                ffStrbufAppendS(&item->inf.status, "Secondaries");
                break;
            case NM_DEVICE_STATE_ACTIVATED:
                ffStrbufAppendS(&item->inf.status, "Activated");
                break;
            case NM_DEVICE_STATE_DEACTIVATING:
                ffStrbufAppendS(&item->inf.status, "Deactivating");
                break;
            case NM_DEVICE_STATE_FAILED:
                ffStrbufAppendS(&item->inf.status, "Failed");
                break;
        }
        if(state != NM_DEVICE_STATE_ACTIVATED)
            continue;

        NMAccessPoint* ap = ffnm_device_wifi_get_active_access_point((NMDeviceWifi *) device);
        if (!ap)
            continue;

        GBytes* activeSsid = ffnm_access_point_get_ssid(ap);
        if (activeSsid)
        {
            char* ssid = ffnm_utils_ssid_to_utf8(ffg_bytes_get_data(activeSsid, NULL), ffg_bytes_get_size(activeSsid));
            ffStrbufAppendS(&item->conn.ssid, ssid);
            ffg_free(ssid);
        }

        ffStrbufAppendS(&item->conn.macAddress, ffnm_access_point_get_bssid(ap));
        item->conn.signalQuality = ffnm_access_point_get_strength(ap);
        item->conn.rxRate = ffnm_access_point_get_max_bitrate(ap);

        if(instance->config.allowSlowOperations)
        {
            FF_STRBUF_AUTO_DESTROY output;
            ffStrbufInit(&output);
            if(!ffProcessAppendStdOut(&output, (char* const[]){
                "iw",
                "dev",
                item->inf.description.chars,
                "link",
                NULL
            }) && ffParsePropLines(output.chars, "tx bitrate: ", &item->conn.protocol))
            {
                if(ffStrbufContainS(&item->conn.protocol, " HE-MCS "))
                    ffStrbufSetS(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
                else if(ffStrbufContainS(&item->conn.protocol, " VHT-MCS "))
                    ffStrbufSetS(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
                else if(ffStrbufContainS(&item->conn.protocol, " MCS "))
                    ffStrbufSetS(&item->conn.protocol, "802.11n (Wi-Fi 4)");
                else
                    ffStrbufSetS(&item->conn.protocol, "802.11a/b/g");
            }
        }

        NM80211ApFlags flags = ffnm_access_point_get_flags(ap);
        NM80211ApSecurityFlags wpaFlags = ffnm_access_point_get_wpa_flags(ap);
        NM80211ApSecurityFlags rsnFlags = ffnm_access_point_get_rsn_flags(ap);

        if ((flags & NM_802_11_AP_FLAGS_PRIVACY) && (wpaFlags == NM_802_11_AP_SEC_NONE)
            && (rsnFlags == NM_802_11_AP_SEC_NONE))
            ffStrbufAppendS(&item->conn.security, "WEP ");
        if (wpaFlags != NM_802_11_AP_SEC_NONE)
            ffStrbufAppendS(&item->conn.security, "WPA ");
        if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
            ffStrbufAppendS(&item->conn.security, "WPA2 ");
        }
        if (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_SAE) {
            ffStrbufAppendS(&item->conn.security, "WPA3 ");
        }
        if ((rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_OWE_TM)) {
            ffStrbufAppendS(&item->conn.security, "OWE ");
        }
        if ((wpaFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
            || (rsnFlags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
            ffStrbufAppendS(&item->conn.security, "802.1X ");
        }
        if (!item->conn.security.length)
            ffStrbufAppendS(&item->conn.security, "Insecure");
        else
            ffStrbufTrimRight(&item->conn.security, ' ');
    }

    ffg_object_unref(client);
    dlclose(nm);

    return NULL;
}

#endif

#include "common/io/io.h"

#include <net/if.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <unistd.h>

static const char* detectWifiWithIoctls(FF_MAYBE_UNUSED const FFinstance* instance, FFlist* result)
{
    struct if_nameindex* infs = if_nameindex();
    if(!infs)
        return "if_nameindex() failed";

    FFstrbuf path;
    ffStrbufInit(&path);

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        ffStrbufSetF(&path, "/sys/class/net/%s/phy80211", i->if_name);
        if(!ffPathExists(path.chars, FF_PATHTYPE_DIRECTORY))
            continue;

        FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
        ffStrbufInitS(&item->inf.description, i->if_name);
        ffStrbufInit(&item->inf.status);
        ffStrbufInit(&item->conn.status);
        ffStrbufInit(&item->conn.ssid);
        ffStrbufInit(&item->conn.macAddress);
        ffStrbufInit(&item->conn.protocol);
        ffStrbufInit(&item->conn.security);
        item->conn.signalQuality = 0.0/0.0;
        item->conn.rxRate = 0.0/0.0;
        item->conn.txRate = 0.0/0.0;

        ffStrbufSetF(&path, "/sys/class/net/%s/operstate", i->if_name);
        if(!ffAppendFileBuffer(path.chars, &item->inf.status) || !ffStrbufEqualS(&item->inf.status, "up"))
            continue;

        int sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
        if(sock < 0)
            continue;

        struct iwreq iwr;
        strncpy(iwr.ifr_name, i->if_name, IFNAMSIZ);
        ffStrbufEnsureFree(&item->conn.ssid, IW_ESSID_MAX_SIZE);
        iwr.u.essid.pointer = (caddr_t) item->conn.ssid.chars;
        iwr.u.essid.length = IW_ESSID_MAX_SIZE + 1;
        iwr.u.essid.flags = 0;
        if(ioctl(sock, SIOCGIWESSID, &iwr) >= 0)
            ffStrbufRecalculateLength(&item->conn.ssid);

        if(ioctl(sock, SIOCGIWNAME, &iwr) >= 0)
        {
            if(strncasecmp(iwr.u.name, "IEEE ", 5) == 0)
                ffStrbufSetS(&item->conn.protocol, iwr.u.name + 5);
            else
                ffStrbufSetS(&item->conn.protocol, iwr.u.name);
        }

        if(ioctl(sock, SIOCGIWAP, &iwr) >= 0)
        {
            for(int i = 0; i < 6; ++i)
                ffStrbufAppendF(&item->conn.macAddress, "%.2X-", (uint8_t) iwr.u.ap_addr.sa_data[i]);
            ffStrbufTrimRight(&item->conn.macAddress, '-');
        }

        //FIXME: doesn't work
        if(ioctl(sock, SIOCGIWSPY, &iwr) >= 0)
            item->conn.signalQuality = iwr.u.qual.level;

        //FIXME: doesn't work
        struct iw_encode_ext iwe;
        iwr.u.data.pointer = &iwe;
        iwr.u.data.length = sizeof(iwe);
        iwr.u.data.flags = 0;
        if(ioctl(sock, SIOCGIWENCODEEXT, &iwr) >= 0)
        {
            struct iw_encode_ext* iwe = iwr.u.data.pointer;
            switch(iwe->alg)
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
                    ffStrbufAppendF(&item->conn.security, "Unknown (%d)", (int) iwe->alg);
                    break;
            }
        }

        close(sock);
    }
    if_freenameindex(infs);
    ffStrbufDestroy(&path);

    return NULL;
}

const char* ffDetectWifi(const FFinstance* instance, FFlist* result)
{
    #ifdef FF_HAVE_LIBNM
    if(!detectWifiWithLibnm(instance, result))
        return NULL;
    #endif

    return detectWifiWithIoctls(instance, result);
}
