#include "localip.h"
#include "common/io/io.h"
#include "common/netif/netif.h"
#include "util/stringUtils.h"
#include "util/debug.h"

#include <string.h>
#include <ctype.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <fcntl.h>

#ifdef __linux__
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#endif

#if __has_include(<netinet6/in6_var.h>)
    #include <netinet6/in6_var.h>
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__) || defined(__NetBSD__) || defined(__HAIKU__)
#include <net/if_media.h>
#include <net/if_dl.h>
#elif !defined(__GNU__)
#include <netpacket/packet.h>
#endif
#if defined(__sun) || defined(__HAIKU__)
#include <sys/sockio.h>
#endif
#if defined(__sun)
#include <kstat.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc)
{
    assert(pkc);
    if (*pkc)
        kstat_close(*pkc);
}
#endif

#define FF_LOCALIP_NIFLAG(name) { IFF_##name, #name }

static const FFLocalIpNIFlag niFlagOptions[] = {
    FF_LOCALIP_NIFLAG(UP),
    FF_LOCALIP_NIFLAG(BROADCAST),
#ifdef IFF_DEBUG
    FF_LOCALIP_NIFLAG(DEBUG),
#endif
    FF_LOCALIP_NIFLAG(LOOPBACK),
    FF_LOCALIP_NIFLAG(POINTOPOINT),
#ifdef IFF_RUNNING
    FF_LOCALIP_NIFLAG(RUNNING),
#endif
    FF_LOCALIP_NIFLAG(NOARP),
    FF_LOCALIP_NIFLAG(PROMISC),
    FF_LOCALIP_NIFLAG(ALLMULTI),
#ifdef IFF_INTELLIGENT
    FF_LOCALIP_NIFLAG(INTELLIGENT),
#endif
    FF_LOCALIP_NIFLAG(MULTICAST),
#ifdef IFF_NOTRAILERS
    FF_LOCALIP_NIFLAG(NOTRAILERS),
#endif
#if defined( __linux__) || defined (__GNU__)
    FF_LOCALIP_NIFLAG(MASTER),
    FF_LOCALIP_NIFLAG(SLAVE),
    FF_LOCALIP_NIFLAG(PORTSEL),
    FF_LOCALIP_NIFLAG(AUTOMEDIA),
    FF_LOCALIP_NIFLAG(DYNAMIC),
#endif
#ifdef __linux__
    FF_LOCALIP_NIFLAG(LOWER_UP),
    FF_LOCALIP_NIFLAG(DORMANT),
    FF_LOCALIP_NIFLAG(ECHO),
#endif
#if defined(__FreeBSD__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__NetBSD__)
    FF_LOCALIP_NIFLAG(OACTIVE),
    FF_LOCALIP_NIFLAG(SIMPLEX),
    FF_LOCALIP_NIFLAG(LINK0),
    FF_LOCALIP_NIFLAG(LINK1),
    FF_LOCALIP_NIFLAG(LINK2),
#endif
#ifdef IFF_ALTPHYS
    FF_LOCALIP_NIFLAG(ALTPHYS),
#endif
#ifdef IFF_CANTCONFIG
    FF_LOCALIP_NIFLAG(CANTCONFIG),
#endif
#ifdef __HAIKU__
    FF_LOCALIP_NIFLAG(AUTOUP),
    FF_LOCALIP_NIFLAG(SIMPLEX),
    FF_LOCALIP_NIFLAG(LINK),
    FF_LOCALIP_NIFLAG(AUTO_CONFIGURED),
    FF_LOCALIP_NIFLAG(CONFIGURING),
#endif
#ifdef __sun
    FF_LOCALIP_NIFLAG(MULTI_BCAST),
    FF_LOCALIP_NIFLAG(UNNUMBERED),
    FF_LOCALIP_NIFLAG(DHCPRUNNING),
    FF_LOCALIP_NIFLAG(PRIVATE),
#endif
    // sentinel
    {},
};

static FFLocalIpIpv6Type getIpv6Type(struct ifaddrs* ifa)
{
    struct sockaddr_in6* addr = (struct sockaddr_in6*) ifa->ifa_addr;

    FF_DEBUG("Checking IPv6 type for interface %s", ifa->ifa_name);

    FFLocalIpIpv6Type result = FF_LOCALIP_IPV6_TYPE_NONE;
    if (IN6_IS_ADDR_GLOBAL(&addr->sin6_addr))
    {
        result = FF_LOCALIP_IPV6_TYPE_GUA_BIT;
        FF_DEBUG("Interface %s has Global Unicast Address", ifa->ifa_name);
    }
    else if (IN6_IS_ADDR_UNIQUE_LOCAL(&addr->sin6_addr))
    {
        result = FF_LOCALIP_IPV6_TYPE_ULA_BIT;
        FF_DEBUG("Interface %s has Unique Local Address", ifa->ifa_name);
    }
    else if (IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr))
    {
        result = FF_LOCALIP_IPV6_TYPE_LLA_BIT;
        FF_DEBUG("Interface %s has Link-Local Address", ifa->ifa_name);
    }
    else
    {
        FF_DEBUG("Interface %s has unknown IPv6 address type", ifa->ifa_name);
        return FF_LOCALIP_IPV6_TYPE_UNKNOWN_BIT;
    }

