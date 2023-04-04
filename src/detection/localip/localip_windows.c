#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "util/mallocHelper.h"
#include "util/windows/unicode.h"
#include "localip.h"

static void addNewIp(FFlist* list, const char* name, const char* value, int type, bool newIp)
{
    FFLocalIpResult* ip = NULL;

    if (newIp)
    {
        ip = (FFLocalIpResult*) ffListAdd(list);
        ffStrbufInitS(&ip->name, name);
        ffStrbufInit(&ip->ipv4);
        ffStrbufInit(&ip->ipv6);
        ffStrbufInit(&ip->mac);
    }
    else
    {
        ip = ffListGet(list, list->length - 1);
    }

    switch (type)
    {
        case AF_INET:
            ffStrbufSetS(&ip->ipv4, value);
            break;
        case AF_INET6:
            ffStrbufSetS(&ip->ipv6, value);
            break;
        case -1:
            ffStrbufSetS(&ip->mac, value);
            break;
    }
}

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results)
{
    IP_ADAPTER_ADDRESSES* FF_AUTO_FREE adapter_addresses = NULL;

    // Start with a 16 KB buffer and resize if needed -
    // multiple attempts in case interfaces change while
    // we are in the middle of querying them.
    DWORD adapter_addresses_buffer_size = 16 * 1024;
    for (int attempts = 0; attempts != 3; ++attempts)
    {
        adapter_addresses = (IP_ADAPTER_ADDRESSES*)realloc(adapter_addresses, adapter_addresses_buffer_size);
        assert(adapter_addresses);

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
        else if (ERROR_BUFFER_OVERFLOW == error)
            continue;
        else
            return "GetAdaptersAddresses() failed";
    }

    // Iterate through all of the adapters
    for (IP_ADAPTER_ADDRESSES* adapter = adapter_addresses; adapter; adapter = adapter->Next)
    {
        bool isLoop = adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK;
        if (isLoop && !(options->showType & FF_LOCALIP_TYPE_LOOP_BIT))
            continue;

        bool newIp = true;

        char name[128];
        WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1, name, sizeof(name), NULL, NULL);
        if (options->namePrefix.length && strncmp(name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        if (options->showType & FF_LOCALIP_TYPE_MAC_BIT && adapter->PhysicalAddressLength == 6)
        {
            char addressBuffer[32];
            uint8_t* ptr = adapter->PhysicalAddress;
            snprintf(addressBuffer, sizeof(addressBuffer), "%02x:%02x:%02x:%02x:%02x:%02x",
                        ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            addNewIp(results, name, addressBuffer, -1, newIp);
            newIp = false;
        }

        for (IP_ADAPTER_UNICAST_ADDRESS* ifa = adapter->FirstUnicastAddress; ifa; ifa = ifa->Next)
        {
            if (ifa->Address.lpSockaddr->sa_family == AF_INET)
            {
                SOCKADDR_IN* ipv4 = (SOCKADDR_IN*) ifa->Address.lpSockaddr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ipv4->sin_addr, addressBuffer, INET_ADDRSTRLEN);
                addNewIp(results, name, addressBuffer, AF_INET, newIp);
                newIp = false;
            }
            else if (ifa->Address.lpSockaddr->sa_family == AF_INET6)
            {
                SOCKADDR_IN6* ipv6 = (SOCKADDR_IN6*) ifa->Address.lpSockaddr;
                char addressBuffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &ipv6->sin6_addr, addressBuffer, INET6_ADDRSTRLEN);
                addNewIp(results, name, addressBuffer, AF_INET6, newIp);
                newIp = false;
            }
        }
    }

    return NULL;
}
