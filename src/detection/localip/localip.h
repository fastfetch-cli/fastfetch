#pragma once

#include "fastfetch.h"
#include "modules/localip/option.h"

#ifndef IN6_IS_ADDR_GLOBAL
#define IN6_IS_ADDR_GLOBAL(a) \
        ((((const uint32_t *) (a))[0] & htonl(0x70000000)) == htonl(0x20000000))
#endif
#ifndef IN6_IS_ADDR_UNIQUE_LOCAL
#define IN6_IS_ADDR_UNIQUE_LOCAL(a) \
        ((((const uint32_t *) (a))[0] & htonl(0xfe000000)) == htonl(0xfc000000))
#endif
#ifndef IN6_IS_ADDR_LINKLOCAL
#define IN6_IS_ADDR_LINKLOCAL(a) \
        ((((const uint32_t *) (a))[0] & htonl(0xffc00000)) == htonl(0xfe800000))
#endif

typedef struct FFLocalIpResult
{
    FFstrbuf name;
    FFstrbuf ipv4;
    FFstrbuf ipv6;
    FFstrbuf mac;
    FFstrbuf flags;
    int32_t mtu;
    int32_t speed; // in Mbps
    FFLocalIpType defaultRoute;
} FFLocalIpResult;

typedef struct FFLocalIpNIFlag
{
    uint32_t flag;
    const char *name;
} FFLocalIpNIFlag;

static inline void ffLocalIpFillNIFlags(FFstrbuf *buf, uint64_t flag, const FFLocalIpNIFlag names[])
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