#ifdef SIOCGIFAFLAG_IN6
    static int sockfd = 0;
    if (sockfd == 0)
    {
        sockfd = socket(AF_INET6, SOCK_DGRAM
            #ifdef SOCK_CLOEXEC
            | SOCK_CLOEXEC
            #endif
        , 0);
        #ifndef SOCK_CLOEXEC
        if (sockfd > 0) fcntl(sockfd, F_SETFD, FD_CLOEXEC);
        #endif
    }
    if (sockfd < 0) return result;

    struct in6_ifreq ifr6 = {};
    ffStrCopy(ifr6.ifr_name, ifa->ifa_name, IFNAMSIZ);
    ifr6.ifr_addr = *addr;

    if (ioctl(sockfd, SIOCGIFAFLAG_IN6, &ifr6) != 0)
        return result;

    #ifdef IN6_IFF_PREFER_SOURCE
        if (ifr6.ifr_ifru.ifru_flags6 & IN6_IFF_PREFER_SOURCE)
            result |= FF_LOCALIP_IPV6_TYPE_PREFERRED_BIT;
    #endif
    if (ifr6.ifr_ifru.ifru_flags6 & (IN6_IFF_DEPRECATED | IN6_IFF_TEMPORARY | IN6_IFF_TENTATIVE | IN6_IFF_DUPLICATED
        #ifdef IN6_IFF_OPTIMISTIC
             | IN6_IFF_OPTIMISTIC
        #endif
    )) result |= FF_LOCALIP_IPV6_TYPE_SECONDARY_BIT;
    return result;
#elif __linux__
    static FFlist addresses = {};
    if (addresses.elementSize == 0)
    {
        ffListInit(&addresses, sizeof(struct in6_addr));
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        if (!ffReadFileBuffer("/proc/net/if_inet6", &buffer))
            return result;

        char* line = NULL;
        size_t len = 0;
        while (ffStrbufGetline(&line, &len, &buffer))
        {
            struct in6_addr* entry = (struct in6_addr*) ffListAdd(&addresses);
            uint8_t flags;
            if (sscanf(line, "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 "%2" SCNx8 " %*s %*s %*s %" SCNx8 " %*s",
                    &entry->s6_addr[0], &entry->s6_addr[1], &entry->s6_addr[2], &entry->s6_addr[3],
                    &entry->s6_addr[4], &entry->s6_addr[5], &entry->s6_addr[6], &entry->s6_addr[7],
                    &entry->s6_addr[8], &entry->s6_addr[9], &entry->s6_addr[10], &entry->s6_addr[11],
                    &entry->s6_addr[12], &entry->s6_addr[13], &entry->s6_addr[14], &entry->s6_addr[15],
                    &flags) != 17 ||
                (!IN6_IS_ADDR_GLOBAL(entry) && !IN6_IS_ADDR_UNIQUE_LOCAL(entry)) ||
                (flags & (IFA_F_DEPRECATED | IFA_F_TEMPORARY | IFA_F_TENTATIVE | IFA_F_DADFAILED | IFA_F_OPTIMISTIC))
            )
                --addresses.length;
        }
    }
    if (addresses.capacity == 0) return result;

    FF_LIST_FOR_EACH(struct in6_addr, entry, addresses)
    {
        if (memcmp(&addr->sin6_addr, entry, sizeof(struct in6_addr)) == 0)
            return result;
    }
    result |= FF_LOCALIP_IPV6_TYPE_SECONDARY_BIT;
    return result;
#elif __sun
    if (ifa->ifa_flags & IFF_PREFERRED)
        result |= FF_LOCALIP_IPV6_TYPE_PREFERRED_BIT;
    if (ifa->ifa_flags & (IFF_DEPRECATED | IFF_TEMPORARY | IFF_DUPLICATE))
        result |= FF_LOCALIP_IPV6_TYPE_SECONDARY_BIT;
    return result;
#else
    return result;
#endif
}

typedef struct {
    struct ifaddrs* mac;
    FFlist /*<struct ifaddrs*>*/ ipv4;
    FFlist /*<struct ifaddrs*>*/ ipv6;
} FFAdapter;

static void appendIpv4(const FFLocalIpOptions* options, FFstrbuf* buffer, const struct ifaddrs* ifa)
{
    struct sockaddr_in* ipv4 = (struct sockaddr_in*) ifa->ifa_addr;

    char addressBuffer[INET_ADDRSTRLEN + 16];
    inet_ntop(AF_INET, &ipv4->sin_addr, addressBuffer, INET_ADDRSTRLEN);

    FF_DEBUG("Adding IPv4 address %s for interface %s", addressBuffer, ifa->ifa_name);

    if (options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT)
    {
        struct sockaddr_in* netmask = (struct sockaddr_in*) ifa->ifa_netmask;
        int cidr = __builtin_popcount(netmask->sin_addr.s_addr);
        if (cidr != 0)
        {
            size_t len = strlen(addressBuffer);
            snprintf(addressBuffer + len, 16, "/%d", cidr);
        }
    }

    if (buffer->length) ffStrbufAppendC(buffer, ',');
    ffStrbufAppendS(buffer, addressBuffer);
}

