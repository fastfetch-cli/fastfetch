#include "wifi.h"
#include "common/processing.h"

#import <CoreWLAN/CoreWLAN.h>

struct Apple80211; // https://code.google.com/archive/p/iphone-wireless/wikis/Apple80211.wiki

// 0 is successful; < 0 is failure
int Apple80211Open(struct Apple80211 **handle) __attribute__((weak_import));
int Apple80211BindToInterface(struct Apple80211 *handle, CFStringRef interface) __attribute__((weak_import));
int Apple80211GetInfoCopy(struct Apple80211 *handle, CFDictionaryRef *info) __attribute__((weak_import));
int Apple80211Close(struct Apple80211 *handle) __attribute__((weak_import));

static NSDictionary* getWifiInfoByApple80211(NSString* ifName)
{
    if (!Apple80211Open || !Apple80211BindToInterface || !Apple80211GetInfoCopy || !Apple80211Close) return NULL;

    struct Apple80211* handle = NULL;
    if (Apple80211Open(&handle) < 0) return NULL;
    if (Apple80211BindToInterface(handle, (__bridge CFStringRef)ifName) < 0) return NULL;
    CFDictionaryRef result;
    if (Apple80211GetInfoCopy(handle, &result) < 0) return NULL;
    Apple80211Close(handle);
    return CFBridgingRelease(result);
}

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

        ffStrbufSetStatic(&item->conn.status, inf.interfaceMode != kCWInterfaceModeNone ? "Active" : "Inactive");
        if(!inf.serviceActive)
            continue;

        NSDictionary* apple = NULL;

        if (inf.ssid) // https://developer.apple.com/forums/thread/732431
            ffStrbufAppendS(&item->conn.ssid, inf.ssid.UTF8String);
        else if (apple || (apple = getWifiInfoByApple80211(inf.interfaceName)))
            ffStrbufAppendS(&item->conn.ssid, [[apple valueForKey:@"SSID_STR"] UTF8String]);
        else
            ffStrbufSetStatic(&item->conn.ssid, "<unknown ssid>"); // https://developer.apple.com/forums/thread/732431

        if (inf.bssid)
            ffStrbufAppendS(&item->conn.macAddress, inf.bssid.UTF8String);
        else if (apple || (apple = getWifiInfoByApple80211(inf.interfaceName)))
            ffStrbufAppendS(&item->conn.macAddress, [[apple valueForKey:@"BSSID"] UTF8String]);

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
            case 11 /*kCWSecurityWPA3Personal*/:
                ffStrbufSetStatic(&item->conn.security, "WPA3 Personal");
                break;
            case 12 /*kCWSecurityWPA3Enterprise*/:
                ffStrbufSetStatic(&item->conn.security, "WPA3 Enterprise");
                break;
            case 13 /*kCWSecurityWPA3Transition*/:
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
                if (apple || (apple = getWifiInfoByApple80211(inf.interfaceName)))
                {
                    NSDictionary* authType = [apple valueForKey:@"AUTH_TYPE"];
                    if (authType)
                    {
                        // AUTH_LOWER seems useless. `airport` verifies if its value is between 1 and 3, and prints `unknown` if not

                        NSNumber* authUpper = [authType valueForKey:@"AUTH_UPPER"];
                        if (!authUpper)
                            ffStrbufSetStatic(&item->conn.security, "Insecure");
                        else
                        {
                            int authUpperValue = [authUpper intValue];
                            switch (authUpperValue)
                            {
                                case 1:
                                    ffStrbufSetStatic(&item->conn.security, "WPA");
                                    break;
                                case 2:
                                    ffStrbufSetStatic(&item->conn.security, "WPA-PSK");
                                    break;
                                case 4:
                                    ffStrbufSetStatic(&item->conn.security, "WPA2");
                                    break;
                                case 8:
                                    ffStrbufSetStatic(&item->conn.security, "WPA2-PSK");
                                    break;
                                case 16:
                                    ffStrbufSetStatic(&item->conn.security, "FT-WPA2-PSK");
                                    break;
                                case 32:
                                    ffStrbufSetStatic(&item->conn.security, "LEAP");
                                    break;
                                case 64:
                                    ffStrbufSetStatic(&item->conn.security, "802.1X");
                                    break;
                                case 128:
                                    ffStrbufSetStatic(&item->conn.security, "FT-WPA2");
                                    break;
                                case 256:
                                    ffStrbufSetStatic(&item->conn.security, "WPS");
                                    break;
                                case 4096:
                                    ffStrbufSetStatic(&item->conn.security, "WPA3-SAE");
                                    break;
                                case 8192:
                                    ffStrbufSetStatic(&item->conn.security, "WPA3-FT-SAE");
                                    break;
                                default: // TODO: support more auth types
                                    ffStrbufAppendF(&item->conn.security, "To be supported (%d)", authUpperValue);
                                    break;
                            }
                        }
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
