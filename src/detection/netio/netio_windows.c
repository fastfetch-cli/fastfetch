#include "netio.h"

#include "common/netif/netif.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"

#include <ws2tcpip.h>
#include <iphlpapi.h>

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options)
{
    IP_ADAPTER_ADDRESSES* FF_AUTO_FREE adapter_addresses = NULL;

    // Multiple attempts in case interfaces change while
    // we are in the middle of querying them.
    DWORD adapter_addresses_buffer_size = 0;
    for (int attempts = 0;; ++attempts)
    {
        if (adapter_addresses_buffer_size)
        {
            adapter_addresses = (IP_ADAPTER_ADDRESSES*)realloc(adapter_addresses, adapter_addresses_buffer_size);
            assert(adapter_addresses);
        }

        DWORD error = GetAdaptersAddresses(
            AF_UNSPEC,
            GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
            NULL,
            adapter_addresses,
            &adapter_addresses_buffer_size);

        if (error == ERROR_SUCCESS)
            break;
        else if (ERROR_BUFFER_OVERFLOW == error && attempts < 4)
            continue;
        else
            return "GetAdaptersAddresses() failed";
    }

    uint32_t defaultRouteIfIndex = ffNetifGetDefaultRouteIfIndex();

    // Iterate through all of the adapters
    for (IP_ADAPTER_ADDRESSES* adapter = adapter_addresses; adapter; adapter = adapter->Next)
    {
        bool isDefaultRoute = adapter->IfIndex == defaultRouteIfIndex;
        if (options->defaultRouteOnly && !isDefaultRoute)
            continue;

        char name[128];
        WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1, name, ARRAY_SIZE(name), NULL, NULL);
        if (options->namePrefix.length && strncmp(name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        MIB_IF_ROW2 ifRow = { .InterfaceIndex = adapter->IfIndex };
        if (GetIfEntry2(&ifRow) == NO_ERROR)
        {
            FFNetIOResult* counters = (FFNetIOResult*) ffListAdd(result);
            *counters = (FFNetIOResult) {
                .name = ffStrbufCreateS(name),
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
    }

    return NULL;
}
