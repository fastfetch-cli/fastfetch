#include "localip.h"

#include <string.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#ifdef __FreeBSD__
    #include <sys/socket.h> // FreeBSD needs this for AF_INET
#endif

static void addNewIp(FFlist* list, const char* name, const char* addr, bool ipv6)
{
    FFLocalIpResult* ip = (FFLocalIpResult*) ffListAdd(list);
    ffStrbufInitS(&ip->name, name);
    ffStrbufInitS(&ip->addr, addr);
    ip->ipv6 = ipv6;
}

const char* ffDetectLocalIps(const FFinstance* instance, FFlist* results)
{
    struct ifaddrs* ifAddrStruct = NULL;
    if(getifaddrs(&ifAddrStruct) < 0)
        return "getifaddrs(&ifAddrStruct) failed";

    for (struct ifaddrs* ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr || !(ifa->ifa_flags & IFF_RUNNING))
            continue;

        // loop back
        if (strncmp(ifa->ifa_name, "lo", 2) == 0 && (ifa->ifa_name[2] == '\0' || isdigit(ifa->ifa_name[2])) && !instance->config.localIpShowLoop)
            continue;

        if (instance->config.localIpNamePrefix.length && strncmp(ifa->ifa_name, instance->config.localIpNamePrefix.chars, instance->config.localIpNamePrefix.length) != 0)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            // IPv4
            if (!instance->config.localIpShowIpV4)
                continue;

            struct sockaddr_in* ipv4 = (struct sockaddr_in*) ifa->ifa_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ipv4->sin_addr, addressBuffer, INET_ADDRSTRLEN);
            addNewIp(results, ifa->ifa_name, addressBuffer, false);
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            // IPv6
            if (!instance->config.localIpShowIpV6)
                continue;

            struct sockaddr_in6* ipv6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &ipv6->sin6_addr, addressBuffer, INET6_ADDRSTRLEN);
            addNewIp(results, ifa->ifa_name, addressBuffer, true);
        }
    }

    if (ifAddrStruct) freeifaddrs(ifAddrStruct);
    return NULL;
}
