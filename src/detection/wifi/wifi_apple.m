#include "wifi.h"

#import <CoreWLAN/CoreWLAN.h>

const char* ffDetectWifi(const FFinstance* instance, FFlist* result)
{
    FF_UNUSED(instance);

    NSArray<CWInterface*>* interfaces = CWWiFiClient.sharedWiFiClient.interfaces;
    if(!interfaces)
        return "CWWiFiClient.sharedWiFiClient.interfaces is nil";

    if(interfaces.count == 0)
        return "No wifi interfaces found";

    for(CWInterface* inf in interfaces)
    {
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
        item->security.type = FF_WIFI_SECURITY_UNKNOWN;
        item->security.oneXEnabled = false;
        ffStrbufInit(&item->security.algorithm);

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
                ffStrbufAppendS(&item->conn.phyType, "none");
                break;
            case kCWPHYMode11a:
                ffStrbufAppendS(&item->conn.phyType, "802.11a");
                break;
            case kCWPHYMode11b:
                ffStrbufAppendS(&item->conn.phyType, "802.11b");
                break;
            case kCWPHYMode11g:
                ffStrbufAppendS(&item->conn.phyType, "802.11g");
                break;
            case kCWPHYMode11n:
                ffStrbufAppendS(&item->conn.phyType, "802.11n (Wi-Fi 4)");
                break;
            case kCWPHYMode11ac:
                ffStrbufAppendS(&item->conn.phyType, "802.11ac (Wi-Fi 5)");
                break;
            case 6 /*kCWPHYMode11ax*/:
                ffStrbufAppendS(&item->conn.phyType, "802.11ax (Wi-Fi 6)");
                break;
            case 7 /*kCWPHYMode11be?*/:
                ffStrbufAppendS(&item->conn.phyType, "802.11be (Wi-Fi 7)");
                break;
            default:
                ffStrbufAppendF(&item->conn.phyType, "Unknown (%ld)", inf.activePHYMode);
                break;
        }
        item->conn.signalQuality = inf.rssiValue >= -50 ? 100 : inf.rssiValue <= -100 ? 0 : (inf.rssiValue + 100) * 2;
        item->conn.txRate = inf.transmitRate;
        item->security.type = inf.security != kCWSecurityNone ? FF_WIFI_SECURITY_ENABLED : FF_WIFI_SECURITY_DISABLED;
        switch(inf.security)
        {
            case kCWSecurityNone:
                ffStrbufAppendS(&item->security.algorithm, "None");
                break;
            case kCWSecurityWEP:
                ffStrbufAppendS(&item->security.algorithm, "WEP");
                break;
            case kCWSecurityWPAPersonal:
                ffStrbufAppendS(&item->security.algorithm, "WPA Personal");
                break;
            case kCWSecurityWPAPersonalMixed:
                ffStrbufAppendS(&item->security.algorithm, "WPA Persional Mixed");
                break;
            case kCWSecurityWPA2Personal:
                ffStrbufAppendS(&item->security.algorithm, "WPA2 Personal");
                break;
            case kCWSecurityPersonal:
                ffStrbufAppendS(&item->security.algorithm, "Personal");
                break;
            case kCWSecurityDynamicWEP:
                ffStrbufAppendS(&item->security.algorithm, "Dynamic WEP");
                break;
            case kCWSecurityWPAEnterprise:
                ffStrbufAppendS(&item->security.algorithm, "WPA Enterprise");
                break;
            case kCWSecurityWPAEnterpriseMixed:
                ffStrbufAppendS(&item->security.algorithm, "WPA Enterprise Mixed");
                break;
            case kCWSecurityWPA2Enterprise:
                ffStrbufAppendS(&item->security.algorithm, "WPA2 Enterprise");
                break;
            case kCWSecurityEnterprise:
                ffStrbufAppendS(&item->security.algorithm, "Enterprise");
                break;
            case kCWSecurityWPA3Personal:
                ffStrbufAppendS(&item->security.algorithm, "WPA3 Personal");
                break;
            case kCWSecurityWPA3Enterprise:
                ffStrbufAppendS(&item->security.algorithm, "WPA3 Enterprise");
                break;
            case kCWSecurityWPA3Transition:
                ffStrbufAppendS(&item->security.algorithm, "WPA3 Transition");
                break;
            case 14 /*kCWSecurityOWE*/:
                ffStrbufAppendS(&item->security.algorithm, "OWE");
                break;
            case 15 /*kCWSecurityOWETransition*/:
                ffStrbufAppendS(&item->security.algorithm, "OWE Transition");
                break;
            default:
                ffStrbufAppendF(&item->security.algorithm, "Unknown (%ld)", inf.security);
                break;
        }
    }
    return NULL;
}
