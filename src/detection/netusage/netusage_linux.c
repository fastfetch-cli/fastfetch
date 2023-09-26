#include "netusage.h"

#include "common/io/io.h"

#include <net/if.h>

const char* ffGetNetIoCounter(FFlist* result)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/net");
    if (!dirp) return "opendir(\"/sys/class/net\") == NULL";

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateA(64);
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufSetF(&path, "/sys/class/net/%s/operstate", entry->d_name);
        if(!ffReadFileBuffer(path.chars, &buffer) || !ffStrbufEqualS(&buffer, "up"))
            continue;

        FFNetUsageIoCounters* counters = (FFNetUsageIoCounters*) ffListAdd(result);
        ffStrbufInitS(&counters->name, entry->d_name);

        ffStrbufSetF(&path, "/sys/class/net/%s/statistics/");
        uint32_t statLen = path.length;

        ffStrbufAppendS(&path, "rx_bytes");
        if (ffReadFileBuffer(path.chars, &buffer))
            counters->rxBytes = ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, statLen);

        ffStrbufAppendS(&path, "tx_bytes");
        if (ffReadFileBuffer(path.chars, &buffer))
            counters->txBytes = ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, statLen);

        ffStrbufAppendS(&path, "rx_packets");
        if (ffReadFileBuffer(path.chars, &buffer))
            counters->rxPackets = ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, statLen);

        ffStrbufAppendS(&path, "tx_packets");
        if (ffReadFileBuffer(path.chars, &buffer))
            counters->txPackets = ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, statLen);

        ffStrbufAppendS(&path, "rx_errors");
        if (ffReadFileBuffer(path.chars, &buffer))
            counters->rxErrors = ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, statLen);

        ffStrbufAppendS(&path, "tx_errors");
        if (ffReadFileBuffer(path.chars, &buffer))
            counters->txErrors = ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, statLen);

        ffStrbufAppendS(&path, "rx_dropped");
        if (ffReadFileBuffer(path.chars, &buffer))
            counters->rxDrops = ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, statLen);

        ffStrbufAppendS(&path, "tx_dropped");
        if (ffReadFileBuffer(path.chars, &buffer))
            counters->txDrops = ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrBefore(&path, statLen);
    }

    return NULL;
}
