#include "wifi.h"
#include "common/library.h"
#include "common/windows/unicode.h"
#include "common/windows/registry.h"

#include <windows.h>
#include <wlanapi.h>

static void convertIfStateToString(WLAN_INTERFACE_STATE state, FFstrbuf* result) {
    switch (state) {
        case wlan_interface_state_not_ready:
            ffStrbufSetStatic(result, "Not ready");
            break;
        case wlan_interface_state_connected:
            ffStrbufSetStatic(result, "Connected");
            break;
        case wlan_interface_state_ad_hoc_network_formed:
            ffStrbufSetStatic(result, "Ad hoc network formed");
            break;
        case wlan_interface_state_disconnecting:
            ffStrbufSetStatic(result, "Disconnecting");
            break;
        case wlan_interface_state_disconnected:
            ffStrbufSetStatic(result, "Disconnected");
            break;
        case wlan_interface_state_associating:
            ffStrbufSetStatic(result, "Associating");
            break;
        case wlan_interface_state_discovering:
            ffStrbufSetStatic(result, "Discovering");
            break;
        case wlan_interface_state_authenticating:
            ffStrbufSetStatic(result, "Authenticating");
            break;
        default:
            ffStrbufSetStatic(result, "Unknown");
            break;
    }
}

typedef struct _WLAN_REALTIME_CONNECTION_QUALITY_LINK_INFO {
    UCHAR ucLinkID;
    ULONG ulChannelCenterFrequencyMhz;
    ULONG ulBandwidth;
    LONG lRssi;
    WLAN_RATE_SET wlanRateSet;
} WLAN_REALTIME_CONNECTION_QUALITY_LINK_INFO, *PWLAN_REALTIME_CONNECTION_QUALITY_LINK_INFO;

typedef struct _WLAN_REALTIME_CONNECTION_QUALITY {
    DOT11_PHY_TYPE dot11PhyType;
    ULONG ulLinkQuality;
    ULONG ulRxRate;
    ULONG ulTxRate;
    BOOL bIsMLOConnection;
    ULONG ulNumLinks;
    // Array of size ulNumLinks
    WLAN_REALTIME_CONNECTION_QUALITY_LINK_INFO linksInfo[];
} WLAN_REALTIME_CONNECTION_QUALITY, *PWLAN_REALTIME_CONNECTION_QUALITY;

enum { wlan_intf_opcode_realtime_connection_quality = 19 };

#define WIFI_DRIVER_REG_PATH L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"

static bool detectWifiDriver(const GUID* interfaceGuid, FFstrbuf* driver) {
    char interfaceGuidA[64];
    snprintf(interfaceGuidA, sizeof(interfaceGuidA),
        "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        (unsigned) interfaceGuid->Data1,
        (unsigned) interfaceGuid->Data2,
        (unsigned) interfaceGuid->Data3,
        (unsigned) interfaceGuid->Data4[0], (unsigned) interfaceGuid->Data4[1],
        (unsigned) interfaceGuid->Data4[2], (unsigned) interfaceGuid->Data4[3],
        (unsigned) interfaceGuid->Data4[4], (unsigned) interfaceGuid->Data4[5],
        (unsigned) interfaceGuid->Data4[6], (unsigned) interfaceGuid->Data4[7]);

    FF_AUTO_CLOSE_FD HANDLE hClassKey = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, WIFI_DRIVER_REG_PATH, &hClassKey, nullptr)) {
        return false;
    }

    wchar_t subKeyW[16];

    for (uint32_t i = 0;; ++i) {
        _snwprintf(subKeyW, ARRAY_SIZE(subKeyW), L"%04u", i);

        FF_AUTO_CLOSE_FD HANDLE hDeviceKey = nullptr;
        if (!ffRegOpenSubkeyForRead(hClassKey, subKeyW, &hDeviceKey, nullptr)) {
            return false;
        }

        FF_STRBUF_AUTO_DESTROY netCfgInstanceId = ffStrbufCreate();
        if (!ffRegReadStrbuf(hDeviceKey, L"NetCfgInstanceId", &netCfgInstanceId, nullptr)) {
            continue;
        }

        if (!ffStrbufEqualS(&netCfgInstanceId, interfaceGuidA)) {
            continue;
        }

        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        if (ffRegReadStrbuf(hDeviceKey, L"ProviderName", &buffer, nullptr)) {
            ffStrbufSet(driver, &buffer);
        }

        if (ffRegReadStrbuf(hDeviceKey, L"DriverVersion", &buffer, nullptr)) {
            if (driver->length) {
                ffStrbufAppendC(driver, ' ');
            }
            ffStrbufAppend(driver, &buffer);
        }

        return true;
    }
}

