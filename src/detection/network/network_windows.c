#include "detection/network/network.h"
#include "util/windows/unicode.h"
#include "util/mallocHelper.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <wchar.h>

const char* ffDetectNetwork(FFinstance* instance, FFlist* result)
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
            AF_UNSPEC,
            GAA_FLAG_SKIP_ANYCAST |
            GAA_FLAG_SKIP_MULTICAST |
            GAA_FLAG_SKIP_DNS_SERVER |
            GAA_FLAG_SKIP_FRIENDLY_NAME,
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
        if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
            continue;

        bool isOn = adapter->OperStatus == IfOperStatusUp;
        if (!isOn && !instance->config.networkAll)
            continue;

        const char* strType = NULL;
        switch (adapter->IfType)
        {
            case IF_TYPE_ETHERNET_CSMACD:
                strType = "Ethernet";
                break;
            case IF_TYPE_ISO88025_TOKENRING:
                strType = "TokenRing";
                break;
            case IF_TYPE_PPP:
                strType = "PPP";
                break;
            case IF_TYPE_ATM:
                strType = "ATM";
                break;
            case IF_TYPE_IEEE80211:
                strType = "IEEE80211";
                break;
            case IF_TYPE_TUNNEL:
                strType = "Tunnel";
                break;
            case IF_TYPE_IEEE1394:
                strType = "Fireware";
                break;
            case IF_TYPE_MODEM:
                strType = "Modem";
                break;
            default:
                strType = "Other";
                break;
        }

        if (instance->config.networkType.length && !ffStrbufContainIgnCaseS(&instance->config.networkType, strType))
            continue;

        FFNetworkResult* item = (FFNetworkResult*) ffListAdd(result);
        ffStrbufInitS(&item->type, strType);
        ffStrbufInit(&item->name);
        ffStrbufInit(&item->address);
        item->mtu = (int32_t) adapter->Mtu;
        item->on = isOn;

        ffStrbufSetWS(&item->name, adapter->FriendlyName);
        for (ULONG i = 0; i < adapter->PhysicalAddressLength; i++)
            ffStrbufAppendF(&item->address, "%.2X-", (int) adapter->PhysicalAddress[i]);
        ffStrbufTrimRight(&item->address, '-');
    }

    if (result->length == 0)
        return "No network interfaces found";

    return NULL;
}
