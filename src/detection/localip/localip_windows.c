#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "common/netif/netif.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"
#include "localip.h"

static const FFLocalIpNIFlag niFlagOptions[] = {
    { IP_ADAPTER_DDNS_ENABLED, "DDNS_ENABLED" },
    { IP_ADAPTER_REGISTER_ADAPTER_SUFFIX, "REGISTER_ADAPTER_SUFFIX" },
    { IP_ADAPTER_DHCP_ENABLED, "DHCP_ENABLED" },
    { IP_ADAPTER_RECEIVE_ONLY, "RECEIVE_ONLY" },
    { IP_ADAPTER_NO_MULTICAST, "NO_MULTICAST" },
    { IP_ADAPTER_IPV6_OTHER_STATEFUL_CONFIG, "IPV6_OTHER_STATEFUL_CONFIG" },
    { IP_ADAPTER_NETBIOS_OVER_TCPIP_ENABLED, "NETBIOS_OVER_TCPIP_ENABLED" },
    { IP_ADAPTER_IPV4_ENABLED, "IPV4_ENABLED" },
    { IP_ADAPTER_IPV6_ENABLED, "IPV6_ENABLED" },
    { IP_ADAPTER_IPV6_MANAGE_ADDRESS_CONFIG, "IPV6_MANAGE_ADDRESS_CONFIG" },
    // sentinel
    {},
};

static void addNewIp(FFlist* list, const char* name, const char* addr, int type, bool newIp, bool defaultRoute)
{
    FFLocalIpResult* ip = NULL;

    if (newIp)
    {
        ip = (FFLocalIpResult*) ffListAdd(list);
        ffStrbufInitS(&ip->name, name);
        ffStrbufInit(&ip->ipv4);
        ffStrbufInit(&ip->ipv6);
        ffStrbufInit(&ip->mac);
        ffStrbufInit(&ip->flags);
        ip->defaultRoute = defaultRoute;
        ip->speed = -1;
        ip->mtu = -1;
    }
    else
    {
        ip = FF_LIST_GET(FFLocalIpResult, *list, list->length - 1);
    }

    switch (type)
    {
        case AF_INET:
            if (ip->ipv4.length) ffStrbufAppendC(&ip->ipv4, ',');
            ffStrbufAppendS(&ip->ipv4, addr);
            break;
        case AF_INET6:
            if (ip->ipv6.length) ffStrbufAppendC(&ip->ipv6, ',');
            ffStrbufAppendS(&ip->ipv6, addr);
            break;
        case -1:
            ffStrbufSetS(&ip->mac, addr);
            break;
    }
}

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results)
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
            options->showType & FF_LOCALIP_TYPE_IPV4_BIT
                ? options->showType & FF_LOCALIP_TYPE_IPV6_BIT ? AF_UNSPEC : AF_INET
                : AF_INET6,
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
        if ((options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT) && !isDefaultRoute)
            continue;

        bool isLoop = adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK;
        if (isLoop && !(options->showType & FF_LOCALIP_TYPE_LOOP_BIT))
            continue;

        bool newIp = true;

        char name[128];
        WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1, name, ARRAY_SIZE(name), NULL, NULL);
        if (options->namePrefix.length && strncmp(name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        if (options->showType & FF_LOCALIP_TYPE_MAC_BIT && adapter->PhysicalAddressLength == 6)
        {
            char addressBuffer[32];
            uint8_t* ptr = adapter->PhysicalAddress;
            snprintf(addressBuffer, ARRAY_SIZE(addressBuffer), "%02x:%02x:%02x:%02x:%02x:%02x",
                        ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            addNewIp(results, name, addressBuffer, -1, newIp, isDefaultRoute);
            newIp = false;
        }

        uint32_t typesToAdd = options->showType & (FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_IPV6_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT);

        for (IP_ADAPTER_UNICAST_ADDRESS* ifa = adapter->FirstUnicastAddress; ifa; ifa = ifa->Next)
        {
            if (ifa->Address.lpSockaddr->sa_family == AF_INET)
            {
                if (!(typesToAdd & (FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT))) continue;

                SOCKADDR_IN* ipv4 = (SOCKADDR_IN*) ifa->Address.lpSockaddr;
                char addressBuffer[INET_ADDRSTRLEN + 6];
                inet_ntop(AF_INET, &ipv4->sin_addr, addressBuffer, INET_ADDRSTRLEN);

                if ((options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT) && ifa->OnLinkPrefixLength)
                {
                    size_t len = strlen(addressBuffer);
                    snprintf(addressBuffer + len, 6, "/%u", (unsigned) ifa->OnLinkPrefixLength);
                }

                addNewIp(results, name, addressBuffer, AF_INET, newIp, isDefaultRoute);
                newIp = false;

                typesToAdd &= ~(unsigned) FF_LOCALIP_TYPE_IPV4_BIT;
                if (typesToAdd == 0) break;
            }
            else if (ifa->Address.lpSockaddr->sa_family == AF_INET6)
            {
                if (!(typesToAdd & (FF_LOCALIP_TYPE_IPV6_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT))) continue;

                SOCKADDR_IN6* ipv6 = (SOCKADDR_IN6*) ifa->Address.lpSockaddr;
                char addressBuffer[INET6_ADDRSTRLEN + 6];
                inet_ntop(AF_INET6, &ipv6->sin6_addr, addressBuffer, INET6_ADDRSTRLEN);

                if ((options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT) && ifa->OnLinkPrefixLength)
                {
                    size_t len = strlen(addressBuffer);
                    snprintf(addressBuffer + len, 6, "/%u", (unsigned) ifa->OnLinkPrefixLength);
                }

                addNewIp(results, name, addressBuffer, AF_INET6, newIp, isDefaultRoute);
                newIp = false;

                typesToAdd &= ~(unsigned) FF_LOCALIP_TYPE_IPV6_BIT;
                if (typesToAdd == 0) break;
            }
        }

        if (!newIp)
        {
            FFLocalIpResult* result = FF_LIST_GET(FFLocalIpResult, *results, results->length - 1);
            if (options->showType & FF_LOCALIP_TYPE_SPEED_BIT)
                result->speed = (int32_t) (adapter->ReceiveLinkSpeed / 1000);
            if (options->showType & FF_LOCALIP_TYPE_MTU_BIT)
                result->mtu = (int32_t) adapter->Mtu;
            if (options->showType & FF_LOCALIP_TYPE_FLAGS_BIT)
                ffLocalIpFillNIFlags(&result->flags, adapter->Flags, niFlagOptions);
        }
    }

    return NULL;
}