const char* ffDetectWifi(FFlist* result) {
    FF_LIBRARY_LOAD_MESSAGE(wlanapi, "wlanapi" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanOpenHandle)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanEnumInterfaces)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanQueryInterface)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanFreeMemory)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanCloseHandle)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanGetNetworkBssList)

    DWORD curVersion;
    HANDLE hClient = nullptr;
    WLAN_INTERFACE_INFO_LIST* ifList = nullptr;
    const char* error = nullptr;

    if (ffWlanOpenHandle(2 /*maxClientVersion*/, nullptr, &curVersion, &hClient) != ERROR_SUCCESS) {
        error = "WlanOpenHandle() failed";
        goto exit;
    }

    if (ffWlanEnumInterfaces(hClient, nullptr, &ifList) != ERROR_SUCCESS) {
        error = "WlanEnumInterfaces() failed";
        goto exit;
    }

    for (uint32_t index = 0; index < ifList->dwNumberOfItems; ++index) {
        WLAN_INTERFACE_INFO* ifInfo = (WLAN_INTERFACE_INFO*) &ifList->InterfaceInfo[index];

        FFWifiResult* item = FF_LIST_ADD(FFWifiResult, *result);
        ffStrbufInitWS(&item->inf.description, ifInfo->strInterfaceDescription);
        ffStrbufInit(&item->inf.status);
        ffStrbufInit(&item->inf.driver);
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

        detectWifiDriver(&ifInfo->InterfaceGuid, &item->inf.driver);
        convertIfStateToString(ifInfo->isState, &item->inf.status);

        if (ifInfo->isState != wlan_interface_state_connected) {
            continue;
        }

        WLAN_CONNECTION_ATTRIBUTES* connInfo = nullptr;
        DWORD bufSize = sizeof(*connInfo);
        WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_query_only;

        if (ffWlanQueryInterface(hClient,
                &ifInfo->InterfaceGuid,
                wlan_intf_opcode_current_connection,
                nullptr,
                &bufSize,
                (PVOID*) &connInfo,
                &opCode) != ERROR_SUCCESS) {
            continue;
        }

        convertIfStateToString(connInfo->isState, &item->conn.status);
        ffStrbufAppendNS(&item->conn.ssid,
            connInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength,
            (const char*) connInfo->wlanAssociationAttributes.dot11Ssid.ucSSID);

        for (size_t i = 0; i < sizeof(connInfo->wlanAssociationAttributes.dot11Bssid); i++) {
            ffStrbufAppendF(&item->conn.bssid, "%.2X:", connInfo->wlanAssociationAttributes.dot11Bssid[i]);
        }
        ffStrbufTrimRight(&item->conn.bssid, ':');

        switch (connInfo->wlanAssociationAttributes.dot11PhyType) {
            case dot11_phy_type_fhss:
                ffStrbufAppendS(&item->conn.protocol, "802.11 (FHSS)");
                break;
            case dot11_phy_type_dsss:
                ffStrbufAppendS(&item->conn.protocol, "802.11 (DSSS)");
                break;
            case dot11_phy_type_irbaseband:
                ffStrbufAppendS(&item->conn.protocol, "802.11 (IR)");
                break;
            case dot11_phy_type_ofdm:
                ffStrbufAppendS(&item->conn.protocol, "802.11a");
                break;
            case dot11_phy_type_hrdsss:
                ffStrbufAppendS(&item->conn.protocol, "802.11b");
                break;
            case dot11_phy_type_erp:
                ffStrbufAppendS(&item->conn.protocol, "802.11g");
                break;
            case dot11_phy_type_ht:
                ffStrbufAppendS(&item->conn.protocol, "802.11n (Wi-Fi 4)");
                break;
            case dot11_phy_type_vht:
                ffStrbufAppendS(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
                break;
            case dot11_phy_type_dmg:
                ffStrbufAppendS(&item->conn.protocol, "802.11ad (WiGig)");
                break;
            case dot11_phy_type_he:
                ffStrbufAppendS(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
                break;
            case dot11_phy_type_eht:
                ffStrbufAppendS(&item->conn.protocol, "802.11be (Wi-Fi 7)");
                break;
            default:
                ffStrbufAppendF(&item->conn.protocol, "Unknown (%u)", (unsigned) connInfo->wlanAssociationAttributes.dot11PhyType);
                break;
        }

        item->conn.signalQuality = connInfo->wlanAssociationAttributes.wlanSignalQuality;
        item->conn.rxRate = connInfo->wlanAssociationAttributes.ulRxRate / 1000.;
        item->conn.txRate = connInfo->wlanAssociationAttributes.ulTxRate / 1000.;

        if (connInfo->wlanSecurityAttributes.bSecurityEnabled) {
            switch (connInfo->wlanSecurityAttributes.dot11AuthAlgorithm) {
                case DOT11_AUTH_ALGO_80211_OPEN:
                    ffStrbufAppendS(&item->conn.security, "802.11 Open");
                    break;
                case DOT11_AUTH_ALGO_80211_SHARED_KEY:
                    ffStrbufAppendS(&item->conn.security, "802.11 Shared");
                    break;
                case DOT11_AUTH_ALGO_WPA:
                    ffStrbufAppendS(&item->conn.security, "WPA");
                    break;
                case DOT11_AUTH_ALGO_WPA_PSK:
                    ffStrbufAppendS(&item->conn.security, "WPA-PSK");
                    break;
                case DOT11_AUTH_ALGO_WPA_NONE:
                    ffStrbufAppendS(&item->conn.security, "WPA-None");
                    break;
                case DOT11_AUTH_ALGO_RSNA:
                    ffStrbufAppendS(&item->conn.security, "WPA2");
                    break;
                case DOT11_AUTH_ALGO_RSNA_PSK:
                    ffStrbufAppendS(&item->conn.security, "WPA2-PSK");
                    break;
                case DOT11_AUTH_ALGO_WPA3:
                    ffStrbufAppendS(&item->conn.security, "WPA3");
                    break;
                case DOT11_AUTH_ALGO_WPA3_SAE:
                    ffStrbufAppendS(&item->conn.security, "WPA3-SAE");
                    break;
                case 10 /* DOT11_AUTH_ALGO_OWE */:
                    ffStrbufAppendS(&item->conn.security, "OWE");
                    break;
                case 11 /* DOT11_AUTH_ALGO_WPA3_ENT */:
                    ffStrbufAppendS(&item->conn.security, "WPA3-ENT");
                    break;
                default:
                    ffStrbufAppendF(&item->conn.security, "Unknown (%u)", (unsigned) connInfo->wlanSecurityAttributes.dot11AuthAlgorithm);
                    break;
            }
            if (connInfo->wlanSecurityAttributes.bOneXEnabled) {
                ffStrbufAppendS(&item->conn.security, " 802.11X");
            }
        } else {
            ffStrbufAppendS(&item->conn.security, "Insecure");
        }

        WLAN_REALTIME_CONNECTION_QUALITY* connectionQuality = nullptr;
        bufSize = 0;
        if (ffWlanQueryInterface(hClient,
                &ifInfo->InterfaceGuid,
                wlan_intf_opcode_realtime_connection_quality,
                nullptr,
                &bufSize,
                (PVOID*) &connectionQuality,
                &opCode) == ERROR_SUCCESS) {
            const WLAN_REALTIME_CONNECTION_QUALITY_LINK_INFO* bestLink = nullptr;
            for (ULONG linkIndex = 0; linkIndex < connectionQuality->ulNumLinks; ++linkIndex) {
                const WLAN_REALTIME_CONNECTION_QUALITY_LINK_INFO* link = &connectionQuality->linksInfo[linkIndex];
                if (!bestLink || link->lRssi > bestLink->lRssi) {
                    bestLink = link;
                }
            }
            if (bestLink) {
                item->conn.channelWidth = (uint16_t) bestLink->ulBandwidth;
                item->conn.frequency = (uint16_t) bestLink->ulChannelCenterFrequencyMhz;
            }
        }
        if (connectionQuality) {
            ffWlanFreeMemory(connectionQuality);
        }

        if (item->conn.frequency == 0) {
            WLAN_BSS_LIST* bssList = nullptr;
            if (ffWlanGetNetworkBssList(hClient,
                    &ifInfo->InterfaceGuid,
                    &connInfo->wlanAssociationAttributes.dot11Ssid,
                    connInfo->wlanAssociationAttributes.dot11BssType,
                    connInfo->wlanSecurityAttributes.bSecurityEnabled,
                    nullptr,
                    &bssList) == ERROR_SUCCESS &&
                bssList->dwNumberOfItems > 0) {
                item->conn.frequency = (uint16_t) (bssList->wlanBssEntries[0].ulChCenterFrequency / 1000);
                ffWlanFreeMemory(bssList);
            }
        }

        ffWlanFreeMemory(connInfo);

        ULONG* channelNumber = 0;
        bufSize = sizeof(*channelNumber);
        if (ffWlanQueryInterface(hClient,
                &ifInfo->InterfaceGuid,
                wlan_intf_opcode_channel_number,
                nullptr,
                &bufSize,
                (PVOID*) &channelNumber,
                &opCode) == ERROR_SUCCESS) {
            item->conn.channel = (uint16_t) *channelNumber;
            ffWlanFreeMemory(channelNumber);
        }
    }

exit:
    if (ifList) {
        ffWlanFreeMemory(ifList);
    }
    if (hClient) {
        ffWlanCloseHandle(hClient, nullptr);
    }
    return error;
}
