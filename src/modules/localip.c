#include "fastfetch.h"

#define FF_LOCALIP_MODULE_NAME "Local IP"
#define FF_LOCALIP_NUM_FORMAT_ARGS 1

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

static void printValue(FFinstance* instance, const char* ifaName, const char* addressBuffer)
{
    FFstrbuf key;
    ffStrbufInit(&key);

    if (instance->config.localIP.key.length == 0)
    {
        ffStrbufAppendF(&key, FF_LOCALIP_MODULE_NAME " (%s)", ifaName);
    }
    else
    {
        ffParseFormatString(&key, &instance->config.localIP.key, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, ifaName}
        });
    }

    if (instance->config.localIP.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, key.chars, 0, NULL);
        puts(addressBuffer);
    }
    else
    {
        ffPrintFormatString(instance, key.chars, 0, NULL, &instance->config.localIP.outputFormat, FF_LOCALIP_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, addressBuffer}
        });
    }

    ffStrbufDestroy(&key);
}

void ffPrintLocalIp(FFinstance* instance)
{
    struct ifaddrs* ifAddrStruct = NULL;
    int ret = getifaddrs(&ifAddrStruct);
    if (ret < 0) {
        ffPrintError(instance, FF_LOCALIP_MODULE_NAME, 0, &instance->config.localIP, "getifaddrs(&ifAddrStruct) < 0 (%i)", ret);
        return;
    }

    for (struct ifaddrs* ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        // loop back
        if (strcmp(ifa->ifa_name, "lo") == 0 && !instance->config.localIpShowLoop)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            if (!instance->config.localIpShowIpV4)
                continue;

            void* tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printValue(instance, ifa->ifa_name, addressBuffer);
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            if (!instance->config.localIpShowIpV6)
                continue;

            void* tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printValue(instance, ifa->ifa_name, addressBuffer);
        }
    }

    if (ifAddrStruct) freeifaddrs(ifAddrStruct);
}
