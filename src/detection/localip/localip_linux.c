#include "localip.h"
#include "common/io/io.h"
#include "common/netif/netif.h"

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

static void addNewIp(FFlist* list, const char* name, const char* addr, int type, bool defaultRoute)
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
            ffStrbufSetS(&ip->ipv4, addr);
            break;
        case AF_INET6:
            ffStrbufSetS(&ip->ipv6, addr);
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

    FF_STRBUF_AUTO_DESTROY defaultRouteIface = ffStrbufCreate();
    ffNetifGetDefaultRoute(&defaultRouteIface);

    for (struct ifaddrs* ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr || !(ifa->ifa_flags & IFF_RUNNING))
            continue;

        if (options->defaultRouteOnly && !ffStrbufEqualS(&defaultRouteIface, ifa->ifa_name))
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
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ipv4->sin_addr, addressBuffer, INET_ADDRSTRLEN);
            addNewIp(results, ifa->ifa_name, addressBuffer, AF_INET, ffStrbufEqualS(&defaultRouteIface, ifa->ifa_name));
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            if (!(options->showType & FF_LOCALIP_TYPE_IPV6_BIT))
                continue;

            struct sockaddr_in6* ipv6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &ipv6->sin6_addr, addressBuffer, INET6_ADDRSTRLEN);
            addNewIp(results, ifa->ifa_name, addressBuffer, AF_INET6, ffStrbufEqualS(&defaultRouteIface, ifa->ifa_name));
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
            addNewIp(results, ifa->ifa_name, addressBuffer, -1, ffStrbufEqualS(&defaultRouteIface, ifa->ifa_name));
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
            addNewIp(results, ifa->ifa_name, addressBuffer, -1, ffStrbufEqualS(&defaultRouteIface, ifa->ifa_name));
        }
        #endif
    }

    if (ifAddrStruct) freeifaddrs(ifAddrStruct);

    return NULL;
}
