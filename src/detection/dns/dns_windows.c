#include "detection/dns/dns.h"
#include "common/netif/netif.h"
#include "util/mallocHelper.h"

#include <ws2tcpip.h>
#include <iphlpapi.h>

const char* ffDetectDNS(FFDNSOptions* options, FFlist* results)
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
            options->showType & FF_DNS_TYPE_IPV4_BIT
                ? options->showType & FF_DNS_TYPE_IPV6_BIT ? AF_UNSPEC : AF_INET
                : AF_INET6,
            GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_FRIENDLY_NAME,
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
        if (adapter->IfIndex != defaultRouteIfIndex) continue;
        if (adapter->OperStatus != IfOperStatusUp) continue;

        for (IP_ADAPTER_DNS_SERVER_ADDRESS_XP * ifa = adapter->FirstDnsServerAddress; ifa; ifa = ifa->Next)
        {
            FFstrbuf* item = (FFstrbuf*) ffListAdd(results);
            if (ifa->Address.lpSockaddr->sa_family == AF_INET)
            {
                SOCKADDR_IN* ipv4 = (SOCKADDR_IN*) ifa->Address.lpSockaddr;
                ffStrbufInitA(item, INET_ADDRSTRLEN);
                inet_ntop(AF_INET, &ipv4->sin_addr, item->chars, item->allocated);
            }
            else if (ifa->Address.lpSockaddr->sa_family == AF_INET6)
            {
                SOCKADDR_IN6* ipv6 = (SOCKADDR_IN6*) ifa->Address.lpSockaddr;
                ffStrbufInitA(item, INET6_ADDRSTRLEN);
                inet_ntop(AF_INET6, &ipv6->sin6_addr, item->chars, item->allocated);
            }
            ffStrbufRecalculateLength(item);
        }
        break;
    }
    return NULL;
}
