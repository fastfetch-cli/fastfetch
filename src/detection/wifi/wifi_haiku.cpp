extern "C" {
    #include "wifi.h"
}

#include <NetworkDevice.h>
#include <NetworkInterface.h>
#include <NetworkRoster.h>

static void initWifiResult(FFWifiResult* item, const char* name) {
    ffStrbufInitS(&item->inf.description, name);
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
}

static void setWifiSecurity(FFstrbuf* security, const wireless_network& network) {
    switch (network.authentication_mode) {
        case B_NETWORK_AUTHENTICATION_NONE:
            if (network.flags & B_NETWORK_IS_ENCRYPTED) {
                ffStrbufSetStatic(security, "Encrypted");
            } else {
                ffStrbufSetStatic(security, "Insecure");
            }
            break;
        case B_NETWORK_AUTHENTICATION_WEP:
            ffStrbufSetStatic(security, "WEP");
            break;
        case B_NETWORK_AUTHENTICATION_WPA:
            ffStrbufSetStatic(security, "WPA");
            break;
        case B_NETWORK_AUTHENTICATION_WPA2:
            ffStrbufSetStatic(security, "WPA2");
            break;
        case B_NETWORK_AUTHENTICATION_EAP:
            ffStrbufSetStatic(security, "WPA Enterprise");
            break;
        default:
            ffStrbufSetF(security, "Unknown (%lu)", (unsigned long) network.authentication_mode);
            break;
    }
}

static void setWifiBssid(FFstrbuf* bssid, const wireless_network& network) {
    const uint8* address = network.address.LinkLevelAddress();
    if (address == nullptr || network.address.LinkLevelAddressLength() < 6) {
        return;
    }

    ffStrbufSetF(bssid, "%02X:%02X:%02X:%02X:%02X:%02X",
        address[0], address[1], address[2], address[3], address[4], address[5]);
}

const char* ffDetectWifi(FFlist* result) {
    BNetworkRoster& roster = BNetworkRoster::Default();
    BNetworkInterface interface;
    uint32 cookie = 0;

    while (roster.GetNextInterface(&cookie, interface) == B_OK) {
        if (!interface.Exists()) {
            continue;
        }

        BNetworkDevice device(interface.Name());
        if (!device.Exists() || !device.IsWireless()) {
            continue;
        }

        FFWifiResult* item = FF_LIST_ADD(FFWifiResult, *result);
        initWifiResult(item, interface.Name());

        ffStrbufSetStatic(&item->inf.status, interface.HasLink() ? "Up" : "Down");
        if (!interface.HasLink() || !device.HasLink()) {
            ffStrbufSetStatic(&item->conn.status, "Not connected");
            continue;
        }

        ffStrbufSetStatic(&item->conn.status, "Connected");

        uint32 networkCookie = 0;
        wireless_network network = {};
        if (device.GetNextAssociatedNetwork(networkCookie, network) != B_OK) {
            continue;
        }

        ffStrbufSetNS(&item->conn.ssid, (uint32_t) strnlen(network.name, sizeof(network.name)), network.name);
        setWifiBssid(&item->conn.bssid, network);
        item->conn.signalQuality = network.signal_strength > 100 ? 100 : network.signal_strength;
        setWifiSecurity(&item->conn.security, network);
    }

    return nullptr;
}
