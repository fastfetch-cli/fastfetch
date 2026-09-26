#include "netio.h"

#include "common/netif.h"
#include "common/windows/unicode.h"

#include <ws2tcpip.h>
#include <iphlpapi.h>

static void addInterface(FFlist* result, const FFNetIOOptions* options, NET_IFINDEX ifIndex, bool isDefaultRoute) {
    MIB_IF_ROW2 ifRow = { .InterfaceIndex = ifIndex };
    if (!NETIO_SUCCESS(GetIfEntry2(&ifRow))) {
        return;
    }

    // localip reports the interfaces that are operational only, so the same filter is applied here
    if (ifRow.OperStatus != IfOperStatusUp) {
        return;
    }

    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateWS(ifRow.Alias);
    if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix)) {
        return;
    }

    FFNetIOResult* counters = FF_LIST_ADD(FFNetIOResult, *result);
    *counters = (FFNetIOResult) {
        .name = ffStrbufCreateMove(&name),
        .txBytes = ifRow.OutOctets,
        .rxBytes = ifRow.InOctets,
        .txPackets = (ifRow.OutUcastPkts + ifRow.OutNUcastPkts),
        .rxPackets = (ifRow.InUcastPkts + ifRow.InNUcastPkts),
        .rxErrors = ifRow.InErrors,
        .txErrors = ifRow.OutErrors,
        .rxDrops = ifRow.InDiscards,
        .txDrops = ifRow.OutDiscards,
        .defaultRoute = isDefaultRoute,
    };
}

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options) {
    uint32_t defaultRouteIfIndex = ffNetifGetDefaultRouteV4()->ifIndex;

    // With `defaultRouteOnly` (the default) the interface carrying the default route is the only one
    // that can be reported, so it is queried with the single entry API instead of walking a table of
    // every interface: GetIfEntry2() on one interface costs about 12 us.
    if (options->defaultRouteOnly) {
        addInterface(result, options, (NET_IFINDEX) defaultRouteIfIndex, true);
        return nullptr;
    }

    // The interface set is taken from the unicast address table, the same source localip uses, so
    // both modules report the same interfaces. Walking the interface table instead would also report
    // the NDIS filter layers of every adapter (Npcap, QoS, WFP, ...) and the adapters that carry no
    // address at all, and those cannot be told apart from real adapters by any MIB_IF_ROW2 field.
    // GetUnicastIpAddressTable() costs about 72 us, GetIfTable2() about 860 us.
    PMIB_UNICASTIPADDRESS_TABLE addressTable = nullptr;
    if (!NETIO_SUCCESS(GetUnicastIpAddressTable(AF_UNSPEC, &addressTable))) {
        return "GetUnicastIpAddressTable() failed";
    }

    for (ULONG i = 0; i < addressTable->NumEntries; ++i) {
        NET_IFINDEX ifIndex = addressTable->Table[i].InterfaceIndex;

        // Rows of the same interface are not adjacent (IPv4 rows come first),
        // so each interface is handled at its first row
        bool seen = false;
        for (ULONG j = 0; j < i && !seen; ++j) {
            seen = addressTable->Table[j].InterfaceIndex == ifIndex;
        }
        if (seen) {
            continue;
        }

        addInterface(result, options, ifIndex, ifIndex == defaultRouteIfIndex);
    }
    FreeMibTable(addressTable);

    return nullptr;
}
