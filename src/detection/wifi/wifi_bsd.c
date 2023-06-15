#include "wifi.h"
#include "common/processing.h"
#include "common/properties.h"

#include <net/if.h>
#include <stdio.h>
#include <string.h>

const char* ffDetectWifi(FF_MAYBE_UNUSED const FFinstance* instance, FFlist* result)
{
    struct if_nameindex* infs = if_nameindex();
    if(!infs)
        return "if_nameindex() failed";

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        if (strncmp(i->if_name, "wlan", strlen("wlan")) != 0) continue;
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
            ffStrbufInit(&item->conn.macAddress);
            ffStrbufInit(&item->conn.protocol);
            ffStrbufInit(&item->conn.security);
            item->conn.signalQuality = 0.0/0.0;
            item->conn.rxRate = 0.0/0.0;
            item->conn.txRate = 0.0/0.0;

            ffParsePropLines(ifconfig.chars, "ssid ", &item->conn.ssid);
            if (item->conn.ssid.length) ffStrbufSubstrBeforeFirstC(&item->conn.ssid, ' ');

            ffParsePropLines(ifconfig.chars, "ether ", &item->conn.macAddress);

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

            ffParsePropLines(ifconfig.chars, "status: ", &item->conn.macAddress);
        }
    }

    return NULL;
}
