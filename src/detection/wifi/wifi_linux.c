#include "wifi.h"

#include "common/io.h"
#include "common/processing.h"
#include "common/properties.h"

#include <net/if.h>

static const char* detectInf(char* ifName, FFWifiResult* wifi)
{
    const char* error = NULL;
    FF_STRBUF_AUTO_DESTROY output;
    ffStrbufInit(&output);
    if((error = ffProcessAppendStdOut(&output, (char* const[]){
        "iw",
        "dev",
        ifName,
        "link",
        NULL
    })))
        return error;

    if(output.length == 0)
        return "iw command execution failed";

    if(!ffParsePropLines(output.chars, "Connected to ", &wifi->conn.macAddress))
    {
        ffStrbufAppendS(&wifi->conn.status, "Disconnected");
        return NULL;
    }

    FF_STRBUF_AUTO_DESTROY temp;
    ffStrbufInit(&temp);

    ffStrbufAppendS(&wifi->conn.status, "Connected");
    ffStrbufSubstrBefore(&wifi->conn.macAddress, ' ');
    ffParsePropLines(output.chars, "SSID: ", &wifi->conn.ssid);

    if(ffParsePropLines(output.chars, "signal: ", &temp))
        wifi->conn.signalQuality = ffStrbufToDouble(&temp);

    if(ffParsePropLines(output.chars, "tx bitrate: ", &temp))
    {
        if(ffStrbufContainS(&temp, " HE-MCS "))
            ffStrbufAppendS(&wifi->conn.phyType, "802.11ax (Wi-Fi 6)");
        else if(ffStrbufContainS(&temp, " VHT-MCS "))
            ffStrbufAppendS(&wifi->conn.phyType, "802.11ac (Wi-Fi 5)");
        else if(ffStrbufContainS(&temp, " MCS "))
            ffStrbufAppendS(&wifi->conn.phyType, "802.11n (Wi-Fi 4)");
        else
            ffStrbufAppendS(&wifi->conn.phyType, "802.11a/b/g");
    }

    wifi->security.enabled = true; //FIXME: Unable to find this info. Set it true so that fastfetch won't print `insecure`

    return NULL;
}

const char* ffDetectWifi(const FFinstance* instance, FFlist* result)
{
    FF_UNUSED(instance);

    struct if_nameindex* infs = if_nameindex();
    if(!infs)
        return "if_nameindex() failed";

    FFstrbuf path;
    ffStrbufInit(&path);

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        ffStrbufSetF(&path, "/sys/class/net/%s/phy80211", i->if_name);
        if(!ffFileExists(path.chars, S_IFDIR))
            continue;

        FFWifiResult* item = (FFWifiResult*)ffListAdd(result);
        ffStrbufInitS(&item->inf.description, i->if_name);
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

        ffStrbufSetF(&path, "/sys/class/net/%s/operstate", i->if_name);
        if(!ffAppendFileBuffer(path.chars, &item->inf.status) || !ffStrbufEqualS(&item->inf.status, "up"))
            continue;

        detectInf(i->if_name, item);
    }
    if_freenameindex(infs);
    ffStrbufDestroy(&path);

    if(result->length == 0)
        return "No wifi interfaces found";

    return NULL;
}
