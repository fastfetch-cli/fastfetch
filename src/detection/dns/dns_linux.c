#include "detection/dns/dns.h"

#include <arpa/inet.h>
#include <resolv.h>

const char* ffDetectDNS(FFlist* results)
{
    if (res_init() < 0)
        return "res_init() failed";

    for (int i = 0; i < _res.nscount; ++i)
    {
        struct sockaddr_in* addr = &_res.nsaddr_list[i];
        char addressBuffer[INET_ADDRSTRLEN + 4];
        inet_ntop(AF_INET, &addr->sin_addr, addressBuffer, INET_ADDRSTRLEN);

        FFstrbuf* result = (FFstrbuf*) ffListAdd(results);
        ffStrbufInitS(result, addressBuffer);
    }

    for (int i = 0; i < _res._u._ext.nscount; ++i)
    {
        struct sockaddr_in6* addr = _res._u._ext.nsaddrs[i];
        char addressBuffer[INET6_ADDRSTRLEN + 4];
        inet_ntop(AF_INET6, &addr->sin6_addr, addressBuffer, INET6_ADDRSTRLEN);

        FFstrbuf* result = (FFstrbuf*) ffListAdd(results);
        ffStrbufInitS(result, addressBuffer);
    }

    return NULL;
}
