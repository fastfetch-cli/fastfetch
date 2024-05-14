#include "localip.h"
#include "common/io/io.h"
#include "common/netif/netif.h"
#include "util/stringUtils.h"

#include <string.h>
#include <ctype.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <net/if_dl.h>
#else
#include <netpacket/packet.h>
#endif

static void addNewIp(FFlist* list, const char* name, const char* addr, int type, bool defaultRoute, bool firstOnly)
{
    FFLocalIpResult* ip = NULL;

    FF_LIST_FOR_EACH(FFLocalIpResult, temp, *list)
    {
        if (!ffStrbufEqualS(&temp->name, name)) continue;
        ip = temp;
        break;
    }
    if (!ip)
    {
        ip = (FFLocalIpResult*) ffListAdd(list);
        ffStrbufInitS(&ip->name, name);
        ffStrbufInit(&ip->ipv4);
        ffStrbufInit(&ip->ipv6);
        ffStrbufInit(&ip->mac);
        ip->defaultRoute = defaultRoute;
    }

    switch (type)
    {
        case AF_INET:
            if (ip->ipv4.length)
            {
                if (firstOnly) return;
                ffStrbufAppendC(&ip->ipv4, ',');
            }
            ffStrbufAppendS(&ip->ipv4, addr);
            break;
        case AF_INET6:
            if (ip->ipv6.length)
            {
                if (firstOnly) return;
                ffStrbufAppendC(&ip->ipv6, ',');
            }
            ffStrbufAppendS(&ip->ipv6, addr);
            break;
        case -1:
            ffStrbufSetS(&ip->mac, addr);
            break;
    }
}

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results)
{
    struct ifaddrs* ifAddrStruct = NULL;
    if(getifaddrs(&ifAddrStruct) < 0)
        return "getifaddrs(&ifAddrStruct) failed";

    const char* defaultRouteIfName = ffNetifGetDefaultRouteIfName();

    for (struct ifaddrs* ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr || !(ifa->ifa_flags & IFF_RUNNING))
            continue;

        bool isDefaultRoute = ffStrEquals(defaultRouteIfName, ifa->ifa_name);
        if ((options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT) && !isDefaultRoute)
            continue;

        if ((ifa->ifa_flags & IFF_LOOPBACK) && !(options->showType & FF_LOCALIP_TYPE_LOOP_BIT))
            continue;

        if (options->namePrefix.length && strncmp(ifa->ifa_name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            if (!(options->showType & FF_LOCALIP_TYPE_IPV4_BIT))
                continue;

            struct sockaddr_in* ipv4 = (struct sockaddr_in*) ifa->ifa_addr;
            char addressBuffer[INET_ADDRSTRLEN + 4];
            inet_ntop(AF_INET, &ipv4->sin_addr, addressBuffer, INET_ADDRSTRLEN);

            if (options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT)
            {
                struct sockaddr_in* netmask = (struct sockaddr_in*) ifa->ifa_netmask;
                int cidr = __builtin_popcount(netmask->sin_addr.s_addr);
                if (cidr != 0)
                {
                    size_t len = strlen(addressBuffer);
                    snprintf(addressBuffer + len, 4, "/%d", cidr);
                }
            }

            addNewIp(results, ifa->ifa_name, addressBuffer, AF_INET, isDefaultRoute, !(options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT));
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            if (!(options->showType & FF_LOCALIP_TYPE_IPV6_BIT))
                continue;

            struct sockaddr_in6* ipv6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            char addressBuffer[INET6_ADDRSTRLEN + 4];
            inet_ntop(AF_INET6, &ipv6->sin6_addr, addressBuffer, INET6_ADDRSTRLEN);

            if (options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT)
            {
                struct sockaddr_in6* netmask = (struct sockaddr_in6*) ifa->ifa_netmask;
                int cidr = 0;
                static_assert(sizeof(netmask->sin6_addr) % sizeof(uint64_t) == 0, "");
                for (uint32_t i = 0; i < sizeof(netmask->sin6_addr) / sizeof(uint64_t); ++i)
                    cidr += __builtin_popcountll(((uint64_t*) &netmask->sin6_addr)[i]);
                if (cidr != 0)
                {
                    size_t len = strlen(addressBuffer);
                    snprintf(addressBuffer + len, 4, "/%d", cidr);
                }
            }

            addNewIp(results, ifa->ifa_name, addressBuffer, AF_INET6, isDefaultRoute, !(options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT));
        }
        #if defined(__FreeBSD__) || defined(__APPLE__)
        else if (ifa->ifa_addr->sa_family == AF_LINK)
        {
            if (!(options->showType & FF_LOCALIP_TYPE_MAC_BIT))
                continue;

            char addressBuffer[32];
            uint8_t* ptr = (uint8_t*) LLADDR((struct sockaddr_dl *)ifa->ifa_addr);
            snprintf(addressBuffer, sizeof(addressBuffer), "%02x:%02x:%02x:%02x:%02x:%02x",
                        ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            addNewIp(results, ifa->ifa_name, addressBuffer, -1, isDefaultRoute, false);
        }
        #else
        else if (ifa->ifa_addr->sa_family == AF_PACKET)
        {
            if (!(options->showType & FF_LOCALIP_TYPE_MAC_BIT))
                continue;

            char addressBuffer[32];
            uint8_t* ptr = ((struct sockaddr_ll *)ifa->ifa_addr)->sll_addr;
            snprintf(addressBuffer, sizeof(addressBuffer), "%02x:%02x:%02x:%02x:%02x:%02x",
                        ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            addNewIp(results, ifa->ifa_name, addressBuffer, -1, isDefaultRoute, false);
        }
        #endif
    }

    if (ifAddrStruct) freeifaddrs(ifAddrStruct);

    return NULL;
}
