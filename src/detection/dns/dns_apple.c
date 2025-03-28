#include "detection/dns/dns.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"
#include "util/apple/cf_helpers.h"
#include "util/debug.h"

#include <SystemConfiguration/SystemConfiguration.h>

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
    // Handle macOS-specific DNS configurations
    FF_DEBUG("Using SystemConfiguration framework for macOS");

    // Create a reference to the dynamic store
    FF_CFTYPE_AUTO_RELEASE SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("fastfetch"), NULL, NULL);
    if (store)
    {
        // Get the network global IPv4 and IPv6 configuration
        FF_CFTYPE_AUTO_RELEASE CFStringRef key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetDNS);
        if (key)
        {
            FF_CFTYPE_AUTO_RELEASE CFDictionaryRef dict = SCDynamicStoreCopyValue(store, key);
            if (dict)
            {
                // Get the DNS server addresses array
                CFArrayRef dnsServers = CFDictionaryGetValue(dict, kSCPropNetDNSServerAddresses);

                if (dnsServers)
                {
                    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
                    for (CFIndex i = 0; i < CFArrayGetCount(dnsServers); i++)
                    {
                        if (ffCfStrGetString(CFArrayGetValueAtIndex(dnsServers, i), &buffer) == NULL)
                        {
                            // Check if the address matches our filter
                            if ((ffStrbufContainC(&buffer, ':') && !(options->showType & FF_DNS_TYPE_IPV6_BIT)) ||
                                (ffStrbufContainC(&buffer, '.') && !(options->showType & FF_DNS_TYPE_IPV4_BIT)))
                                continue;

                            // Add to results
                            FFstrbuf* item = (FFstrbuf*) ffListAdd(results);
                            ffStrbufInitMove(item, &buffer);
                            FF_DEBUG("Found DNS server on macOS: %s", item->chars);
                        }
                    }
                }
            }
        }
    }

    // If we didn't find any servers, try resolv.conf as fallback
    if (results->length > 0)
        return NULL;

    FF_DEBUG("No DNS servers found via SystemConfiguration, trying resolv.conf");
    // Try standard resolv.conf location on macOS as a fallback
    return detectDnsFromConf("/var/run/resolv.conf", options, results);
}
