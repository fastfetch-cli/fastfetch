#include "netif.h"
#include "util/mallocHelper.h"

#include <ws2tcpip.h> // AF_INET6, IN6_IS_ADDR_UNSPECIFIED
#include <iphlpapi.h>

bool ffNetifGetDefaultRouteImpl(FF_MAYBE_UNUSED char iface[IF_NAMESIZE + 1], uint32_t* ifIndex)
{
    PMIB_IPFORWARD_TABLE2 pIpForwardTable = NULL;
    DWORD result = GetIpForwardTable2(AF_UNSPEC, &pIpForwardTable);

    if (result != NO_ERROR)
        return false;

    bool foundDefault = false;

    for (ULONG i = 0; i < pIpForwardTable->NumEntries; ++i)
    {
        MIB_IPFORWARD_ROW2* row = &pIpForwardTable->Table[i];

        if ((row->DestinationPrefix.PrefixLength == 0) &&
            ((row->DestinationPrefix.Prefix.Ipv4.sin_family == AF_INET &&
              row->DestinationPrefix.Prefix.Ipv4.sin_addr.S_un.S_addr == 0) ||
             (row->DestinationPrefix.Prefix.Ipv6.sin6_family == AF_INET6 &&
              IN6_IS_ADDR_UNSPECIFIED(&row->DestinationPrefix.Prefix.Ipv6.sin6_addr))))
        {
            *ifIndex = row->InterfaceIndex;
            foundDefault = true;
            break;
        }
    }

    FreeMibTable(pIpForwardTable);

    return foundDefault;
}
