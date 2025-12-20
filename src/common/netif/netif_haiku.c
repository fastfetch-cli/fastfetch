#include "netif.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <stdio.h>

// loosely based on Haiku's src/bin/network/route/route.cpp

bool ffNetifGetDefaultRouteImplV4(FFNetifDefaultRouteResult* result)
{
    FF_AUTO_CLOSE_FD int pfRoute = socket(AF_INET, SOCK_RAW, AF_INET);
    if (pfRoute < 0)
        return false;

    struct ifconf config;
    config.ifc_len = sizeof(config.ifc_value);
    if (ioctl(pfRoute, SIOCGRTSIZE, &config, sizeof(struct ifconf)) < 0)
        return false;

    int size = config.ifc_value;
    if (size <= 0)
        return false;

    FF_AUTO_FREE void *buffer = malloc((size_t) size);
    if (buffer == NULL) {
        return false;
    }

    config.ifc_len = size;
    config.ifc_buf = buffer;
    if (ioctl(pfRoute, SIOCGRTTABLE, &config, sizeof(struct ifconf)) < 0)
        return false;

    struct ifreq *interface = (struct ifreq*)buffer;
    struct ifreq *end = (struct ifreq*)((uint8_t*)buffer + size);

    while (interface < end) {
        if (interface->ifr_route.flags & RTF_DEFAULT) {
            // interface->ifr_metric?
            strlcpy(result->ifName, interface->ifr_name, IF_NAMESIZE);
            result->ifIndex = if_nametoindex(interface->ifr_name);
            if (interface->ifr_route.source)
                result->preferredSourceAddrV4 = ((struct sockaddr_in*)interface->ifr_route.source)->sin_addr.s_addr;
            return true;
        }

        size_t addressSize = 0;
        if (interface->ifr_route.destination != NULL)
            addressSize += interface->ifr_route.destination->sa_len;
        if (interface->ifr_route.mask != NULL)
            addressSize += interface->ifr_route.mask->sa_len;
        if (interface->ifr_route.gateway != NULL)
            addressSize += interface->ifr_route.gateway->sa_len;

        interface = (struct ifreq*)((addr_t)interface + IF_NAMESIZE + sizeof(struct route_entry) + addressSize);
    }

    return false;
}

bool ffNetifGetDefaultRouteImplV6(FFNetifDefaultRouteResult* result)
{
    FF_AUTO_CLOSE_FD int pfRoute = socket(AF_INET, SOCK_RAW, AF_INET6);
    if (pfRoute < 0)
        return false;

    struct ifconf config;
    config.ifc_len = sizeof(config.ifc_value);
    if (ioctl(pfRoute, SIOCGRTSIZE, &config, sizeof(struct ifconf)) < 0)
        return false;

    int size = config.ifc_value;
    if (size <= 0)
        return false;

    FF_AUTO_FREE void *buffer = malloc((size_t) size);
    if (buffer == NULL) {
        return false;
    }

    config.ifc_len = size;
    config.ifc_buf = buffer;
    if (ioctl(pfRoute, SIOCGRTTABLE, &config, sizeof(struct ifconf)) < 0)
        return false;

    struct ifreq *interface = (struct ifreq*)buffer;
    struct ifreq *end = (struct ifreq*)((uint8_t*)buffer + size);

    while (interface < end) {
        if (interface->ifr_route.flags & RTF_DEFAULT) {
            strlcpy(result->ifName, interface->ifr_name, IF_NAMESIZE);
            result->ifIndex = if_nametoindex(interface->ifr_name);
            return true;
        }

        size_t addressSize = 0;
        if (interface->ifr_route.destination != NULL)
            addressSize += interface->ifr_route.destination->sa_len;
        if (interface->ifr_route.mask != NULL)
            addressSize += interface->ifr_route.mask->sa_len;
        if (interface->ifr_route.gateway != NULL)
            addressSize += interface->ifr_route.gateway->sa_len;

        interface = (struct ifreq*)((addr_t)interface + IF_NAMESIZE + sizeof(struct route_entry) + addressSize);
    }
    return false;
}
