#pragma once

#include "fastfetch.h"
#include "modules/localip/option.h"

#ifndef IN6_IS_ADDR_GLOBAL
/* Global Unicast: 2000::/3 (001...) */
#    define IN6_IS_ADDR_GLOBAL(a) (((a)->s6_addr[0] & 0xE0) == 0x20)
#endif
#ifndef IN6_IS_ADDR_UNIQUE_LOCAL
/* Unique Local: fc00::/7 (1111 110...) */
#    define IN6_IS_ADDR_UNIQUE_LOCAL(a) (((a)->s6_addr[0] & 0xFE) == 0xFC)
#endif
#ifndef IN6_IS_ADDR_LINKLOCAL
/* Link-Local: fe80::/10 (1111 1110 10...) */
#    define IN6_IS_ADDR_LINKLOCAL(a) (((a)->s6_addr[0] == 0xFE) && (((a)->s6_addr[1] & 0xC0) == 0x80))
#endif

typedef struct FFLocalIpResult {
    FFstrbuf name;
    FFstrbuf ipv4;
    FFstrbuf ipv6;
    FFstrbuf mac;
    FFstrbuf flags;
    int32_t mtu;
    int32_t speed; // in Mbps
    FFLocalIpType defaultRoute;
} FFLocalIpResult;

typedef struct FFLocalIpNIFlag {
    uint32_t flag;
    const char* name;
} FFLocalIpNIFlag;

static inline void ffLocalIpFillNIFlags(FFstrbuf* buf, uint64_t flag, const FFLocalIpNIFlag names[]) {
    for (const FFLocalIpNIFlag* nf = names; flag && nf->name; ++nf) {
        if (flag & nf->flag) {
            if (buf->length > 0) {
                ffStrbufAppendC(buf, ',');
            }
            ffStrbufAppendS(buf, nf->name);
            flag &= ~nf->flag;
        }
    }
}

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results);
