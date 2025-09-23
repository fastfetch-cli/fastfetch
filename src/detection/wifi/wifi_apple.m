#include "wifi.h"
#include "common/processing.h"
#include "util/stringUtils.h"

#import <CoreWLAN/CoreWLAN.h>

static inline double rssiToSignalQuality(int rssi)
{
    return (double) (rssi >= -50 ? 100 : rssi <= -100 ? 0 : (rssi + 100) * 2);
}

@interface CWNetworkProfile()
@property(readonly, retain, nullable) NSArray<NSDictionary *> *bssidList;
@end

const char* ffDetectWifi(FFlist* result)
{
    NSArray<CWInterface*>* interfaces = CWWiFiClient.sharedWiFiClient.interfaces;
    if (!interfaces)
        return "CWWiFiClient.sharedWiFiClient.interfaces is nil";

    for (CWInterface* inf in interfaces)
    {
        FFWifiResult* item = (FFWifiResult*) ffListAdd(result);
        ffStrbufInit(&item->inf.description);
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
        item->conn.frequency = 0;

        ffStrbufAppendS(&item->inf.description, inf.interfaceName.UTF8String);
        ffStrbufSetStatic(&item->inf.status, inf.powerOn ? "Power On" : "Power Off");
        if(!inf.powerOn)
            continue;

        ffStrbufSetStatic(&item->conn.status, inf.interfaceMode != kCWInterfaceModeNone ? "Active" : "Inactive");
        if(inf.interfaceMode == kCWInterfaceModeNone)
            continue;

        FF_STRBUF_AUTO_DESTROY ipconfig = ffStrbufCreate();

        CWNetworkProfile* networkProfile = inf.configuration.networkProfiles.firstObject;

        if (inf.ssid) // https://developer.apple.com/forums/thread/732431
            ffStrbufAppendS(&item->conn.ssid, inf.ssid.UTF8String);
        else if (networkProfile.ssid)
            ffStrbufSetStatic(&item->conn.ssid, inf.configuration.networkProfiles.firstObject.ssid.UTF8String);
        else
            ffStrbufSetStatic(&item->conn.ssid, "<redacted>"); // https://developer.apple.com/forums/thread/732431

        if (inf.bssid)
            ffStrbufAppendS(&item->conn.bssid, inf.bssid.UTF8String);
        else if (networkProfile.bssidList)
            ffStrbufSetStatic(&item->conn.bssid, [networkProfile.bssidList.firstObject[@"BSSID"] UTF8String]);
        else
            ffStrbufSetStatic(&item->conn.bssid, "<redacted>");

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
                if (inf.activePHYMode < 8)
                    ffStrbufAppendF(&item->conn.protocol, "Unknown (%ld)", inf.activePHYMode);
                break;
        }
        item->conn.signalQuality = rssiToSignalQuality((int) inf.rssiValue);
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
            default:
                ffStrbufAppendF(&item->conn.security, "Unknown (%ld)", inf.security);
                break;
        }

        item->conn.channel = (uint16_t) inf.wlanChannel.channelNumber;
        switch (inf.wlanChannel.channelBand)
        {
            case kCWChannelBand2GHz: item->conn.frequency = 2400; break;
            case kCWChannelBand5GHz: item->conn.frequency = 5400; break;
            case 3 /*kCWChannelBand6GHz*/: item->conn.frequency = 6400; break;
            default: item->conn.frequency = 0; break;
        }
    }

    return NULL;
}
