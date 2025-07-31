#include "wifi.h"
#include "common/processing.h"
#include "util/stringUtils.h"

#import <CoreWLAN/CoreWLAN.h>

static inline double rssiToSignalQuality(int rssi)
{
    return (double) (rssi >= -50 ? 100 : rssi <= -100 ? 0 : (rssi + 100) * 2);
}

static bool queryIpconfig(const char* ifName, FFstrbuf* result)
{
    if (@available(macOS 26.0, *))
    {
        // ipconfig no longer work in Tahoe
        return false;
    }

    return ffProcessAppendStdOut(result, (char* const[]) {
        "/usr/sbin/ipconfig",
        "getsummary",
        (char* const) ifName,
        NULL
    }) == NULL;
}

static bool getWifiInfoByIpconfig(FFstrbuf* ipconfig, const char* prefix, FFstrbuf* result)
{
    // `ipconfig getsummary <interface>` returns a string like this:
    // <dictionary> {
    //   BSSID : <redacted>
    //   IPv4 : <array> {
    //   ...
    //   }
    //   IPv6 : <array> {
    //   ...
    //   }
    //   InterfaceType : WiFi
    //   LinkStatusActive : TRUE
    //   NetworkID : XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    //   SSID : XXXXXX
    //   Security : WPA2_PSK
    // }

    const char* start = memmem(ipconfig->chars, ipconfig->length, prefix, strlen(prefix));
    if (!start) return false;
    start += strlen(prefix);
    const char* end = strchr(start, '\n');
    if (!end) return false;
    ffStrbufSetNS(result, (uint32_t) (end - start), start);
    return true;
}

