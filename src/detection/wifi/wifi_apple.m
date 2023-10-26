#include "wifi.h"
#include "common/processing.h"

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
        ffStrbufSetStatic(&item->inf.status, inf.powerOn ? "Power On" : "Power Off");
        if(!inf.powerOn)
            continue;

        ffStrbufSetStatic(&item->conn.status, inf.serviceActive ? "Active" : "Inactive");
        if(!inf.serviceActive)
            continue;

        if (inf.ssid)
            ffStrbufAppendS(&item->conn.ssid, inf.ssid.UTF8String);
        else if (!ffProcessAppendStdOut(&item->conn.ssid, (char* []) {
            "/usr/sbin/networksetup",
            "-getairportnetwork",
            item->inf.description.chars,
            NULL
        }) && item->conn.ssid.length > 0)
        {
            uint32_t index = ffStrbufFirstIndexC(&item->conn.ssid, ':');
            if (index < item->conn.ssid.length)
                ffStrbufSubstrAfter(&item->conn.ssid, index + 1);
        }
        else
            ffStrbufSetStatic(&item->conn.ssid, "<unknown ssid>"); // https://developer.apple.com/forums/thread/732431

        ffStrbufAppendS(&item->conn.macAddress, inf.hardwareAddress.UTF8String);
        switch(inf.activePHYMode)
        {
            case kCWPHYModeNone:
                ffStrbufSetStatic(&item->conn.protocol, "none");
                break;
            case kCWPHYMode11a:
                ffStrbufSetStatic(&item->conn.protocol, "802.11a");
                break;
            case kCWPHYMode11b:
                ffStrbufSetStatic(&item->conn.protocol, "802.11b");
                break;
            case kCWPHYMode11g:
                ffStrbufSetStatic(&item->conn.protocol, "802.11g");
                break;
            case kCWPHYMode11n:
                ffStrbufSetStatic(&item->conn.protocol, "802.11n (Wi-Fi 4)");
                break;
            case kCWPHYMode11ac:
                ffStrbufSetStatic(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
                break;
            case 6 /*kCWPHYMode11ax*/:
                ffStrbufSetStatic(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
                break;
            case 7 /*kCWPHYMode11be?*/:
                ffStrbufSetStatic(&item->conn.protocol, "802.11be (Wi-Fi 7)");
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
                ffStrbufSetStatic(&item->conn.security, "Insecure");
                break;
            case kCWSecurityWEP:
                ffStrbufSetStatic(&item->conn.security, "WEP");
                break;
            case kCWSecurityWPAPersonal:
                ffStrbufSetStatic(&item->conn.security, "WPA Personal");
                break;
            case kCWSecurityWPAPersonalMixed:
                ffStrbufSetStatic(&item->conn.security, "WPA Persional Mixed");
                break;
            case kCWSecurityWPA2Personal:
                ffStrbufSetStatic(&item->conn.security, "WPA2 Personal");
                break;
            case kCWSecurityPersonal:
                ffStrbufSetStatic(&item->conn.security, "Personal");
                break;
            case kCWSecurityDynamicWEP:
                ffStrbufSetStatic(&item->conn.security, "Dynamic WEP");
                break;
            case kCWSecurityWPAEnterprise:
                ffStrbufSetStatic(&item->conn.security, "WPA Enterprise");
                break;
            case kCWSecurityWPAEnterpriseMixed:
                ffStrbufSetStatic(&item->conn.security, "WPA Enterprise Mixed");
                break;
            case kCWSecurityWPA2Enterprise:
                ffStrbufSetStatic(&item->conn.security, "WPA2 Enterprise");
                break;
            case kCWSecurityEnterprise:
                ffStrbufSetStatic(&item->conn.security, "Enterprise");
                break;
            case kCWSecurityWPA3Personal:
                ffStrbufSetStatic(&item->conn.security, "WPA3 Personal");
                break;
            case kCWSecurityWPA3Enterprise:
                ffStrbufSetStatic(&item->conn.security, "WPA3 Enterprise");
                break;
            case kCWSecurityWPA3Transition:
                ffStrbufSetStatic(&item->conn.security, "WPA3 Transition");
                break;
            case 14 /*kCWSecurityOWE*/:
                ffStrbufSetStatic(&item->conn.security, "OWE");
                break;
            case 15 /*kCWSecurityOWETransition*/:
                ffStrbufSetStatic(&item->conn.security, "OWE Transition");
                break;
            case kCWSecurityUnknown:
                // Sonoma...
                {
                    if (!ffProcessAppendStdOut(&item->conn.security, (char* []) {
                        "/System/Library/PrivateFrameworks/Apple80211.framework/Resources/airport",
                        "-I",
                        NULL
                    }))
                    {
                        {
                            uint32_t ssidIndex = ffStrbufFirstIndexS(&item->conn.security, " SSID: ");
                            if (ssidIndex == item->conn.security.length) break;
                            ssidIndex += (uint32_t) strlen(" SSID: ");
                            uint32_t ssidEndIndex = ffStrbufNextIndexC(&item->conn.security, ssidIndex, '\n');
                            if (item->conn.ssid.length != ssidEndIndex - ssidIndex) break;
                        }

                        uint32_t linkAuthIndex = ffStrbufFirstIndexS(&item->conn.security, "  link auth: ");
                        if (linkAuthIndex == item->conn.security.length) break;
                        linkAuthIndex += (uint32_t) strlen("  link auth: ");
                        uint32_t linkAuthEndIndex = ffStrbufNextIndexC(&item->conn.security, linkAuthIndex, '\n');
                        ffStrbufSubstrBefore(&item->conn.security, linkAuthEndIndex);
                        ffStrbufSubstrAfter(&item->conn.security, linkAuthIndex - 1);
                        ffStrbufUpperCase(&item->conn.security);
                    }
                }
                break;
            default:
                ffStrbufAppendF(&item->conn.security, "Unknown (%ld)", inf.security);
                break;
        }
    }
    return NULL;
}
