#include "wifi.h"
#include "common/processing.h"
#include "common/properties.h"
#include "util/stringUtils.h"

#include <net/if.h>
#include <stdio.h>
#include <string.h>

const char* ffDetectWifi(FFlist* result)
{
    struct if_nameindex* infs = if_nameindex();
    if(!infs)
        return "if_nameindex() failed";

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        if (!ffStrStartsWith(i->if_name, "wlan")) continue;
        FF_STRBUF_AUTO_DESTROY ifconfig = ffStrbufCreate();
        if (ffProcessAppendStdOut(&ifconfig, (char* const[]) {
            "ifconfig",
            i->if_name,
            NULL
        }) == NULL)
        {
            FFWifiResult* item = (FFWifiResult*) ffListAdd(result);
            ffStrbufInitS(&item->inf.description, i->if_name);
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

            ffParsePropLines(ifconfig.chars, "status: ", &item->conn.status);
            if (!ffStrbufEqualS(&item->conn.status, "associated"))
                continue;

            ffParsePropLines(ifconfig.chars, "ssid ", &item->conn.ssid);
            if (item->conn.ssid.length)
            {
                // This doesn't work for quoted SSID values
                uint32_t idx = ffStrbufFirstIndexS(&item->conn.ssid, " bssid ");
                if (idx < item->conn.ssid.length)
                {
                    ffStrbufSetS(&item->conn.bssid, item->conn.ssid.chars + idx + (uint32_t) strlen(" bssid "));
                    ffStrbufSubstrBefore(&item->conn.ssid, idx);
                }

                idx = ffStrbufFirstIndexS(&item->conn.ssid, " channel ");
                if (idx < item->conn.ssid.length)
                {
                    const char* pchannel = item->conn.ssid.chars + idx + strlen(" channel ");
                    sscanf(pchannel, "%hu (%hu MHz %*s)", &item->conn.channel, &item->conn.frequency);
                }

                ffStrbufSubstrBefore(&item->conn.ssid, idx);
            }

            ffParsePropLines(ifconfig.chars, "media: ", &item->conn.protocol);
            if (item->conn.protocol.length)
            {
                uint32_t index = ffStrbufFirstIndexS(&item->conn.protocol, " mode ");
                if (index == item->conn.protocol.length)
                    ffStrbufClear(&item->conn.protocol);
                else
                {
                    ffStrbufSubstrAfter(&item->conn.protocol, index + strlen(" mode ") - 1);
                    ffStrbufPrependS(&item->conn.protocol, "802.");
                }
            }
        }
    }

    return NULL;
}
