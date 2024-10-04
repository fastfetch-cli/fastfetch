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

typedef struct NIFlag
{
    int flag;
    const char *name;
} NIFlag;

static inline void writeNIFlagsToStrBuf(FFstrbuf *buf, int flag, const NIFlag names[])
{
    for (const NIFlag *nf = names; nf->name != NULL && flag != 0; ++nf)
    {
        if (flag & nf->flag) {
            if (nf - names != 0)
                ffStrbufAppendC(buf, ',');
            ffStrbufAppendS(buf, nf->name);
            flag &= ~nf->flag;
        }
    }
}

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results);
