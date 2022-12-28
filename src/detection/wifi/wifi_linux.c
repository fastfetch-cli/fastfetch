#include "wifi.h"

#include "common/io.h"
#include <linux/nl80211.h>
#include <net/if.h>

static const char* detectInf(uint32_t ifIndex, FFWifiResult* wifi)
{
    //TODO: play with netlink
    FF_UNUSED(ifIndex, wifi);
    return "Unimplemented";
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

        ffStrbufSetF(&path, "/sys/class/net/%s/address", i->if_name);
        ffAppendFileBuffer(path.chars, &item->conn.macAddress);

        detectInf(i->if_index, item);
    }
    if_freenameindex(infs);
    ffStrbufDestroy(&path);

    if(result->length == 0)
        return "No wifi interfaces found";

    return NULL;
}
