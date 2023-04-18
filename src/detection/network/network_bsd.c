#include "common/io/io.h"
#include "detection/network/network.h"

#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <ifaddrs.h>

const char* ffDetectNetwork(FF_MAYBE_UNUSED FFinstance* instance, FFlist* result)
{
    struct ifaddrs* ifAddrStruct = NULL;
    if(getifaddrs(&ifAddrStruct) < 0)
        return "getifaddrs(&ifAddrStruct) failed";

    int FF_AUTO_CLOSE_FD sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    for (struct ifaddrs* ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_LINK)
            continue;

        if (ifa->ifa_flags & IFF_LOOPBACK || !(ifa->ifa_flags & IFF_RUNNING))
            continue;

        bool isUp = !!(ifa->ifa_flags & IFF_UP);
        if (!isUp && !instance->config.networkAll)
            continue;

        FFNetworkResult* item = (FFNetworkResult*) ffListAdd(result);
        ffStrbufInit(&item->type);
        ffStrbufInitS(&item->name, ifa->ifa_name);

        uint8_t* ptr = (uint8_t*) LLADDR((struct sockaddr_dl *)ifa->ifa_addr);
        ffStrbufInitF(&item->address, "%02x:%02x:%02x:%02x:%02x:%02x",
                      ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);

        if (sockfd > 0)
        {
            struct ifreq ifr;
            strcpy(ifr.ifr_name, ifa->ifa_name);
            item->mtu = ioctl(sockfd, SIOCGIFMTU, &ifr) >= 0 ? (uint32_t) ifr.ifr_mtu : 0;
        }

        item->up = isUp;
    }

    freeifaddrs(ifAddrStruct);

    return NULL;
}
