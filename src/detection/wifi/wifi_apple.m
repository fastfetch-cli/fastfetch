#include "wifi.h"

#import <CoreWLAN/CoreWLAN.h>

const char* ffDetectWifi(FFlist* result)
{
    NSArray<CWInterface*>* interfaces = CWWiFiClient.sharedWiFiClient.interfaces;
    if(!interfaces)
        return "CWWiFiClient.sharedWiFiClient.interfaces is nil";

    for(CWInterface* inf in interfaces)
    {
        FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
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

        ffStrbufAppendS(&item->inf.description, inf.interfaceName.UTF8String);
        ffStrbufAppendS(&item->inf.status, inf.powerOn ? "Power On" : "Power Off");
        if(!inf.powerOn)
            continue;

        ffStrbufAppendS(&item->conn.status, inf.serviceActive ? "Active" : "Inactive");
        if(!inf.serviceActive)
            continue;

        ffStrbufAppendS(&item->conn.ssid, inf.ssid.UTF8String);
        ffStrbufAppendS(&item->conn.macAddress, inf.hardwareAddress.UTF8String);
        switch(inf.activePHYMode)
        {
            case kCWPHYModeNone:
                ffStrbufAppendS(&item->conn.protocol, "none");
                break;
            case kCWPHYMode11a:
                ffStrbufAppendS(&item->conn.protocol, "802.11a");
                break;
            case kCWPHYMode11b:
                ffStrbufAppendS(&item->conn.protocol, "802.11b");
                break;
            case kCWPHYMode11g:
                ffStrbufAppendS(&item->conn.protocol, "802.11g");
                break;
            case kCWPHYMode11n:
                ffStrbufAppendS(&item->conn.protocol, "802.11n (Wi-Fi 4)");
                break;
            case kCWPHYMode11ac:
                ffStrbufAppendS(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
                break;
            case 6 /*kCWPHYMode11ax*/:
                ffStrbufAppendS(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
                break;
            case 7 /*kCWPHYMode11be?*/:
                ffStrbufAppendS(&item->conn.protocol, "802.11be (Wi-Fi 7)");
                break;
            default:
                ffStrbufAppendF(&item->conn.protocol, "Unknown (%ld)", inf.activePHYMode);
                break;
        }
        item->conn.signalQuality = (double) (inf.rssiValue >= -50 ? 100 : inf.rssiValue <= -100 ? 0 : (inf.rssiValue + 100) * 2);
        item->conn.txRate = inf.transmitRate;

        switch(inf.security)
        {
            case kCWSecurityNone:
                ffStrbufAppendS(&item->conn.security, "Insecure");
                break;
            case kCWSecurityWEP:
                ffStrbufAppendS(&item->conn.security, "WEP");
                break;
            case kCWSecurityWPAPersonal:
                ffStrbufAppendS(&item->conn.security, "WPA Personal");
                break;
            case kCWSecurityWPAPersonalMixed:
                ffStrbufAppendS(&item->conn.security, "WPA Persional Mixed");
                break;
            case kCWSecurityWPA2Personal:
                ffStrbufAppendS(&item->conn.security, "WPA2 Personal");
                break;
            case kCWSecurityPersonal:
                ffStrbufAppendS(&item->conn.security, "Personal");
                break;
            case kCWSecurityDynamicWEP:
                ffStrbufAppendS(&item->conn.security, "Dynamic WEP");
                break;
            case kCWSecurityWPAEnterprise:
                ffStrbufAppendS(&item->conn.security, "WPA Enterprise");
                break;
            case kCWSecurityWPAEnterpriseMixed:
                ffStrbufAppendS(&item->conn.security, "WPA Enterprise Mixed");
                break;
            case kCWSecurityWPA2Enterprise:
                ffStrbufAppendS(&item->conn.security, "WPA2 Enterprise");
                break;
            case kCWSecurityEnterprise:
                ffStrbufAppendS(&item->conn.security, "Enterprise");
                break;
            case kCWSecurityWPA3Personal:
                ffStrbufAppendS(&item->conn.security, "WPA3 Personal");
                break;
            case kCWSecurityWPA3Enterprise:
                ffStrbufAppendS(&item->conn.security, "WPA3 Enterprise");
                break;
            case kCWSecurityWPA3Transition:
                ffStrbufAppendS(&item->conn.security, "WPA3 Transition");
                break;
            case 14 /*kCWSecurityOWE*/:
                ffStrbufAppendS(&item->conn.security, "OWE");
                break;
            case 15 /*kCWSecurityOWETransition*/:
                ffStrbufAppendS(&item->conn.security, "OWE Transition");
                break;
            default:
                ffStrbufAppendF(&item->conn.security, "Unknown (%ld)", inf.security);
                break;
        }
    }
    return NULL;
}