static void appendIpv6(const FFLocalIpOptions* options, FFstrbuf* buffer, const struct ifaddrs* ifa)
{
    struct sockaddr_in6* ipv6 = (struct sockaddr_in6*) ifa->ifa_addr;

    char addressBuffer[INET6_ADDRSTRLEN + 16];
    inet_ntop(AF_INET6, &ipv6->sin6_addr, addressBuffer, INET6_ADDRSTRLEN);

    FF_DEBUG("Adding IPv6 address %s for interface %s", addressBuffer, ifa->ifa_name);

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
            snprintf(addressBuffer + len, 16, "/%d", cidr);
        }
    }

    if (buffer->length) ffStrbufAppendC(buffer, ',');
    ffStrbufAppendS(buffer, addressBuffer);
}

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results)
{
    FF_DEBUG("Starting local IP detection with showType=0x%x, namePrefix='%s'",
             options->showType, options->namePrefix.chars);

    struct ifaddrs* ifAddrStruct = NULL;
    if(getifaddrs(&ifAddrStruct) < 0)
    {
        FF_DEBUG("getifaddrs() failed");
        return "getifaddrs(&ifAddrStruct) failed";
    }

    FF_DEBUG("Successfully retrieved interface addresses");

    FF_LIST_AUTO_DESTROY adapters = ffListCreate(sizeof(FFAdapter));

    for (struct ifaddrs* ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            FF_DEBUG("Skipping interface %s (no address)", ifa->ifa_name);
            continue;
        }

        #ifdef IFF_RUNNING
        if (!(ifa->ifa_flags & IFF_RUNNING))
        {
            FF_DEBUG("Skipping interface %s (not running)", ifa->ifa_name);
            continue;
        }
        #endif

        if ((ifa->ifa_flags & IFF_LOOPBACK) && !(options->showType & FF_LOCALIP_TYPE_LOOP_BIT))
        {
            FF_DEBUG("Skipping loopback interface %s", ifa->ifa_name);
            continue;
        }

        if (options->namePrefix.length && strncmp(ifa->ifa_name, options->namePrefix.chars, options->namePrefix.length) != 0)
        {
            FF_DEBUG("Skipping interface %s (doesn't match prefix '%s')",
                     ifa->ifa_name, options->namePrefix.chars);
            continue;
        }

        if (!(options->showType & FF_LOCALIP_TYPE_MAC_BIT) &&
            ifa->ifa_addr->sa_family != AF_INET && ifa->ifa_addr->sa_family != AF_INET6)
        {
            FF_DEBUG("Skipping interface %s (unsupported address family %d)",
                     ifa->ifa_name, ifa->ifa_addr->sa_family);
            continue;
        }

        if (options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT)
        {
            // If the interface is not the default route for either IPv4 or IPv6, skip it
            if (!((options->showType & FF_LOCALIP_TYPE_IPV4_BIT) && ffStrEquals(ffNetifGetDefaultRouteV4()->ifName, ifa->ifa_name)) &&
                !((options->showType & FF_LOCALIP_TYPE_IPV6_BIT) && ffStrEquals(ffNetifGetDefaultRouteV6()->ifName, ifa->ifa_name)))
            {
                FF_DEBUG("Skipping interface %s (not default route interface)", ifa->ifa_name);
                    continue;
            }
        }

        FF_DEBUG("Processing interface %s (family=%d, flags=0x%x)",
                 ifa->ifa_name, ifa->ifa_addr->sa_family, ifa->ifa_flags);

        FFAdapter* adapter = NULL;
        FF_LIST_FOR_EACH(FFAdapter, x, adapters)
        {
            if (ffStrEquals(x->mac->ifa_name, ifa->ifa_name))
            {
                adapter = x;
                break;
            }
        }
        if (!adapter)
        {
            adapter = ffListAdd(&adapters);
            *adapter = (FFAdapter) {
                .mac = ifa,
                .ipv4 = ffListCreate(sizeof(struct ifaddrs*)),
                .ipv6 = ffListCreate(sizeof(struct ifaddrs*)),
            };
            FF_DEBUG("Created new adapter entry for interface %s", ifa->ifa_name);
        }

        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
                if (options->showType & FF_LOCALIP_TYPE_IPV4_BIT)
                {
                    *FF_LIST_ADD(struct ifaddrs*, adapter->ipv4) = ifa;
                    FF_DEBUG("Added IPv4 entry for interface %s", ifa->ifa_name);
                }
                break;
            case AF_INET6:
                if (options->showType & FF_LOCALIP_TYPE_IPV6_BIT)
                {
                    *FF_LIST_ADD(struct ifaddrs*, adapter->ipv6) = ifa;
                    FF_DEBUG("Added IPv6 entry for interface %s", ifa->ifa_name);
                }
                break;
            #if __FreeBSD__ || __OpenBSD__ || __APPLE__ || __NetBSD__ || __HAIKU__
            case AF_LINK:
                adapter->mac = ifa;
                FF_DEBUG("Updated MAC entry for interface %s", ifa->ifa_name);
                break;
            #elif !__sun && !__GNU__
            case AF_PACKET:
                adapter->mac = ifa;
                FF_DEBUG("Updated MAC entry for interface %s", ifa->ifa_name);
                break;
            #endif
        }
    }

    FF_DEBUG("Found %u network adapters", adapters.length);

    FF_LIST_FOR_EACH(FFAdapter, adapter, adapters)
    {
        FF_DEBUG("Processing adapter %s (IPv4 entries: %u, IPv6 entries: %u)",
                 adapter->mac->ifa_name, adapter->ipv4.length, adapter->ipv6.length);

        if (adapter->ipv4.length == 0 && adapter->ipv6.length == 0 &&
            !(options->showType & FF_LOCALIP_TYPE_MAC_BIT) )
        {
            FF_DEBUG("Skipping interface %s (no IP addresses)", adapter->mac->ifa_name);
            continue;
        }

        FFLocalIpResult* item = FF_LIST_ADD(FFLocalIpResult, *results);
        ffStrbufInitS(&item->name, adapter->mac->ifa_name);
        ffStrbufInit(&item->ipv4);
        ffStrbufInit(&item->ipv6);
        ffStrbufInit(&item->mac);
        ffStrbufInit(&item->flags);
        item->defaultRoute = FF_LOCALIP_TYPE_NONE;
        item->mtu = -1;
        item->speed = -1;

        if (options->showType & FF_LOCALIP_TYPE_FLAGS_BIT)
        {
            ffLocalIpFillNIFlags(&item->flags, adapter->mac->ifa_flags, niFlagOptions);
            FF_DEBUG("Added flags for interface %s: %s", adapter->mac->ifa_name, item->flags.chars);
        }

        if ((options->showType & FF_LOCALIP_TYPE_IPV4_BIT))
        {
            const FFNetifDefaultRouteResult* defaultRouteV4 = ffNetifGetDefaultRouteV4();
            bool isDefaultRouteIf = ffStrEquals(defaultRouteV4->ifName, adapter->mac->ifa_name);

            if (isDefaultRouteIf)
            {
                item->defaultRoute |= FF_LOCALIP_TYPE_IPV4_BIT;
                FF_DEBUG("Interface %s is IPv4 default route", adapter->mac->ifa_name);
            }

            if (options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT)
            {
                if (!isDefaultRouteIf)
                {
                    FF_DEBUG("Skipping IPv4 for interface %s (not default route)", adapter->mac->ifa_name);
                    goto v6;
                }
            }

            if (!(options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT))
            {
                struct ifaddrs* ifa = NULL;
                if (isDefaultRouteIf && defaultRouteV4->preferredSourceAddrV4 != 0)
                {
                    FF_LIST_FOR_EACH(struct ifaddrs*, pifa, adapter->ipv4)
                    {
                        struct sockaddr_in* ipv4 = (struct sockaddr_in*) (*pifa)->ifa_addr;
                        if (ipv4->sin_addr.s_addr == defaultRouteV4->preferredSourceAddrV4)
                        {
                            ifa = *pifa;
                            FF_DEBUG("Found preferred IPv4 source address for interface %s", adapter->mac->ifa_name);
                            break;
                        }
                    }
                }
                if (ifa)
                    appendIpv4(options, &item->ipv4, ifa);
                else if (adapter->ipv4.length > 0)
                {
                    appendIpv4(options, &item->ipv4, *FF_LIST_GET(struct ifaddrs*, adapter->ipv4, 0));
                    FF_DEBUG("Using first IPv4 address for interface %s", adapter->mac->ifa_name);
                }
            }
            else
            {
                FF_DEBUG("Adding all IPv4 addresses for interface %s", adapter->mac->ifa_name);
                FF_LIST_FOR_EACH(struct ifaddrs*, pifa, adapter->ipv4)
                    appendIpv4(options, &item->ipv4, *pifa);
            }
        }
    v6:
        if ((options->showType & FF_LOCALIP_TYPE_IPV6_BIT))
        {
            const FFNetifDefaultRouteResult* defaultRouteV6 = ffNetifGetDefaultRouteV6();
            bool isDefaultRouteIf = ffStrEquals(defaultRouteV6->ifName, adapter->mac->ifa_name);

            if (isDefaultRouteIf)
            {
                item->defaultRoute |= FF_LOCALIP_TYPE_IPV6_BIT;
                FF_DEBUG("Interface %s is IPv6 default route", adapter->mac->ifa_name);
            }

            if (options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT)
            {
                if (!isDefaultRouteIf)
                {
                    FF_DEBUG("Skipping IPv6 for interface %s (not default route)", adapter->mac->ifa_name);
                    goto mac;
                }
            }

            if (options->ipv6Type == FF_LOCALIP_IPV6_TYPE_AUTO)
            {
                if (!(options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT))
                {
                    struct ifaddrs* selected = NULL;
                    struct ifaddrs* secondary = NULL;

                    FF_LIST_FOR_EACH(struct ifaddrs*, pifa, adapter->ipv6)
                    {
                        FFLocalIpIpv6Type type = getIpv6Type(*pifa);
                        if (type & FF_LOCALIP_IPV6_TYPE_PREFERRED_BIT)
                        {
                            selected = *pifa;
                            FF_DEBUG("Found preferred IPv6 address for interface %s", adapter->mac->ifa_name);
                            break;
                        }
                        else if ((type & FF_LOCALIP_IPV6_TYPE_GUA_BIT) && !(type & FF_LOCALIP_IPV6_TYPE_SECONDARY_BIT) && !selected)
                        {
                            selected = *pifa;
                            FF_DEBUG("Found GUA IPv6 address for interface %s", adapter->mac->ifa_name);
                        }
                        else if ((type & FF_LOCALIP_IPV6_TYPE_ULA_BIT) && !(type & FF_LOCALIP_IPV6_TYPE_SECONDARY_BIT) && !secondary)
                        {
                            secondary = *pifa;
                            FF_DEBUG("Found ULA IPv6 address for interface %s", adapter->mac->ifa_name);
                        }
                    }
                    if (!selected) selected = secondary;

                    if (selected)
                        appendIpv6(options, &item->ipv6, selected);
                    else if (adapter->ipv6.length > 0)
                    {
                        appendIpv6(options, &item->ipv6, *FF_LIST_GET(struct ifaddrs*, adapter->ipv6, 0));
                        FF_DEBUG("Using first IPv6 address for interface %s", adapter->mac->ifa_name);
                    }
                }
                else
                {
                    FF_DEBUG("Adding all IPv6 addresses for interface %s", adapter->mac->ifa_name);
                    FF_LIST_FOR_EACH(struct ifaddrs*, pifa, adapter->ipv6)
                        appendIpv6(options, &item->ipv6, *pifa);
                }
            }
            else
            {
                FF_LIST_FOR_EACH(struct ifaddrs*, pifa, adapter->ipv6)
                {
                    FFLocalIpIpv6Type type = getIpv6Type(*pifa);
                    if (type & options->ipv6Type)
                    {
                        if ((options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT) || !(type & FF_LOCALIP_IPV6_TYPE_SECONDARY_BIT))
                        {
                            appendIpv6(options, &item->ipv6, *pifa);
                            if (!(options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT))
                                break;
                        }
                    }
                }
            }
        }
    mac:
        #if !defined( __sun)  && !defined(__GNU__)
        if (options->showType & FF_LOCALIP_TYPE_MAC_BIT)
        {
            if (adapter->mac->ifa_addr)
            {
                #if __FreeBSD__ || __OpenBSD__ || __APPLE__ || __NetBSD__ || __HAIKU__
                uint8_t* ptr = (uint8_t*) LLADDR((struct sockaddr_dl *)adapter->mac->ifa_addr);
                #else
                uint8_t* ptr = ((struct sockaddr_ll *)adapter->mac->ifa_addr)->sll_addr;
                #endif
                ffStrbufSetF(&item->mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                    ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
                FF_DEBUG("Added MAC address %s for interface %s", item->mac.chars, adapter->mac->ifa_name);
            }
            else
            {
                FF_DEBUG("No MAC address available for interface %s", adapter->mac->ifa_name);
            }
        }
        #else
        (void) adapter;
        #endif
    }

    FF_LIST_FOR_EACH(FFAdapter, adapter, adapters)
    {
        ffListDestroy(&adapter->ipv4);
        ffListDestroy(&adapter->ipv6);
    }

    if (ifAddrStruct)
    {
        freeifaddrs(ifAddrStruct);
        ifAddrStruct = NULL;
        FF_DEBUG("Cleaned up interface address structures");
    }

    if ((options->showType & FF_LOCALIP_TYPE_MTU_BIT) || (options->showType & FF_LOCALIP_TYPE_SPEED_BIT)
        #ifdef __sun
        || (options->showType & FF_LOCALIP_TYPE_MAC_BIT)
        #endif
    )
    {
        FF_DEBUG("Retrieving additional interface properties (MTU/Speed/MAC)");
        FF_AUTO_CLOSE_FD int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd > 0)
        {
            FF_LIST_FOR_EACH(FFLocalIpResult, iface, *results)
            {
                struct ifreq ifr = {};
                ffStrCopy(ifr.ifr_name, iface->name.chars, IFNAMSIZ);

                if (options->showType & FF_LOCALIP_TYPE_MTU_BIT)
                {
                    if (ioctl(sockfd, SIOCGIFMTU, &ifr) == 0)
                    {
                        iface->mtu = (int32_t) ifr.ifr_mtu;
                        FF_DEBUG("Interface %s MTU: %d", iface->name.chars, iface->mtu);
                    }
                    else
                    {
                        FF_DEBUG("Failed to get MTU for interface %s", iface->name.chars);
                    }
                }

                if (options->showType & FF_LOCALIP_TYPE_SPEED_BIT)
                {
                    #ifdef __linux__
                    struct ethtool_cmd edata = { .cmd = ETHTOOL_GSET };
                    ifr.ifr_data = (void*) &edata;
                    if (ioctl(sockfd, SIOCETHTOOL, &ifr) == 0)
                    {
                        iface->speed = (edata.speed_hi << 16) | edata.speed;
                        FF_DEBUG("Interface %s speed: %d Mbps", iface->name.chars, iface->speed);
                    }
                    else
                    {
                        // ethtool_cmd_speed is not available on Android
                        FF_DEBUG("Failed to get speed for interface %s via ethtool", iface->name.chars);
                    }
                    #elif __FreeBSD__ || __APPLE__ || __OpenBSD__ || __NetBSD__
                    struct ifmediareq ifmr = {};
                    ffStrCopy(ifmr.ifm_name, iface->name.chars, IFNAMSIZ);
                    if (ioctl(sockfd, SIOCGIFMEDIA, &ifmr) == 0 && (IFM_TYPE(ifmr.ifm_active) & IFM_ETHER))
                    {
                        FF_DEBUG("Interface %s media type: 0x%x", iface->name.chars, IFM_SUBTYPE(ifmr.ifm_active));
                        switch (IFM_SUBTYPE(ifmr.ifm_active))
                        {
                        #ifdef IFM_HPNA_1
                        case IFM_HPNA_1:
                        #endif
                            iface->speed = 1; break;
                        #ifdef IFM_1000_CX
                        case IFM_1000_CX:
                        #endif
                        #ifdef IFM_1000_CX_SGMII
                        case IFM_1000_CX_SGMII:
                        #endif
                        #ifdef IFM_1000_KX
                        case IFM_1000_KX:
                        #endif
                        #ifdef IFM_1000_LX
                        case IFM_1000_LX:
                        #endif
                        #ifdef IFM_1000_SGMII
                        case IFM_1000_SGMII:
                        #endif
                        #ifdef IFM_1000_SX
                        case IFM_1000_SX:
                        #endif
                        #ifdef IFM_1000_T
                        case IFM_1000_T:
                        #endif
                            iface->speed = 1000; break;
                        #ifdef IFM_100G_AUI2
                        case IFM_100G_AUI2:
                        #endif
                        #ifdef IFM_100G_AUI2_AC
                        case IFM_100G_AUI2_AC:
                        #endif
                        #ifdef IFM_100G_AUI4
                        case IFM_100G_AUI4:
                        #endif
                        #ifdef IFM_100G_AUI4_AC
                        case IFM_100G_AUI4_AC:
                        #endif
                        #ifdef IFM_100G_CAUI2
                        case IFM_100G_CAUI2:
                        #endif
                        #ifdef IFM_100G_CAUI2_AC
                        case IFM_100G_CAUI2_AC:
                        #endif
                        #ifdef IFM_100G_CAUI4
                        case IFM_100G_CAUI4:
                        #endif
                        #ifdef IFM_100G_CAUI4_AC
                        case IFM_100G_CAUI4_AC:
                        #endif
                        #ifdef IFM_100G_CP2
                        case IFM_100G_CP2:
                        #endif
                        #ifdef IFM_100G_CR4
                        case IFM_100G_CR4:
                        #endif
                        #ifdef IFM_100G_CR_PAM4
                        case IFM_100G_CR_PAM4:
                        #endif
                        #ifdef IFM_100G_DR
                        case IFM_100G_DR:
                        #endif
                        #ifdef IFM_100G_KR2_PAM4
                        case IFM_100G_KR2_PAM4:
                        #endif
                        #ifdef IFM_100G_KR4
                        case IFM_100G_KR4:
                        #endif
                        #ifdef IFM_100G_KR_PAM4
                        case IFM_100G_KR_PAM4:
                        #endif
                        #ifdef IFM_100G_LR4
                        case IFM_100G_LR4:
                        #endif
                        #ifdef IFM_100G_SR2
                        case IFM_100G_SR2:
                        #endif
                        #ifdef IFM_100G_SR4
                        case IFM_100G_SR4:
                        #endif
                            iface->speed = 100000; break;
                        #ifdef IFM_100_FX
                        case IFM_100_FX:
                        #endif
                        #ifdef IFM_100_SGMII
                        case IFM_100_SGMII:
                        #endif
                        #ifdef IFM_100_T
                        case IFM_100_T:
                        #endif
                        #ifdef IFM_100_T2
                        case IFM_100_T2:
                        #endif
                        #ifdef IFM_100_T4
                        case IFM_100_T4:
                        #endif
                        #ifdef IFM_100_TX
                        case IFM_100_TX:
                        #endif
                        #ifdef IFM_100_VG
                        case IFM_100_VG:
                        #endif
                            iface->speed = 100; break;
                        #ifdef IFM_10G_AOC
                        case IFM_10G_AOC:
                        #endif
                        #ifdef IFM_10G_CR1
                        case IFM_10G_CR1:
                        #endif
                        #ifdef IFM_10G_CX4
                        case IFM_10G_CX4:
                        #endif
                        #ifdef IFM_10G_ER
                        case IFM_10G_ER:
                        #endif
                        #ifdef IFM_10G_KR
                        case IFM_10G_KR:
                        #endif
                        #ifdef IFM_10G_KX4
                        case IFM_10G_KX4:
                        #endif
                        #ifdef IFM_10G_LR
                        case IFM_10G_LR:
                        #endif
                        #ifdef IFM_10G_LRM
                        case IFM_10G_LRM:
                        #endif
                        #ifdef IFM_10G_SFI
                        case IFM_10G_SFI:
                        #endif
                        #ifdef IFM_10G_SR
                        case IFM_10G_SR:
                        #endif
                        #ifdef IFM_10G_T
                        case IFM_10G_T:
                        #endif
                        #ifdef IFM_10G_TWINAX
                        case IFM_10G_TWINAX:
                        #endif
                        #ifdef IFM_10G_TWINAX_LONG
                        case IFM_10G_TWINAX_LONG:
                        #endif
                            iface->speed = 10000; break;
                        #ifdef IFM_10_2
                        case IFM_10_2:
                        #endif
                        #ifdef IFM_10_5
                        case IFM_10_5:
                        #endif
                        #ifdef IFM_10_FL
                        case IFM_10_FL:
                        #endif
                        #ifdef IFM_10_STP
                        case IFM_10_STP:
                        #endif
                        #ifdef IFM_10_T
                        case IFM_10_T:
                        #endif
                            iface->speed = 10; break;
                        #ifdef IFM_200G_AUI4
                        case IFM_200G_AUI4:
                        #endif
                        #ifdef IFM_200G_AUI4_AC
                        case IFM_200G_AUI4_AC:
                        #endif
                        #ifdef IFM_200G_AUI8
                        case IFM_200G_AUI8:
                        #endif
                        #ifdef IFM_200G_AUI8_AC
                        case IFM_200G_AUI8_AC:
                        #endif
                        #ifdef IFM_200G_CR4_PAM4
                        case IFM_200G_CR4_PAM4:
                        #endif
                        #ifdef IFM_200G_DR4
                        case IFM_200G_DR4:
                        #endif
                        #ifdef IFM_200G_FR4
                        case IFM_200G_FR4:
                        #endif
                        #ifdef IFM_200G_KR4_PAM4
                        case IFM_200G_KR4_PAM4:
                        #endif
                        #ifdef IFM_200G_LR4
                        case IFM_200G_LR4:
                        #endif
                        #ifdef IFM_200G_SR4
                        case IFM_200G_SR4:
                        #endif
                            iface->speed = 200000; break;
                        #ifdef IFM_20G_KR2
                        case IFM_20G_KR2:
                        #endif
                            iface->speed = 20000; break;
                        #ifdef IFM_2500_KX
                        case IFM_2500_KX:
                        #endif
                        #ifdef IFM_2500_SX
                        case IFM_2500_SX:
                        #endif
                        #ifdef IFM_2500_T
                        case IFM_2500_T:
                        #endif
                        #ifdef IFM_2500_X
                        case IFM_2500_X:
                        #endif
                            iface->speed = 2500; break;
                        #ifdef IFM_25G_ACC
                        case IFM_25G_ACC:
                        #endif
                        #ifdef IFM_25G_AOC
                        case IFM_25G_AOC:
                        #endif
                        #ifdef IFM_25G_AUI
                        case IFM_25G_AUI:
                        #endif
                        #ifdef IFM_25G_CR
                        case IFM_25G_CR:
                        #endif
                        #ifdef IFM_25G_CR1
                        case IFM_25G_CR1:
                        #endif
                        #ifdef IFM_25G_CR_S
                        case IFM_25G_CR_S:
                        #endif
                        #ifdef IFM_25G_KR
                        case IFM_25G_KR:
                        #endif
                        #ifdef IFM_25G_KR1
                        case IFM_25G_KR1:
                        #endif
                        #ifdef IFM_25G_KR_S
                        case IFM_25G_KR_S:
                        #endif
                        #ifdef IFM_25G_LR
                        case IFM_25G_LR:
                        #endif
                        #ifdef IFM_25G_PCIE
                        case IFM_25G_PCIE:
                        #endif
                        #ifdef IFM_25G_SR
                        case IFM_25G_SR:
                        #endif
                        #ifdef IFM_25G_T
                        case IFM_25G_T:
                        #endif
                            iface->speed = 25000; break;
                        #ifdef IFM_400G_AUI8
                        case IFM_400G_AUI8:
                        #endif
                        #ifdef IFM_400G_AUI8_AC
                        case IFM_400G_AUI8_AC:
                        #endif
                        #ifdef IFM_400G_DR4
                        case IFM_400G_DR4:
                        #endif
                        #ifdef IFM_400G_FR8
                        case IFM_400G_FR8:
                        #endif
                        #ifdef IFM_400G_LR8
                        case IFM_400G_LR8:
                        #endif
                            iface->speed = 400000; break;
                        #ifdef IFM_40G_CR4
                        case IFM_40G_CR4:
                        #endif
                        #ifdef IFM_40G_ER4
                        case IFM_40G_ER4:
                        #endif
                        #ifdef IFM_40G_KR4
                        case IFM_40G_KR4:
                        #endif
                        #ifdef IFM_40G_LR4
                        case IFM_40G_LR4:
                        #endif
                        #ifdef IFM_40G_SR4
                        case IFM_40G_SR4:
                        #endif
                        #ifdef IFM_40G_XLAUI
                        case IFM_40G_XLAUI:
                        #endif
                        #ifdef IFM_40G_XLAUI_AC
                        case IFM_40G_XLAUI_AC:
                        #endif
                        #ifdef IFM_40G_XLPPI
                        case IFM_40G_XLPPI:
                        #endif
                        #ifdef IFM_40G_LM4
                        case IFM_40G_LM4:
                        #endif
                            iface->speed = 40000; break;
                        #ifdef IFM_5000_KR
                        case IFM_5000_KR:
                        #endif
                        #ifdef IFM_5000_KR1
                        case IFM_5000_KR1:
                        #endif
                        #ifdef IFM_5000_KR_S
                        case IFM_5000_KR_S:
                        #endif
                        #ifdef IFM_5000_T
                        case IFM_5000_T:
                        #endif
                            iface->speed = 5000; break;
                        #ifdef IFM_50G_AUI1
                        case IFM_50G_AUI1:
                        #endif
                        #ifdef IFM_50G_AUI1_AC
                        case IFM_50G_AUI1_AC:
                        #endif
                        #ifdef IFM_50G_AUI2
                        case IFM_50G_AUI2:
                        #endif
                        #ifdef IFM_50G_AUI2_AC
                        case IFM_50G_AUI2_AC:
                        #endif
                        #ifdef IFM_50G_CP
                        case IFM_50G_CP:
                        #endif
                        #ifdef IFM_50G_CR2
                        case IFM_50G_CR2:
                        #endif
                        #ifdef IFM_50G_FR
                        case IFM_50G_FR:
                        #endif
                        #ifdef IFM_50G_KR2
                        case IFM_50G_KR2:
                        #endif
                        #ifdef IFM_50G_KR_PAM4
                        case IFM_50G_KR_PAM4:
                        #endif
                        #ifdef IFM_50G_LAUI2
                        case IFM_50G_LAUI2:
                        #endif
                        #ifdef IFM_50G_LAUI2_AC
                        case IFM_50G_LAUI2_AC:
                        #endif
                        #ifdef IFM_50G_LR
                        case IFM_50G_LR:
                        #endif
                        #ifdef IFM_50G_LR2
                        case IFM_50G_LR2:
                        #endif
                        #ifdef IFM_50G_PCIE
                        case IFM_50G_PCIE:
                        #endif
                        #ifdef IFM_50G_SR
                        case IFM_50G_SR:
                        #endif
                        #ifdef IFM_50G_SR2
                        case IFM_50G_SR2:
                        #endif
                        #ifdef IFM_50G_KR4
                        case IFM_50G_KR4:
                        #endif
                            iface->speed = 50000; break;
                        #ifdef IFM_56G_R4
                        case IFM_56G_R4:
                        #endif
                            iface->speed = 56000; break;
                        default:
                            iface->speed = -1;
                            FF_DEBUG("Unknown media subtype for interface %s", iface->name.chars);
                            break;
                        }
                        if (iface->speed > 0)
                            FF_DEBUG("Interface %s speed: %d Mbps", iface->name.chars, iface->speed);
                    }
                    else
                    {
                        FF_DEBUG("Failed to get media info for interface %s", iface->name.chars);
                    }
                    #endif
                }

                #if __sun || __GNU__
                if ((options->showType & FF_LOCALIP_TYPE_MAC_BIT) && ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0)
                {
                    const uint8_t* ptr = (uint8_t*) ifr.ifr_addr.sa_data; // NOT ifr_enaddr
                    ffStrbufSetF(&iface->mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                                ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
                    FF_DEBUG("Added MAC address %s for interface %s (Solaris/GNU)", iface->mac.chars, iface->name.chars);
                }
                #endif
                #if __sun
                if (options->showType & FF_LOCALIP_TYPE_SPEED_BIT)
                {
                    __attribute__((__cleanup__(kstatFreeWrap))) kstat_ctl_t* kc = kstat_open();
                    for (kstat_t* ks = kc->kc_chain; ks; ks = ks->ks_next)
                    {
                        if (!ffStrEquals(ks->ks_class, "net") || !ffStrEquals(ks->ks_module, "link")) continue;
                        if (ffStrbufEqualS(&iface->name, ks->ks_name))
                        {
                            if (kstat_read(kc, ks, NULL) >= 0)
                            {
                                kstat_named_t* ifspeed = (kstat_named_t*) kstat_data_lookup(ks, "ifspeed");
                                if (ifspeed)
                                {
                                    iface->speed = (int32_t) (ifspeed->value.ui64 / 1000 / 1000);
                                    FF_DEBUG("Interface %s speed: %d Mbps (kstat)", iface->name.chars, iface->speed);
                                }
                            }
                            break;
                        }
                    }
                }
                #endif
            }
        }
        else
        {
            FF_DEBUG("Failed to create socket for interface property retrieval");
        }
    }

    FF_DEBUG("Local IP detection completed, found %u interfaces", results->length);
    return NULL;
}
