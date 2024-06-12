#include "detection/dns/dns.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

const char* ffDetectDNS(FFlist* results)
{
    FF_AUTO_CLOSE_FILE FILE* file = fopen(FASTFETCH_TARGET_DIR_ROOT "/etc/resolv.conf", "r");
    if (!file)
        return "fopen (" FASTFETCH_TARGET_DIR_ROOT "/etc/resolv.conf) failed";

    FF_AUTO_FREE char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1)
    {
        if (ffStrStartsWith(line, "nameserver "))
        {
            FFstrbuf* item = (FFstrbuf*) ffListAdd(results);
            ffStrbufInitS(item, line + strlen("nameserver "));
            ffStrbufTrimRightSpace(item);
            ffStrbufTrimLeft(item, ' ');
        }
    }
    return NULL;
}
