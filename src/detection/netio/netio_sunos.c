#include "netio.h"
#include "common/netif/netif.h"
#include "util/stringUtils.h"
#include <kstat.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc)
{
    assert(pkc);
    if (*pkc)
        kstat_close(*pkc);
}

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options)
{
    __attribute__((__cleanup__(kstatFreeWrap))) kstat_ctl_t* kc = kstat_open();
    if (!kc)
        return "kstat_open() failed";

    const char* defaultRoute = ffNetifGetDefaultRouteIfName();

    for (kstat_t* ks = kc->kc_chain; ks; ks = ks->ks_next)
    {
        if (!ffStrEquals(ks->ks_class, "net") || !ffStrEquals(ks->ks_module, "link")) continue;

        if (options->namePrefix.length && strncmp(ks->ks_name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        bool isDefaultRoute = ffStrEquals(ks->ks_name, defaultRoute);
        if (options->defaultRouteOnly && !isDefaultRoute)
            continue;

        if (kstat_read(kc, ks, NULL) < 0)
            continue;

        FFNetIOResult* counters = (FFNetIOResult*) ffListAdd(result);

        kstat_named_t* wbytes = (kstat_named_t*) kstat_data_lookup(ks, "obytes64");
        kstat_named_t* rbytes = (kstat_named_t*) kstat_data_lookup(ks, "rbytes64");
        kstat_named_t* wpkts = (kstat_named_t*) kstat_data_lookup(ks, "opackets64");
        kstat_named_t* rpkts = (kstat_named_t*) kstat_data_lookup(ks, "ipackets64");
        kstat_named_t* werrs = (kstat_named_t*) kstat_data_lookup(ks, "oerrors");
        kstat_named_t* rerrs = (kstat_named_t*) kstat_data_lookup(ks, "ierrors");

        *counters = (FFNetIOResult) {
            .name = ffStrbufCreateS(ks->ks_name),
            .txBytes = wbytes->value.ui64,
            .rxBytes = rbytes->value.ui64,
            .txPackets = wpkts->value.ui64,
            .rxPackets = rpkts->value.ui64,
            .txErrors = werrs->value.ui64,
            .rxErrors = rerrs->value.ui64,
            .txDrops = 0, // unsupported
            .rxDrops = 0,
            .defaultRoute = isDefaultRoute,
        };

        if (options->defaultRouteOnly)
            break;
    }

    return NULL;
}
