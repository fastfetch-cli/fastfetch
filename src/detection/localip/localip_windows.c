#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "common/netif/netif.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"
#include "util/debug.h"
#include "localip.h"

#define FF_LOCALIP_NIFLAG(name) { IP_ADAPTER_##name, #name }

static const FFLocalIpNIFlag niFlagOptions[] = {
    FF_LOCALIP_NIFLAG(DDNS_ENABLED),
    FF_LOCALIP_NIFLAG(REGISTER_ADAPTER_SUFFIX),
    FF_LOCALIP_NIFLAG(DHCP_ENABLED),
    FF_LOCALIP_NIFLAG(RECEIVE_ONLY),
    FF_LOCALIP_NIFLAG(NO_MULTICAST),
    FF_LOCALIP_NIFLAG(IPV6_OTHER_STATEFUL_CONFIG),
    FF_LOCALIP_NIFLAG(NETBIOS_OVER_TCPIP_ENABLED),
    FF_LOCALIP_NIFLAG(IPV4_ENABLED),
    FF_LOCALIP_NIFLAG(IPV6_ENABLED),
    FF_LOCALIP_NIFLAG(IPV6_MANAGE_ADDRESS_CONFIG),
    // sentinel
    {},
};

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results)
{
    FF_DEBUG("Starting local IP detection with showType=0x%X, namePrefix='%.*s'",
             options->showType, (int)options->namePrefix.length, options->namePrefix.chars);

    IP_ADAPTER_ADDRESSES* FF_AUTO_FREE adapter_addresses = NULL;

    // Multiple attempts in case interfaces change while
    // we are in the middle of querying them.
    DWORD adapter_addresses_buffer_size = 0;
    for (int attempts = 0;; ++attempts)
    {
        FF_DEBUG("Attempt %d to get adapter addresses, buffer size: %lu", attempts + 1, adapter_addresses_buffer_size);

        if (adapter_addresses_buffer_size)
        {
            adapter_addresses = (IP_ADAPTER_ADDRESSES*)realloc(adapter_addresses, adapter_addresses_buffer_size);
            assert(adapter_addresses);
        }

        DWORD family = options->showType & FF_LOCALIP_TYPE_IPV4_BIT
            ? options->showType & FF_LOCALIP_TYPE_IPV6_BIT ? AF_UNSPEC : AF_INET
            : AF_INET6;
        FF_DEBUG("Calling GetAdaptersAddresses with family=%u, flags=0x%X", (unsigned)family,
                 GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER);

        DWORD error = GetAdaptersAddresses(
            family,
            GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
            NULL,
            adapter_addresses,
            &adapter_addresses_buffer_size);

        if (error == ERROR_SUCCESS)
        {
            FF_DEBUG("GetAdaptersAddresses succeeded on attempt %d", attempts + 1);
            break;
        }
        else if (ERROR_BUFFER_OVERFLOW == error && attempts < 4)
        {
            FF_DEBUG("Buffer overflow, need %lu bytes, retrying", adapter_addresses_buffer_size);
            continue;
        }
        else
        {
            FF_DEBUG("GetAdaptersAddresses failed with error %lu after %d attempts", error, attempts + 1);
            return "GetAdaptersAddresses() failed";
        }
    }

    FF_MAYBE_UNUSED int adapterCount = 0, processedCount = 0;

    // Iterate through all of the adapters
    for (IP_ADAPTER_ADDRESSES* adapter = adapter_addresses; adapter; adapter = adapter->Next)
    {
        adapterCount++;

        FF_DEBUG("Processing adapter %d: IfIndex=%u, IfType=%u, OperStatus=%u",
                 adapterCount, (unsigned)adapter->IfIndex, (unsigned)adapter->IfType, (unsigned)adapter->OperStatus);

        if (adapter->OperStatus != IfOperStatusUp)
        {
            FF_DEBUG("Skipping adapter %u (not operational, status=%d)", (unsigned)adapter->IfIndex, adapter->OperStatus);
            continue;
        }

        bool isLoop = adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK;
        FF_DEBUG("Adapter %u: isLoopback=%s", (unsigned)adapter->IfIndex, isLoop ? "true" : "false");

        if (isLoop && !(options->showType & FF_LOCALIP_TYPE_LOOP_BIT))
        {
            FF_DEBUG("Skipping loopback adapter %u (loopback not requested)", (unsigned)adapter->IfIndex);
            continue;
        }

        char name[128];
        WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1, name, ARRAY_SIZE(name), NULL, NULL);
        FF_DEBUG("Adapter %u name: '%s'", (unsigned)adapter->IfIndex, name);

        if (options->namePrefix.length && strncmp(name, options->namePrefix.chars, options->namePrefix.length) != 0)
        {
            FF_DEBUG("Skipping adapter %u (name doesn't match prefix '%.*s')",
                     (unsigned)adapter->IfIndex, (int)options->namePrefix.length, options->namePrefix.chars);
            continue;
        }

        if (options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT)
        {
            if (!((options->showType & FF_LOCALIP_TYPE_IPV4_BIT) && ffNetifGetDefaultRouteV4()->ifIndex == adapter->IfIndex) &&
                !((options->showType & FF_LOCALIP_TYPE_IPV6_BIT) && ffNetifGetDefaultRouteV6()->ifIndex == adapter->IfIndex))
            {
                FF_DEBUG("Skipping interface %u (not default route interface)", (unsigned)adapter->IfIndex);
                continue;
            }
        }

        processedCount++;
        FF_DEBUG("Creating result item for adapter %u ('%s')", (unsigned)adapter->IfIndex, name);

        FFLocalIpResult* item = (FFLocalIpResult*) ffListAdd(results);
        ffStrbufInitS(&item->name, name);
        ffStrbufInit(&item->ipv4);
        ffStrbufInit(&item->ipv6);
        ffStrbufInit(&item->mac);
        ffStrbufInit(&item->flags);
        item->defaultRoute = FF_LOCALIP_TYPE_NONE;
        item->speed = -1;
        item->mtu = -1;

        uint32_t typesToAdd = options->showType & (FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_IPV6_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT);
        FF_DEBUG("Types to add for adapter %u: 0x%X", (unsigned)adapter->IfIndex, typesToAdd);

        FF_MAYBE_UNUSED int ipv4Count = 0, ipv6Count = 0;

        for (IP_ADAPTER_UNICAST_ADDRESS* ifa = adapter->FirstUnicastAddress; ifa; ifa = ifa->Next)
        {
            FF_DEBUG("Processing unicast address: prefix origin=%d, suffix origin=%d, family=%d, DadState=%d",
                     ifa->PrefixOrigin, ifa->SuffixOrigin, ifa->Address.lpSockaddr->sa_family, ifa->DadState);

            if (!(options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT))
            {
                if (ifa->DadState != IpDadStatePreferred)
                {
                    FF_DEBUG("Skipping address (not preferred)");
                    continue;
                }

                if (ifa->SuffixOrigin == IpSuffixOriginRandom)
                {
                    FF_DEBUG("Skipping temporary address (random suffix)");
                    continue;
                }

                // MIB_UNICASTIPADDRESS_ROW::SkipAsSource
            }

            if (ifa->Address.lpSockaddr->sa_family == AF_INET)
            {
                if (!(typesToAdd & (FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT)))
                {
                    FF_DEBUG("Skipping IPv4 address (not requested in typesToAdd=0x%X)", typesToAdd);
                    continue;
                }

                bool isDefaultRoute = ((options->showType & FF_LOCALIP_TYPE_IPV4_BIT) && ffNetifGetDefaultRouteV4()->ifIndex == adapter->IfIndex);
                if ((options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT) && !isDefaultRoute)
                {
                    FF_DEBUG("Skipping IPv4 address (not on default route interface)");
                    continue;
                }

                SOCKADDR_IN* ipv4 = (SOCKADDR_IN*) ifa->Address.lpSockaddr;
                char addressBuffer[INET_ADDRSTRLEN + 6];
                inet_ntop(AF_INET, &ipv4->sin_addr, addressBuffer, INET_ADDRSTRLEN);

                if ((options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT) && ifa->OnLinkPrefixLength)
                {
                    size_t len = strlen(addressBuffer);
                    snprintf(addressBuffer + len, 6, "/%u", (unsigned) ifa->OnLinkPrefixLength);
                }

                FF_DEBUG("Adding IPv4 address: %s (isDefaultRoute=%s)", addressBuffer, isDefaultRoute ? "true" : "false");

                if (item->ipv4.length) ffStrbufAppendC(&item->ipv4, ',');
                ffStrbufAppendS(&item->ipv4, addressBuffer);
                if (isDefaultRoute) item->defaultRoute |= FF_LOCALIP_TYPE_IPV4_BIT;

                ipv4Count++;
                typesToAdd &= ~(unsigned) FF_LOCALIP_TYPE_IPV4_BIT;
                if (typesToAdd == 0) break;
            }
            else if (ifa->Address.lpSockaddr->sa_family == AF_INET6)
            {
                if (!(typesToAdd & (FF_LOCALIP_TYPE_IPV6_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT)))
                {
                    FF_DEBUG("Skipping IPv6 address (not requested in typesToAdd=0x%X)", typesToAdd);
                    continue;
                }

                SOCKADDR_IN6* ipv6 = (SOCKADDR_IN6*) ifa->Address.lpSockaddr;

                FFLocalIpIpv6Type ipv6Type = FF_LOCALIP_IPV6_TYPE_NONE;
                if (IN6_IS_ADDR_GLOBAL(&ipv6->sin6_addr)) ipv6Type |= FF_LOCALIP_IPV6_TYPE_GUA_BIT;
                else if (IN6_IS_ADDR_UNIQUE_LOCAL(&ipv6->sin6_addr)) ipv6Type |= FF_LOCALIP_IPV6_TYPE_ULA_BIT;
                else if (IN6_IS_ADDR_LINKLOCAL(&ipv6->sin6_addr)) ipv6Type |= FF_LOCALIP_IPV6_TYPE_LLA_BIT;
                else ipv6Type |= FF_LOCALIP_IPV6_TYPE_UNKNOWN_BIT;

                if (!(options->ipv6Type & ipv6Type))
                {
                    FF_DEBUG("Skipping IPv6 address (doesn't match requested type 0x%X)", options->ipv6Type);
                    continue;
                }

                bool isDefaultRoute = ((options->showType & FF_LOCALIP_TYPE_IPV6_BIT) && ffNetifGetDefaultRouteV6()->ifIndex == adapter->IfIndex);
                if ((options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT) && !isDefaultRoute)
                {
                    FF_DEBUG("Skipping IPv6 address (not on default route interface)");
                    continue;
                }

                char addressBuffer[INET6_ADDRSTRLEN + 6];
                inet_ntop(AF_INET6, &ipv6->sin6_addr, addressBuffer, INET6_ADDRSTRLEN);

                if ((options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT) && ifa->OnLinkPrefixLength)
                {
                    size_t len = strlen(addressBuffer);
                    snprintf(addressBuffer + len, 6, "/%u", (unsigned) ifa->OnLinkPrefixLength);
                }

                FF_DEBUG("Adding IPv6 address: %s (isDefaultRoute=%s)", addressBuffer, isDefaultRoute ? "true" : "false");

                if (item->ipv6.length) ffStrbufAppendC(&item->ipv6, ',');
                ffStrbufAppendS(&item->ipv6, addressBuffer);
                if (isDefaultRoute) item->defaultRoute |= FF_LOCALIP_TYPE_IPV6_BIT;

                ipv6Count++;
                typesToAdd &= ~(unsigned) FF_LOCALIP_TYPE_IPV6_BIT;
                if (typesToAdd == 0) break;
            }
        }

        FF_DEBUG("Adapter %u: collected %d IPv4 and %d IPv6 addresses", (unsigned)adapter->IfIndex, ipv4Count, ipv6Count);

        if (options->showType & FF_LOCALIP_TYPE_SPEED_BIT)
        {
            item->speed = (int32_t) (adapter->ReceiveLinkSpeed / 1000000);
            FF_DEBUG("Adapter %u speed: %d Mbps (raw: %llu)", (unsigned)adapter->IfIndex, item->speed, adapter->ReceiveLinkSpeed);
        }
        if (options->showType & FF_LOCALIP_TYPE_MTU_BIT)
        {
            item->mtu = (int32_t) adapter->Mtu;
            FF_DEBUG("Adapter %u MTU: %d", (unsigned)adapter->IfIndex, item->mtu);
        }
        if (options->showType & FF_LOCALIP_TYPE_FLAGS_BIT)
        {
            ffLocalIpFillNIFlags(&item->flags, adapter->Flags, niFlagOptions);
            FF_DEBUG("Adapter %u flags: 0x%lX -> '%s'", (unsigned)adapter->IfIndex, adapter->Flags, item->flags.chars);
        }
        if (options->showType & FF_LOCALIP_TYPE_MAC_BIT && adapter->PhysicalAddressLength == 6)
        {
            uint8_t* ptr = adapter->PhysicalAddress;
            ffStrbufSetF(&item->mac, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            FF_DEBUG("Adapter %u MAC: %s", (unsigned)adapter->IfIndex, item->mac.chars);
        }
    }

    FF_DEBUG("Local IP detection completed: scanned %d adapters, processed %d, results count: %u",
             adapterCount, processedCount, results->length);

    return NULL;
}