static const char* detectByWdutil(FFlist* result)
{
    FF_STRBUF_AUTO_DESTROY wdutil = ffStrbufCreate();

    if (geteuid() != 0)
        return "wdutil requires root privileges to run";

    bool ok = ffProcessAppendStdOut(&wdutil, (char* const[]) {
        "/usr/bin/wdutil",
        "info",
        NULL
    }) == NULL;
    if (!ok) return "Failed to run wdutil info command";

    // ...
    // ————————————————————————————————————————————————————————————————————
    // WIFI
    // ————————————————————————————————————————————————————————————————————
    //     <WIFI INFO>
    // ————————————————————————————————————————————————————————————————————
    // ...

    {
        // Remove unrelated lines
        uint32_t start = ffStrbufFirstIndexS(&wdutil, "\nWIFI\n");
        if (start >= wdutil.length)
            return "wdutil info command did not return WIFI section (1)";

        start += 6; // Skip "\nWIFI\n"
        start = ffStrbufNextIndexC(&wdutil, start, '\n');
        if (start >= wdutil.length)
            return "wdutil info command did not return WIFI section (2)";
        start++;

        uint32_t end = ffStrbufNextIndexS(&wdutil, start, "\n——————————");

        ffStrbufSubstr(&wdutil, start, end);
    }


    // `wdutil info <interface>` returns a string like this:
    //     MAC Address          : xx:xx:xx:xx:xx:xx (hw=xx:xx:xx:xx:xx:xx)
    //     Interface Name       : en0
    //     Power                : On [On]
    //     Op Mode              : STA
    //     SSID                 : XXX-XXX
    //     BSSID                : xx:xx:xx:xx:xx:xx
    //     RSSI                 : -58 dBm
    //     CCA                  : 29 %
    //     Noise                : -96 dBm
    //     Tx Rate              : 173.0 Mbps
    //     Security             : WPA2 Enterprise
    //     802.1X Mode          : User
    //     802.1X Supplicant    : Authenticated
    //     PHY Mode             : 11ac
    //     MCS Index            : 8
    //     Guard Interval       : 400
    //     NSS                  : 2
    //     Channel              : 5g165/20
    //     Country Code         : CN
    //     Scan Cache Count     : 28
    //     NetworkServiceID     : XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    //     IPv4 Config Method   : DHCP
    //     IPv4 Address         : xx.xx.xx.xx
    //     IPv4 Router          : xx.xx.xx.x
    //     IPv6 Config Method   : Automatic
    //     IPv6 Address         : xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
    //     IPv6 Router          : None
    //     DNS                  : xxx.xxx.xxx.xxx
    //                          : xxx.xxx.xxx.xxx
    //     BTC Mode             : Off
    //     Desense              :
    //     Chain Ack            : []
    //     BTC Profile 2.4GHz   : Disabled
    //     BTC Profile 5GHz     : Disabled
    //     Sniffer Supported    : YES
    //     Supports 6e          : No
    //     Supported Channels   : 2g1/20,2g2/20,2g3/20,2g4/20,2g5/20,2g6/20,2g7/20,2g8/20,2g9/20,2g10/20,2g11/20,2g12/20,2g13/20,5g36/20,5g40/20,5g44/20,5g48/20,5g52/20,5g56/20,5g60/20,5g64/20,5g149/20,5g153/20,5g157/20,5g161/20,5g165/20,5g36/40,5g40/40,5g44/40,5g48/40,5g52/40,5g56/40,5g60/40,5g64/40,5g149/40,5g153/40,5g157/40,5g161/40,5g36/80,5g40/80,5g44/80,5g48/80,5g52/80,5g56/80,5g60/80,5g64/80,5g149/80,5g153/80,5g157/80,5g161/80

    FFWifiResult* item = (FFWifiResult*) ffListAdd(result);
    ffStrbufInit(&item->inf.description);
    ffStrbufInit(&item->inf.status);
    ffStrbufInit(&item->conn.status);
    ffStrbufInit(&item->conn.ssid);
    ffStrbufInit(&item->conn.bssid);
    ffStrbufInit(&item->conn.protocol);
    ffStrbufInit(&item->conn.security);
    item->conn.signalQuality = 0.0/0.0;
    item->conn.rxRate = 0.0/0.0;
    item->conn.txRate = 0.0/0.0;
    item->conn.channel = 0;
    item->conn.frequency = 0;

    char* line = NULL;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, &wdutil))
    {
        const char* key = line + 4; // Skip "    "
        const char* value = key + strlen("MAC Address          : ");
        switch (key[0] << 24 | key[1] << 16 | key[2] << 8 | key[3])
        {
            case 'Inte': // Interface Name
                ffStrbufAppendS(&item->inf.description, value);
                break;
            case 'Powe': // Power
                if (ffStrStartsWith(value, "On "))
                    ffStrbufSetStatic(&item->inf.status, "Power On");
                else
                    ffStrbufSetStatic(&item->inf.status, "Power Off");
                break;
            case 'SSID': // SSID
                ffStrbufAppendS(&item->conn.ssid, value);
                break;
            case 'BSSI': // BSSID
                if (ffStrEquals(value, "None") && ffStrbufEqualS(&item->conn.ssid, "None"))
                {
                    ffStrbufSetStatic(&item->conn.status, "Inactive");
                    ffStrbufClear(&item->conn.ssid); // None
                    return NULL;
                }
                ffStrbufSetStatic(&item->conn.status, "Active");
                ffStrbufAppendS(&item->conn.bssid, value);
                break;
            case 'RSSI': // RSSI
                item->conn.signalQuality = rssiToSignalQuality((int) strtol(value, NULL, 10));
                break;
            case 'Tx R': // Tx Rate
                item->conn.txRate = strtod(value, NULL);
                break;
            case 'Secu': // Security
                if (ffStrEquals(value, "None"))
                    ffStrbufSetStatic(&item->conn.security, "Insecure");
                else
                    ffStrbufAppendS(&item->conn.security, value);
                break;
            case 'PHY ': // PHY Mode
                if (ffStrEquals(value, "None"))
                    ffStrbufSetStatic(&item->conn.protocol, "none");
                else if (ffStrEquals(value, "11a"))
                    ffStrbufSetStatic(&item->conn.protocol, "802.11a");
                else if (ffStrEquals(value, "11b"))
                    ffStrbufSetStatic(&item->conn.protocol, "802.11b");
                else if (ffStrEquals(value, "11g"))
                    ffStrbufSetStatic(&item->conn.protocol, "802.11g");
                else if (ffStrEquals(value, "11n"))
                    ffStrbufSetStatic(&item->conn.protocol, "802.11n (Wi-Fi 4)");
                else if (ffStrEquals(value, "11ac"))
                    ffStrbufSetStatic(&item->conn.protocol, "802.11ac (Wi-Fi 5)");
                else if (ffStrEquals(value, "11ax"))
                    ffStrbufSetStatic(&item->conn.protocol, "802.11ax (Wi-Fi 6)");
                else if (ffStrEquals(value, "11be"))
                    ffStrbufSetStatic(&item->conn.protocol, "802.11be (Wi-Fi 7)");
                else
                    ffStrbufAppendS(&item->conn.protocol, value);
                break;
            case 'Chan': // Channel
            {
                int band, channel;
                if (sscanf(value, "%dg%d", &band, &channel) == 2)
                {
                    item->conn.channel = (uint16_t) channel;
                    switch (band)
                    {
                        case 2: item->conn.frequency = 2400; break;
                        case 5: item->conn.frequency = 5400; break;
                        case 6: item->conn.frequency = 6400; break;
                        default: item->conn.frequency = 0; break;
                    }
                }
                break;
            }
        }
    }

    return NULL;
}

static const char* detectByCoreWlan(FFlist* result)
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
        item->conn.signalQuality = 0.0/0.0;
        item->conn.rxRate = 0.0/0.0;
        item->conn.txRate = 0.0/0.0;
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

        if (inf.ssid) // https://developer.apple.com/forums/thread/732431
            ffStrbufAppendS(&item->conn.ssid, inf.ssid.UTF8String);
        else if (ipconfig.length || (queryIpconfig(item->inf.description.chars, &ipconfig)))
            getWifiInfoByIpconfig(&ipconfig, "\n  SSID : ", &item->conn.ssid);
        else
            ffStrbufSetStatic(&item->conn.ssid, "<redacted>"); // https://developer.apple.com/forums/thread/732431

        if (inf.bssid)
            ffStrbufAppendS(&item->conn.bssid, inf.bssid.UTF8String);
        else if (ipconfig.length || (queryIpconfig(item->inf.description.chars, &ipconfig)))
            getWifiInfoByIpconfig(&ipconfig, "\n  BSSID : ", &item->conn.bssid);
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
            case kCWSecurityUnknown:
                // Sonoma?
                if (ipconfig.length || (queryIpconfig(item->inf.description.chars, &ipconfig)))
                    getWifiInfoByIpconfig(&ipconfig, "\n  Security : ", &item->conn.security);
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

const char* ffDetectWifi(FFlist* result)
{
    if (@available(macOS 26.0, *))
    {
        if (detectByWdutil(result) == NULL)
            return NULL;
    }

    return detectByCoreWlan(result);
}
