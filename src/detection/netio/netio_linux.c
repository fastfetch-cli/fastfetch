#include "netio.h"

#include "common/io/io.h"
#include "common/netif/netif.h"
#include "util/stringUtils.h"

#include <fcntl.h>
#include <net/if.h>

static void getData(FFstrbuf* buffer, const char* ifName, bool isDefaultRoute, int basefd, FFlist* result)
{
    FF_AUTO_CLOSE_FD int dfd = openat(basefd, ifName, O_RDONLY | O_DIRECTORY);
    if (dfd < 0)
        return;

    char operstate;
    if(!ffReadFileDataRelative(dfd, "operstate", 1, &operstate) || operstate != 'u' /* up or unknown */)
        return;

    FFNetIOResult* counters = (FFNetIOResult*) ffListAdd(result);
    ffStrbufInitS(&counters->name, ifName);
    counters->defaultRoute = isDefaultRoute;

    if (ffReadFileBufferRelative(dfd, "statistics/rx_bytes", buffer))
        counters->rxBytes = ffStrbufToUInt(buffer, 0);

    if (ffReadFileBufferRelative(dfd, "statistics/tx_bytes", buffer))
        counters->txBytes = ffStrbufToUInt(buffer, 0);

    if (ffReadFileBufferRelative(dfd, "statistics/rx_packets", buffer))
        counters->rxPackets = ffStrbufToUInt(buffer, 0);

    if (ffReadFileBufferRelative(dfd, "statistics/tx_packets", buffer))
        counters->txPackets = ffStrbufToUInt(buffer, 0);

    if (ffReadFileBufferRelative(dfd, "statistics/rx_errors", buffer))
        counters->rxErrors = ffStrbufToUInt(buffer, 0);

    if (ffReadFileBufferRelative(dfd, "statistics/tx_errors", buffer))
        counters->txErrors = ffStrbufToUInt(buffer, 0);

    if (ffReadFileBufferRelative(dfd, "statistics/rx_dropped", buffer))
        counters->rxDrops = ffStrbufToUInt(buffer, 0);

    if (ffReadFileBufferRelative(dfd, "statistics/tx_dropped", buffer))
        counters->txDrops = ffStrbufToUInt(buffer, 0);
}

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/net");
    if (!dirp) return "opendir(\"/sys/class/net\") == NULL";

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    const char* defaultRouteIfName = ffNetifGetDefaultRouteV4()->ifName;

    if (options->defaultRouteOnly)
    {
        if (options->namePrefix.length && strncmp(defaultRouteIfName, options->namePrefix.chars, options->namePrefix.length) != 0)
            return NULL;

       getData(&buffer, defaultRouteIfName, true, dirfd(dirp), result);
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

            getData(&buffer, ifName, ffStrEquals(ifName, defaultRouteIfName), dirfd(dirp), result);
        }
    }

    return NULL;
}
