#include "netif.h"
#include "util/mallocHelper.h"

#include <iphlpapi.h>

bool ffNetifGetDefaultRoute(uint32_t* ifIndex)
{
    ULONG size = 0;
    if (GetIpForwardTable(NULL, &size, TRUE) != ERROR_INSUFFICIENT_BUFFER)
        return false;

    FF_AUTO_FREE MIB_IPFORWARDTABLE* pIpForwardTable = (MIB_IPFORWARDTABLE*) malloc(size);
    if (GetIpForwardTable(pIpForwardTable, &size, TRUE) != ERROR_SUCCESS)
        return false;

    for (uint32_t i = 0; i < pIpForwardTable->dwNumEntries; ++i)
    {
        MIB_IPFORWARDROW* ipForwardRow = &pIpForwardTable->table[i];
        if (ipForwardRow->dwForwardDest == 0 && ipForwardRow->dwForwardMask == 0)
        {
            *ifIndex = ipForwardRow->dwForwardIfIndex;
            break;
        }
    }

    return true;
}
