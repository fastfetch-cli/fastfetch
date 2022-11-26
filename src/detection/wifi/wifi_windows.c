#include "wifi.h"
#include "util/windows/unicode.h"

#include <wlanapi.h>

static inline void wrapCloseHandle(HANDLE* handle)
{
    if(*handle)
        CloseHandle(*handle);
}
static inline void wrapWlanFreeMemory(void* memory)
{
    if(*(void**)memory)
        WlanFreeMemory(*(void**)memory);
}

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

void ffDetectWifi(const FFinstance* instance, FFWifiResult* result)
{
    FF_UNUSED(instance);

    DWORD curVersion;
    HANDLE __attribute__((__cleanup__(wrapCloseHandle))) hClient = NULL;
    if(WlanOpenHandle(1, NULL, &curVersion, &hClient) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&result->error, "WlanOpenHandle() failed");
        return;
    }

    WLAN_INTERFACE_INFO_LIST* __attribute__((__cleanup__(wrapWlanFreeMemory))) ifList = NULL;
    if(WlanEnumInterfaces(hClient, NULL, &ifList) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&result->error, "WlanEnumInterfaces() failed");
        return;
    }

    if(ifList->dwNumberOfItems == 0)
    {
        ffStrbufAppendS(&result->error, "WlanEnumInterfaces() returns empty result");
        return;
    }

    WLAN_INTERFACE_INFO* ifInfo = (WLAN_INTERFACE_INFO*)&ifList->InterfaceInfo[0];
    ffStrbufSetWS(&result->inf.description, ifInfo->strInterfaceDescription);
    convertIfStateToString(ifInfo->isState, &result->inf.status);

    if(ifInfo->isState != wlan_interface_state_connected)
        return;

    WLAN_CONNECTION_ATTRIBUTES* __attribute__((__cleanup__(wrapWlanFreeMemory))) connInfo = NULL;
    DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
    WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;

    if(WlanQueryInterface(hClient,
        &ifInfo->InterfaceGuid,
        wlan_intf_opcode_current_connection,
        NULL,
        &connectInfoSize,
        (PVOID*)&connInfo,
        &opCode) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&result->error, "WlanQueryInterface() failed");
        return;
    }

    convertIfStateToString(connInfo->isState, &result->conn.status);
    ffStrbufAppendNS(&result->conn.ssid,
        connInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength,
        (const char *)connInfo->wlanAssociationAttributes.dot11Ssid.ucSSID);

    for (size_t i = 0; i < sizeof(connInfo->wlanAssociationAttributes.dot11Bssid); i++)
        ffStrbufAppendF(&result->conn.macAddress, "%.2X-", connInfo->wlanAssociationAttributes.dot11Bssid[i]);
    ffStrbufTrimRight(&result->conn.macAddress, '-');

    switch (connInfo->wlanAssociationAttributes.dot11PhyType)
    {
        case dot11_phy_type_fhss:
            ffStrbufAppendS(&result->conn.phyType, "802.11 (FHSS)");
            break;
        case dot11_phy_type_dsss:
            ffStrbufAppendS(&result->conn.phyType, "802.11 (DSSS)");
            break;
        case dot11_phy_type_irbaseband:
            ffStrbufAppendS(&result->conn.phyType, "802.11 (IR)");
            break;
        case dot11_phy_type_ofdm:
            ffStrbufAppendS(&result->conn.phyType, "802.11a");
            break;
        case dot11_phy_type_hrdsss:
            ffStrbufAppendS(&result->conn.phyType, "802.11b");
            break;
        case dot11_phy_type_erp:
            ffStrbufAppendS(&result->conn.phyType, "802.11g");
            break;
        case dot11_phy_type_ht:
            ffStrbufAppendS(&result->conn.phyType, "802.11n (Wi-Fi 4)");
            break;
        case 8 /*dot11_phy_type_vht*/:
            ffStrbufAppendS(&result->conn.phyType, "802.11ac (Wi-Fi 5)");
            break;
        case 9 /*dot11_phy_type_dmg*/:
            ffStrbufAppendS(&result->conn.phyType, "802.11ad (WiGig)");
            break;
        case 10 /*dot11_phy_type_he*/:
            ffStrbufAppendS(&result->conn.phyType, "802.11ax (Wi-Fi 6)");
            break;
        case 11 /*dot11_phy_type_eht*/:
            ffStrbufAppendS(&result->conn.phyType, "802.11be (Wi-Fi 7)");
            break;
        default:
            ffStrbufAppendF(&result->conn.phyType, "Unknown (%u)", (unsigned)connInfo->wlanAssociationAttributes.dot11PhyType);
            break;
    }

    result->conn.signalQuality = connInfo->wlanAssociationAttributes.wlanSignalQuality;
    result->conn.rxRate = connInfo->wlanAssociationAttributes.ulRxRate;
    result->conn.txRate = connInfo->wlanAssociationAttributes.ulTxRate;

    result->security.enabled = connInfo->wlanSecurityAttributes.bSecurityEnabled;
    result->security.oneXEnabled = connInfo->wlanSecurityAttributes.bOneXEnabled;
    switch (connInfo->wlanSecurityAttributes.dot11AuthAlgorithm)
    {
        case DOT11_AUTH_ALGO_80211_OPEN:
            ffStrbufAppendS(&result->security.authAlgo, "802.11 Open");
            break;
        case DOT11_AUTH_ALGO_80211_SHARED_KEY:
            ffStrbufAppendS(&result->security.authAlgo, "802.11 Shared");
            break;
        case DOT11_AUTH_ALGO_WPA:
            ffStrbufAppendS(&result->security.authAlgo, "WPA");
            break;
        case DOT11_AUTH_ALGO_WPA_PSK:
            ffStrbufAppendS(&result->security.authAlgo, "WPA-PSK");
            break;
        case DOT11_AUTH_ALGO_WPA_NONE:
            ffStrbufAppendS(&result->security.authAlgo, "WPA-None");
            break;
        case DOT11_AUTH_ALGO_RSNA:
            ffStrbufAppendS(&result->security.authAlgo, "RSNA");
            break;
        case DOT11_AUTH_ALGO_RSNA_PSK:
            ffStrbufAppendS(&result->security.authAlgo, "RSNA with PSK");
            break;
        case 8 /* DOT11_AUTH_ALGO_WPA3 */:
            ffStrbufAppendS(&result->security.authAlgo, "WPA3");
            break;
        case 9 /* DOT11_AUTH_ALGO_WPA3_SAE */:
            ffStrbufAppendS(&result->security.authAlgo, "WPA3-SAE");
            break;
        case 10 /* DOT11_AUTH_ALGO_OWE */:
            ffStrbufAppendS(&result->security.authAlgo, "OWE");
            break;
        case 11 /* DOT11_AUTH_ALGO_WPA3_ENT */:
            ffStrbufAppendS(&result->security.authAlgo, "OWE-ENT");
            break;
        default:
            ffStrbufAppendF(&result->security.authAlgo, "Unknown (%u)", (unsigned)connInfo->wlanSecurityAttributes.dot11AuthAlgorithm);
            break;
    }
    switch (connInfo->wlanSecurityAttributes.dot11CipherAlgorithm)
    {
    case DOT11_CIPHER_ALGO_NONE:
        ffStrbufAppendS(&result->security.cipherAlgo, "None");
        break;
    case DOT11_CIPHER_ALGO_WEP40:
        ffStrbufAppendS(&result->security.cipherAlgo, "WEP-40");
        break;
    case DOT11_CIPHER_ALGO_TKIP:
        ffStrbufAppendS(&result->security.cipherAlgo, "TKIP");
        break;
    case DOT11_CIPHER_ALGO_CCMP:
        ffStrbufAppendS(&result->security.cipherAlgo, "CCMP");
        break;
    case DOT11_CIPHER_ALGO_WEP104:
        ffStrbufAppendS(&result->security.cipherAlgo, "WEP-104");
        break;
    case 0x06 /* DOT11_CIPHER_ALGO_BIP */:
        ffStrbufAppendS(&result->security.cipherAlgo, "BIP-CMAC-128");
        break;
    case 0x08 /* DOT11_CIPHER_ALGO_GCMP */:
        ffStrbufAppendS(&result->security.cipherAlgo, "GCMP-128");
        break;
    case 0x09 /* DOT11_CIPHER_ALGO_GCMP_256 */:
        ffStrbufAppendS(&result->security.cipherAlgo, "GCMP-256");
        break;
    case 0x0a /* DOT11_CIPHER_ALGO_CCMP_256 */:
        ffStrbufAppendS(&result->security.cipherAlgo, "CCMP-256");
        break;
    case 0x0b /* DOT11_CIPHER_ALGO_BIP_GMAC_128 */:
        ffStrbufAppendS(&result->security.cipherAlgo, "BIP-GMAC-128");
        break;
    case 0x0c /* DOT11_CIPHER_ALGO_BIP_GMAC_256 */:
        ffStrbufAppendS(&result->security.cipherAlgo, "BIP-GMAC-256");
        break;
    case 0x0d /* DOT11_CIPHER_ALGO_BIP_CMAC_256 */:
        ffStrbufAppendS(&result->security.cipherAlgo, "BIP-CMAC-256");
        break;
    case DOT11_CIPHER_ALGO_WEP:
        ffStrbufAppendS(&result->security.cipherAlgo, "WEP");
        break;
    default:
        ffStrbufAppendF(&result->security.cipherAlgo, "Unknown (%u)", (unsigned)connInfo->wlanSecurityAttributes.dot11CipherAlgorithm);
        break;
    }
}

#pragma GCC diagnostic pop
