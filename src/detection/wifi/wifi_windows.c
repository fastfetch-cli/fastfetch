#include "wifi.h"
#include "common/library.h"
#include "util/windows/unicode.h"

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
        ffStrbufAppendS(result, "First node in a ad hoc network");
        break;
    case wlan_interface_state_disconnecting:
        ffStrbufAppendS(result, "Disconnecting");
        break;
    case wlan_interface_state_disconnected:
        ffStrbufAppendS(result, "Not connected");
        break;
    case wlan_interface_state_associating:
        ffStrbufAppendS(result, "Attempting to associate with a network");
        break;
    case wlan_interface_state_discovering:
        ffStrbufAppendS(result, "Auto configuration is discovering settings for the network");
        break;
    case wlan_interface_state_authenticating:
        ffStrbufAppendS(result, "In process of authenticating");
        break;
    default:
        ffStrbufAppendS(result, "Unknown");
        break;
    }
}

const char* ffDetectWifi(FF_UNUSED_PARAM const FFinstance* instance, FFlist* result)
{
    FF_LIBRARY_LOAD(wlanapi, NULL, "dlopen wlanapi"FF_LIBRARY_EXTENSION" failed", "wlanapi"FF_LIBRARY_EXTENSION, 1)
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

    if(ifList->dwNumberOfItems == 0)
    {
        error = "No wifi interfaces found";
        goto exit;
    }

    for(uint32_t index = 0; index < ifList->dwNumberOfItems; ++index)
    {
        WLAN_INTERFACE_INFO* ifInfo = (WLAN_INTERFACE_INFO*)&ifList->InterfaceInfo[index];

        FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
        ffStrbufInit(&item->inf.description);
        ffStrbufInit(&item->inf.status);
        ffStrbufInit(&item->conn.status);
        ffStrbufInit(&item->conn.ssid);
        ffStrbufInit(&item->conn.macAddress);
        ffStrbufInit(&item->conn.phyType);
        item->conn.signalQuality = 0.0/0.0;
        item->conn.rxRate = 0.0/0.0;
        item->conn.txRate = 0.0/0.0;
        item->security.enabled = false;
        item->security.oneXEnabled = false;
        ffStrbufInit(&item->security.algorithm);

        ffStrbufSetWS(&item->inf.description, ifInfo->strInterfaceDescription);
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
            ffStrbufAppendF(&item->conn.macAddress, "%.2X-", connInfo->wlanAssociationAttributes.dot11Bssid[i]);
        ffStrbufTrimRight(&item->conn.macAddress, '-');

        switch (connInfo->wlanAssociationAttributes.dot11PhyType)
        {
            case dot11_phy_type_fhss:
                ffStrbufAppendS(&item->conn.phyType, "802.11 (FHSS)");
                break;
            case dot11_phy_type_dsss:
                ffStrbufAppendS(&item->conn.phyType, "802.11 (DSSS)");
                break;
            case dot11_phy_type_irbaseband:
                ffStrbufAppendS(&item->conn.phyType, "802.11 (IR)");
                break;
            case dot11_phy_type_ofdm:
                ffStrbufAppendS(&item->conn.phyType, "802.11a");
                break;
            case dot11_phy_type_hrdsss:
                ffStrbufAppendS(&item->conn.phyType, "802.11b");
                break;
            case dot11_phy_type_erp:
                ffStrbufAppendS(&item->conn.phyType, "802.11g");
                break;
            case dot11_phy_type_ht:
                ffStrbufAppendS(&item->conn.phyType, "802.11n (Wi-Fi 4)");
                break;
            case 8 /*dot11_phy_type_vht*/:
                ffStrbufAppendS(&item->conn.phyType, "802.11ac (Wi-Fi 5)");
                break;
            case 9 /*dot11_phy_type_dmg*/:
                ffStrbufAppendS(&item->conn.phyType, "802.11ad (WiGig)");
                break;
            case 10 /*dot11_phy_type_he*/:
                ffStrbufAppendS(&item->conn.phyType, "802.11ax (Wi-Fi 6)");
                break;
            case 11 /*dot11_phy_type_eht*/:
                ffStrbufAppendS(&item->conn.phyType, "802.11be (Wi-Fi 7)");
                break;
            default:
                ffStrbufAppendF(&item->conn.phyType, "Unknown (%u)", (unsigned)connInfo->wlanAssociationAttributes.dot11PhyType);
                break;
        }

        item->conn.signalQuality = connInfo->wlanAssociationAttributes.wlanSignalQuality;
        item->conn.rxRate = connInfo->wlanAssociationAttributes.ulRxRate;
        item->conn.txRate = connInfo->wlanAssociationAttributes.ulTxRate;

        item->security.enabled = connInfo->wlanSecurityAttributes.bSecurityEnabled;
        item->security.oneXEnabled = connInfo->wlanSecurityAttributes.bOneXEnabled;
        switch (connInfo->wlanSecurityAttributes.dot11AuthAlgorithm)
        {
            case DOT11_AUTH_ALGO_80211_OPEN:
                ffStrbufAppendS(&item->security.algorithm, "802.11 Open");
                break;
            case DOT11_AUTH_ALGO_80211_SHARED_KEY:
                ffStrbufAppendS(&item->security.algorithm, "802.11 Shared");
                break;
            case DOT11_AUTH_ALGO_WPA:
                ffStrbufAppendS(&item->security.algorithm, "WPA");
                break;
            case DOT11_AUTH_ALGO_WPA_PSK:
                ffStrbufAppendS(&item->security.algorithm, "WPA-PSK");
                break;
            case DOT11_AUTH_ALGO_WPA_NONE:
                ffStrbufAppendS(&item->security.algorithm, "WPA-None");
                break;
            case DOT11_AUTH_ALGO_RSNA:
                ffStrbufAppendS(&item->security.algorithm, "RSNA");
                break;
            case DOT11_AUTH_ALGO_RSNA_PSK:
                ffStrbufAppendS(&item->security.algorithm, "RSNA with PSK");
                break;
            case 8 /* DOT11_AUTH_ALGO_WPA3 */:
                ffStrbufAppendS(&item->security.algorithm, "WPA3");
                break;
            case 9 /* DOT11_AUTH_ALGO_WPA3_SAE */:
                ffStrbufAppendS(&item->security.algorithm, "WPA3-SAE");
                break;
            case 10 /* DOT11_AUTH_ALGO_OWE */:
                ffStrbufAppendS(&item->security.algorithm, "OWE");
                break;
            case 11 /* DOT11_AUTH_ALGO_WPA3_ENT */:
                ffStrbufAppendS(&item->security.algorithm, "OWE-ENT");
                break;
            default:
                ffStrbufAppendF(&item->security.algorithm, "Unknown (%u)", (unsigned)connInfo->wlanSecurityAttributes.dot11AuthAlgorithm);
                break;
        }
        ffStrbufAppendS(&item->security.algorithm, " - ");
        switch (connInfo->wlanSecurityAttributes.dot11CipherAlgorithm)
        {
        case DOT11_CIPHER_ALGO_NONE:
            ffStrbufAppendS(&item->security.algorithm, "None");
            break;
        case DOT11_CIPHER_ALGO_WEP40:
            ffStrbufAppendS(&item->security.algorithm, "WEP-40");
            break;
        case DOT11_CIPHER_ALGO_TKIP:
            ffStrbufAppendS(&item->security.algorithm, "TKIP");
            break;
        case DOT11_CIPHER_ALGO_CCMP:
            ffStrbufAppendS(&item->security.algorithm, "CCMP");
            break;
        case DOT11_CIPHER_ALGO_WEP104:
            ffStrbufAppendS(&item->security.algorithm, "WEP-104");
            break;
        case 0x06 /* DOT11_CIPHER_ALGO_BIP */:
            ffStrbufAppendS(&item->security.algorithm, "BIP-CMAC-128");
            break;
        case 0x08 /* DOT11_CIPHER_ALGO_GCMP */:
            ffStrbufAppendS(&item->security.algorithm, "GCMP-128");
            break;
        case 0x09 /* DOT11_CIPHER_ALGO_GCMP_256 */:
            ffStrbufAppendS(&item->security.algorithm, "GCMP-256");
            break;
        case 0x0a /* DOT11_CIPHER_ALGO_CCMP_256 */:
            ffStrbufAppendS(&item->security.algorithm, "CCMP-256");
            break;
        case 0x0b /* DOT11_CIPHER_ALGO_BIP_GMAC_128 */:
            ffStrbufAppendS(&item->security.algorithm, "BIP-GMAC-128");
            break;
        case 0x0c /* DOT11_CIPHER_ALGO_BIP_GMAC_256 */:
            ffStrbufAppendS(&item->security.algorithm, "BIP-GMAC-256");
            break;
        case 0x0d /* DOT11_CIPHER_ALGO_BIP_CMAC_256 */:
            ffStrbufAppendS(&item->security.algorithm, "BIP-CMAC-256");
            break;
        case DOT11_CIPHER_ALGO_WEP:
            ffStrbufAppendS(&item->security.algorithm, "WEP");
            break;
        default:
            ffStrbufAppendF(&item->security.algorithm, "Unknown (%u)", (unsigned)connInfo->wlanSecurityAttributes.dot11CipherAlgorithm);
            break;
        }
        ffWlanFreeMemory(connInfo);
    }

exit:
    if(ifList) ffWlanFreeMemory(ifList);
    if(hClient) ffWlanCloseHandle(hClient, NULL);
    dlclose(wlanapi);
    return error;
}

#pragma GCC diagnostic pop
