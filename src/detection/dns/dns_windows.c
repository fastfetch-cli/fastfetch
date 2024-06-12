#include "detection/dns/dns.h"
#include "util/mallocHelper.h"

#include <iphlpapi.h>

const char* ffDetectDNS(FFlist* results)
{
    FF_AUTO_FREE FIXED_INFO* fixedInfo = malloc(sizeof(FIXED_INFO));
    ULONG size = sizeof(fixedInfo);
    while (true)
    {
        DWORD res = GetNetworkParams(fixedInfo, &size);
        if (res == ERROR_BUFFER_OVERFLOW)
        {
            fixedInfo = realloc(fixedInfo, size);
            continue;
        }
        else if (res != ERROR_SUCCESS)
            return "GetNetworkParams() failed";
        break;
    }

    for (IP_ADDR_STRING* dns = &fixedInfo->DnsServerList; dns; dns = dns->Next)
    {
        FFstrbuf* item = (FFstrbuf*) ffListAdd(results);
        ffStrbufInitS(item, dns->IpAddress.String);
        ffStrbufTrimRightSpace(item);
    }
    return NULL;
}
