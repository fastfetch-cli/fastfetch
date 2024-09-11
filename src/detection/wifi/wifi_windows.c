#include "wifi.h"
#include "common/library.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <wlanapi.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

static void convertIfStateToString(WLAN_INTERFACE_STATE state, FFstrbuf* result)
{
    switch (state) {
    case wlan_interface_state_not_ready:
        ffStrbufAppendS(result, "Not ready");
        break;
    case wlan_interface_state_connected:
        ffStrbufAppendS(result, "Connected");
        break;
    case wlan_interface_state_ad_hoc_network_formed:
        ffStrbufAppendS(result, "Ad hoc network formed");
        break;
    case wlan_interface_state_disconnecting:
        ffStrbufAppendS(result, "Disconnecting");
        break;
    case wlan_interface_state_disconnected:
        ffStrbufAppendS(result, "Disconnected");
        break;
    case wlan_interface_state_associating:
        ffStrbufAppendS(result, "Associating");
        break;
    case wlan_interface_state_discovering:
        ffStrbufAppendS(result, "Discovering");
        break;
    case wlan_interface_state_authenticating:
        ffStrbufAppendS(result, "Authenticating");
        break;
    default:
        ffStrbufAppendS(result, "Unknown");
        break;
    }
}

const char* ffDetectWifi(FFlist* result)
{
    FF_LIBRARY_LOAD(wlanapi, "dlopen wlanapi" FF_LIBRARY_EXTENSION " failed", "wlanapi" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanOpenHandle)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanEnumInterfaces)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanQueryInterface)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanFreeMemory)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wlanapi, WlanCloseHandle)

    DWORD curVersion;
    HANDLE hClient = NULL;
    WLAN_INTERFACE_INFO_LIST* ifList = NULL;
    const char* error = NULL;

    if(ffWlanOpenHandle(2, NULL, &curVersion, &hClient) != ERROR_SUCCESS)
    {
        error = "WlanOpenHandle() failed";
        goto exit;
    }

    if(ffWlanEnumInterfaces(hClient, NULL, &ifList) != ERROR_SUCCESS)
    {
        error = "WlanEnumInterfaces() failed";
        goto exit;
    }

    for(uint32_t index = 0; index < ifList->dwNumberOfItems; ++index)
    {
        WLAN_INTERFACE_INFO* ifInfo = (WLAN_INTERFACE_INFO*)&ifList->InterfaceInfo[index];

        FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
        ffStrbufInitWS(&item->inf.description, ifInfo->strInterfaceDescription);
        ffStrbufInit(&item->inf.status);
        ffStrbufInit(&item->conn.status);
        ffStrbufInit(&item->conn.ssid);
        ffStrbufInit(&item->conn.bssid);
        ffStrbufInit(&item->conn.protocol);
        ffStrbufInit(&item->conn.security);
        item->conn.signalQuality = 0.0/0.0;
        item->conn.rxRate = 0.0/0.0;
        item->conn.txRate = 0.0/0.0;

        convertIfStateToString(ifInfo->isState, &item->inf.status);

        if(ifInfo->isState != wlan_interface_state_connected)
            continue;

        DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
        WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;
        WLAN_CONNECTION_ATTRIBUTES* connInfo = NULL;

        if(ffWlanQueryInterface(hClient,
            &ifInfo->InterfaceGuid,
            wlan_intf_opcode_current_connection,
            NULL,
            &connectInfoSize,
            (PVOID*)&connInfo,
            &opCode) != ERROR_SUCCESS
        ) continue;

        convertIfStateToString(connInfo->isState, &item->conn.status);
        ffStrbufAppendNS(&item->conn.ssid,
            connInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength,
            (const char *)connInfo->wlanAssociationAttributes.dot11Ssid.ucSSID);

        for (size_t i = 0; i < sizeof(connInfo->wlanAssociationAttributes.dot11Bssid); i++)
            ffStrbufAppendF(&item->conn.bssid, "%.2X-", connInfo->wlanAssociationAttributes.dot11Bssid[i]);
        ffStrbufTrimRight(&item->conn.bssid, '-');

        switch (connInfo->wlanAssociationAttributes.dot11PhyType)
        {
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
                ffStrbufAppendF(&item->conn.protocol, "Unknown (%u)", (unsigned)connInfo->wlanAssociationAttributes.dot11PhyType);
                break;
        }

        item->conn.signalQuality = connInfo->wlanAssociationAttributes.wlanSignalQuality;
        item->conn.rxRate = connInfo->wlanAssociationAttributes.ulRxRate;
        item->conn.txRate = connInfo->wlanAssociationAttributes.ulTxRate;

        if(connInfo->wlanSecurityAttributes.bSecurityEnabled)
        {
            switch (connInfo->wlanSecurityAttributes.dot11AuthAlgorithm)
            {
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
                    ffStrbufAppendS(&item->conn.security, "OWE-ENT");
                    break;
                default:
                    ffStrbufAppendF(&item->conn.security, "Unknown (%u)", (unsigned)connInfo->wlanSecurityAttributes.dot11AuthAlgorithm);
                    break;
            }
            if(connInfo->wlanSecurityAttributes.bOneXEnabled)
                ffStrbufAppendS(&item->conn.security, " 802.11X");
        }
        else
            ffStrbufAppendS(&item->conn.security, "Insecure");

        ffWlanFreeMemory(connInfo);
    }

exit:
    if(ifList) ffWlanFreeMemory(ifList);
    if(hClient) ffWlanCloseHandle(hClient, NULL);
    return error;
}

#pragma GCC diagnostic pop
