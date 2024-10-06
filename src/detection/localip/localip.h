#pragma once

#include "fastfetch.h"

typedef struct FFLocalIpResult
{
    FFstrbuf name;
    FFstrbuf ipv4;
    FFstrbuf ipv6;
    FFstrbuf mac;
    FFstrbuf flags;
    int32_t mtu;
    int32_t speed;
    bool defaultRoute;
} FFLocalIpResult;

typedef struct FFLocalIpNIFlag
{
    uint32_t flag;
    const char *name;
} FFLocalIpNIFlag;

static inline void ffLocalIpFillNIFlags(FFstrbuf *buf, uint32_t flag, const FFLocalIpNIFlag names[])
{
    for (const FFLocalIpNIFlag *nf = names; flag && nf->name; ++nf)
    {
        if (flag & nf->flag)
        {
            if (buf->length > 0)
                ffStrbufAppendC(buf, ',');
            ffStrbufAppendS(buf, nf->name);
            flag &= ~nf->flag;
        }
    }
}

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results);
