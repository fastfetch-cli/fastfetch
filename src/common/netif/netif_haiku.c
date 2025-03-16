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

bool ffNetifGetDefaultRouteImpl(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex)
{
    // TODO: AF_INET6
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
            strlcpy(iface, interface->ifr_name, IF_NAMESIZE);
            *ifIndex = if_nametoindex(interface->ifr_name);
            return true;
        }

        size_t addressSize = 0;
        if (interface->ifr_route.destination != NULL)
            addressSize += interface->ifr_route.destination->sa_len;
        if (interface->ifr_route.mask != NULL)
            addressSize += interface->ifr_route.mask->sa_len;
        if (interface->ifr_route.gateway != NULL)
            addressSize += interface->ifr_route.gateway->sa_len;

        interface = (struct ifreq*)((addr_t)interface + IF_NAMESIZE
            + sizeof(struct route_entry) + addressSize);
    }

    return false;
}
