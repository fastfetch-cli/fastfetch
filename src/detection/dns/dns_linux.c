#include "detection/dns/dns.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

const char* ffDetectDNS(FFDNSOptions* options, FFlist* results)
{
    FF_AUTO_CLOSE_FILE FILE* file = fopen(FASTFETCH_TARGET_DIR_ROOT "/etc/resolv.conf", "r");
    if (!file)
        return "fopen (" FASTFETCH_TARGET_DIR_ROOT "/etc/resolv.conf) failed";

    FF_AUTO_FREE char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1)
    {
        if (ffStrStartsWith(line, "nameserver"))
        {
            char* nameserver = line + strlen("nameserver");
            while (*nameserver == ' ' || *nameserver == '\t')
                nameserver++;
            if (*nameserver == '\0') continue;

            char* comment = strchr(nameserver, '#');
            if (comment) *comment = '\0';

            if ((ffStrContainsC(nameserver, ':') && !(options->showType & FF_DNS_TYPE_IPV6_BIT)) ||
                (ffStrContainsC(nameserver, '.') && !(options->showType & FF_DNS_TYPE_IPV4_BIT)))
                continue;

            FFstrbuf* item = (FFstrbuf*) ffListAdd(results);
            ffStrbufInitS(item, nameserver);
            ffStrbufTrimRightSpace(item);
        }
    }
    return NULL;
}
