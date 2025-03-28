#include "detection/dns/dns.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"
#include "util/debug.h"

#ifdef __HAIKU__
#define RESOLV_CONF "/system/settings/network/resolv.conf"
#else
#define RESOLV_CONF "/etc/resolv.conf"
#endif

static const char* detectDnsFromConf(const char* path, FFDNSOptions* options, FFlist* results)
{
    FF_DEBUG("Attempting to read DNS config from %s", path);

    FF_AUTO_CLOSE_FILE FILE* file = fopen(path, "r");
    if (!file)
    {
        FF_DEBUG("Failed to open %s: %m", path);
        return "fopen(path, r) failed";
    }

    if (results->length > 0)
    {
        FF_DEBUG("Clearing existing DNS entries (%u entries)", results->length);
        FF_LIST_FOR_EACH(FFstrbuf, item, *results)
            ffStrbufDestroy(item);
        ffListClear(results);
    }

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
            FF_DEBUG("Found DNS server: %s", item->chars);
        }
    }

    FF_DEBUG("Found %u DNS servers in %s", results->length, path);
    return NULL;
}

const char* ffDetectDNS(FFDNSOptions* options, FFlist* results)
{
    FF_DEBUG("Starting DNS detection");

    const char* error = detectDnsFromConf(FASTFETCH_TARGET_DIR_ROOT RESOLV_CONF, options, results);
    if (error != NULL)
    {
        FF_DEBUG("Error detecting DNS: %s", error);
        return error;
    }

    #if __linux__ && !__ANDROID__
    // Handle different DNS management services
    if (results->length == 1)
    {
        const FFstrbuf* firstEntry = FF_LIST_GET(FFstrbuf, *results, 0);

        if (ffStrbufEqualS(firstEntry, "127.0.0.53"))
        {
            FF_DEBUG("Detected systemd-resolved (127.0.0.53), checking actual DNS servers");
            // Managed by systemd-resolved
            if (detectDnsFromConf("/run/systemd/resolve/resolv.conf", options, results) == NULL)
                return NULL;
        }
        else if (ffStrbufEqualS(firstEntry, "127.0.0.1"))
        {
            FF_DEBUG("Detected possible NetworkManager (127.0.0.1), checking actual DNS servers");
            // Managed by NetworkManager
            if (detectDnsFromConf("/var/run/NetworkManager/resolv.conf", options, results) == NULL)
                return NULL;
        }
    }

    // Check other possible DNS configuration files
    if (results->length == 0)
    {
        FF_DEBUG("No DNS servers found, trying alternative config files");

        // Try resolvconf
        FF_DEBUG("Trying resolvconf configuration");
        if (detectDnsFromConf(FASTFETCH_TARGET_DIR_ROOT "/run/resolvconf/resolv.conf", options, results) == NULL && results->length > 0)
            return NULL;

        // Try dnsmasq
        FF_DEBUG("Trying dnsmasq configuration");
        if (detectDnsFromConf(FASTFETCH_TARGET_DIR_ROOT "/var/run/dnsmasq/resolv.conf", options, results) == NULL && results->length > 0)
            return NULL;

        // Try openresolv
        FF_DEBUG("Trying openresolv configuration");
        if (detectDnsFromConf(FASTFETCH_TARGET_DIR_ROOT "/etc/resolv.conf.openresolv", options, results) == NULL && results->length > 0)
            return NULL;
    }
    #elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // Handle BSD-specific DNS configurations
    if (results->length == 0)
    {
        FF_DEBUG("No DNS servers found, trying BSD-specific config files");

        // FreeBSD and other BSDs may use resolvconf service
        FF_DEBUG("Trying BSD resolvconf configuration");
        if (detectDnsFromConf(FASTFETCH_TARGET_DIR_ROOT "/var/run/resolvconf/resolv.conf", options, results) == NULL && results->length > 0)
            return NULL;

        // Some BSDs store DNS configuration here
        FF_DEBUG("Trying BSD nameserver configuration");
        if (detectDnsFromConf(FASTFETCH_TARGET_DIR_ROOT "/var/run/nameserver", options, results) == NULL && results->length > 0)
            return NULL;

        // Try common BSD paths
        FF_DEBUG("Trying BSD common paths");
        if (detectDnsFromConf(FASTFETCH_TARGET_DIR_ROOT "/etc/nameserver", options, results) == NULL && results->length > 0)
            return NULL;
    }
    #endif

    FF_DEBUG("DNS detection completed with %u servers found", results->length);
    return NULL;
}
