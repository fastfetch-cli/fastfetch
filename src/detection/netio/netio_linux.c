#include "netio.h"

#include "common/io/io.h"
#include "common/netif/netif.h"
#include "util/stringUtils.h"

#include <net/if.h>

static void getData(FFstrbuf* buffer, const char* ifName, bool isDefaultRoute, FFstrbuf* path, FFlist* result)
{
    ffStrbufSetF(path, "/sys/class/net/%s/operstate", ifName);
    if(!ffReadFileBuffer(path->chars, buffer) || !ffStrbufEqualS(buffer, "up\n"))
        return;

    FFNetIOResult* counters = (FFNetIOResult*) ffListAdd(result);
    ffStrbufInitS(&counters->name, ifName);
    counters->defaultRoute = isDefaultRoute;

    ffStrbufSetF(path, "/sys/class/net/%s/statistics/", ifName);
    uint32_t statLen = path->length;

    ffStrbufAppendS(path, "rx_bytes");
    if (ffReadFileBuffer(path->chars, buffer))
        counters->rxBytes = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, statLen);

    ffStrbufAppendS(path, "tx_bytes");
    if (ffReadFileBuffer(path->chars, buffer))
        counters->txBytes = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, statLen);

    ffStrbufAppendS(path, "rx_packets");
    if (ffReadFileBuffer(path->chars, buffer))
        counters->rxPackets = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, statLen);

    ffStrbufAppendS(path, "tx_packets");
    if (ffReadFileBuffer(path->chars, buffer))
        counters->txPackets = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, statLen);

    ffStrbufAppendS(path, "rx_errors");
    if (ffReadFileBuffer(path->chars, buffer))
        counters->rxErrors = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, statLen);

    ffStrbufAppendS(path, "tx_errors");
    if (ffReadFileBuffer(path->chars, buffer))
        counters->txErrors = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, statLen);

    ffStrbufAppendS(path, "rx_dropped");
    if (ffReadFileBuffer(path->chars, buffer))
        counters->rxDrops = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, statLen);

    ffStrbufAppendS(path, "tx_dropped");
    if (ffReadFileBuffer(path->chars, buffer))
        counters->txDrops = ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(path, statLen);
}

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/net");
    if (!dirp) return "opendir(\"/sys/class/net\") == NULL";

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateA(64);
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    const char* defaultRouteIfName = ffNetifGetDefaultRouteIfName();

    if (options->defaultRouteOnly)
    {
        if (options->namePrefix.length && strncmp(defaultRouteIfName, options->namePrefix.chars, options->namePrefix.length) != 0)
            return NULL;

       getData(&buffer, defaultRouteIfName, true, &path, result);
    }
    else
    {
        struct dirent* entry;
        while((entry = readdir(dirp)) != NULL)
        {
            const char* ifName = entry->d_name;
            if(ifName[0] == '.')
                continue;

            if (options->namePrefix.length && strncmp(ifName, options->namePrefix.chars, options->namePrefix.length) != 0)
                continue;

            getData(&buffer, ifName, ffStrEquals(ifName, defaultRouteIfName), &path, result);
        }
    }

    return NULL;
}
