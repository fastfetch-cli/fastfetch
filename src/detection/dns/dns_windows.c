#include "detection/dns/dns.h"
#include "common/netif.h"
#include "common/debug.h"
#include "common/windows/registry.h"

#include <ws2tcpip.h>
#include <iphlpapi.h>

// The resolvers of an interface are stored in the registry, under a key named after its GUID:
//   HKLM\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces\{GUID}    IPv4
//   HKLM\SYSTEM\CurrentControlSet\Services\Tcpip6\Parameters\Interfaces\{GUID}   IPv6
// A statically configured `NameServer` takes precedence over the value handed out by DHCP, which is
// the precedence the IP stack applies as well - verified against GetAdaptersAddresses() on a machine
// with both set, where only the static list was reported.
//
// This replaces GetAdaptersAddresses(), which builds the unicast, anycast, multicast and DNS server
// lists of every adapter - about 9 ms on a machine with 52 of them - even though the module only
// reports the resolvers of the interface that carries the default route. Reading the one key that
// matters costs about 16 us (12 us to open it, 4 us to read the value, see
// src/common/windows/registry.c), and the module as a whole goes from about 11 ms to about 0.3 ms.
//
// Known limitation: a resolver learned from a router advertisement (RDNSS) is not written to the
// registry by any Windows version, so on a SLAAC-only network (no DHCPv6 lease, no static
// `NameServer`) IPv6 resolvers are not reported at all. GetAdaptersAddresses() reported them.
#define FF_DNS_REGISTRY_KEY_V4 L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\"
#define FF_DNS_REGISTRY_KEY_V6 L"SYSTEM\\CurrentControlSet\\Services\\Tcpip6\\Parameters\\Interfaces\\"

// Enough for either key path plus the 38 characters of a GUID
#define FF_DNS_REGISTRY_PATH_SIZE 256

static void appendAddress(FFlist* results, FFstrbuf* servers, uint32_t start, uint32_t end, FFDNSShowType showType) {
    // A link local address may carry a zone index, which the reported address never includes
    char address[INET6_ADDRSTRLEN];
    uint32_t length = end - start;
    if (length >= sizeof(address)) {
        length = sizeof(address) - 1;
    }

    uint32_t i = 0;
    for (; i < length && servers->chars[start + i] != '%'; ++i) {
        address[i] = servers->chars[start + i];
    }
    address[i] = '\0';

    if (showType & FF_DNS_TYPE_IPV4_BIT) {
        IN_ADDR ipv4;
        if (InetPtonA(AF_INET, address, &ipv4) == 1) {
            FFstrbuf* item = FF_LIST_ADD(FFstrbuf, *results);
            ffStrbufInitA(item, INET_ADDRSTRLEN);
            item->length = (uint32_t) (RtlIpv4AddressToStringA(&ipv4, item->chars) - item->chars);
            return;
        }
    }

    if (showType & FF_DNS_TYPE_IPV6_BIT) {
        IN6_ADDR ipv6;
        if (InetPtonA(AF_INET6, address, &ipv6) == 1) {
            FFstrbuf* item = FF_LIST_ADD(FFstrbuf, *results);
            ffStrbufInitA(item, INET6_ADDRSTRLEN);
            item->length = (uint32_t) (RtlIpv6AddressToStringA(&ipv6, item->chars) - item->chars);
        }
    }
}

static void appendAddressList(FFstrbuf* servers, FFDNSShowType showType, FFlist* results) {
    uint32_t start = 0;
    while (start < servers->length) {
        while (start < servers->length && (servers->chars[start] == ',' || servers->chars[start] == ' ' || servers->chars[start] == '\t')) {
            ++start;
        }

        uint32_t end = start;
        while (end < servers->length && servers->chars[end] != ',' && servers->chars[end] != ' ' && servers->chars[end] != '\t') {
            ++end;
        }

        if (end > start) {
            appendAddress(results, servers, start, end, showType);
        }
        start = end;
    }
}

// A DHCPv6 lease stores its resolvers as a plain list of 16 byte addresses
static void appendAddressArray(const BYTE* data, uint32_t dataLength, FFDNSShowType showType, FFlist* results) {
    if (!(showType & FF_DNS_TYPE_IPV6_BIT)) {
        return;
    }

    for (uint32_t offset = 0; offset + sizeof(IN6_ADDR) <= dataLength; offset += sizeof(IN6_ADDR)) {
        IN6_ADDR ipv6;
        memcpy(&ipv6, data + offset, sizeof(ipv6));

        FFstrbuf* item = FF_LIST_ADD(FFstrbuf, *results);
        ffStrbufInitA(item, INET6_ADDRSTRLEN);
        item->length = (uint32_t) (RtlIpv6AddressToStringA(&ipv6, item->chars) - item->chars);
    }
}

// `Dhcpv6InterfaceOptions` stores the DHCPv6 options of the last lease as an array of records: a 20
// byte header (option code, 0, payload length, 0, 0xFFFFFFFF) followed by the payload, padded to 16
// bytes. Option 23 is OPTION_DNS_SERVERS (RFC 3646).
static void appendDhcpv6Options(const BYTE* data, uint32_t dataLength, FFDNSShowType showType, FFlist* results) {
    for (uint32_t offset = 0; offset + 20 <= dataLength;) {
        uint32_t code, length;
        memcpy(&code, data + offset, sizeof(code));
        memcpy(&length, data + offset + 8, sizeof(length));

        uint32_t payloadLength = dataLength - (offset + 20);
        if (payloadLength > length) {
            payloadLength = length;
        }

        if (code == 23) {
            appendAddressArray(data + offset + 20, payloadLength, showType, results);
            return;
        }

        if (payloadLength < length) {
            return;
        }
        offset += 20 + ((payloadLength + 15) & ~(uint32_t) 15);
    }
}

static void detectV4(const wchar_t* guid, FFDNSOptions* options, FFlist* results) {
    wchar_t keyPath[FF_DNS_REGISTRY_PATH_SIZE] = FF_DNS_REGISTRY_KEY_V4;
    wcscat(keyPath, guid);
    FF_DEBUG("v4 key '%ls'", keyPath);

    FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, keyPath, &hKey, nullptr)) {
        FF_DEBUG("v4 key open failed");
        return;
    }

    FF_STRBUF_AUTO_DESTROY servers = ffStrbufCreate();
    // `NameServer` holds the statically configured list and wins over the one handed out by DHCP. A
    // value that exists but holds no address must not shadow the fallback, hence the length checks.
    bool got = (ffRegReadStrbuf(hKey, L"NameServer", &servers, nullptr) && servers.length) ||
        (ffRegReadStrbuf(hKey, L"DhcpNameServer", &servers, nullptr) && servers.length);
    FF_DEBUG("v4 got=%d servers='%s'", got, servers.chars);
    if (!got) {
        return;
    }

    appendAddressList(&servers, options->showType, results);
    FF_DEBUG("v4 results=%u", results->length);
}

static void detectV6(const wchar_t* guid, FFDNSOptions* options, FFlist* results) {
    wchar_t keyPath[FF_DNS_REGISTRY_PATH_SIZE] = FF_DNS_REGISTRY_KEY_V6;
    wcscat(keyPath, guid);
    FF_DEBUG("v6 key '%ls'", keyPath);

    FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, keyPath, &hKey, nullptr)) {
        FF_DEBUG("v6 key open failed");
        return;
    }

    FF_STRBUF_AUTO_DESTROY servers = ffStrbufCreate();
    if (ffRegReadStrbuf(hKey, L"NameServer", &servers, nullptr) && servers.length) {
        FF_DEBUG("v6 servers='%s'", servers.chars);
        appendAddressList(&servers, options->showType, results);
        FF_DEBUG("v6 results=%u", results->length);
        return;
    }

    // The two DHCPv6 values below are documented as REG_BINARY and are always written that way; the
    // registry reader reports the bytes without their type, so the parsing below trusts that.
    BYTE data[4096];

    FFArgBuffer dhcpv6Servers = { .data = data, .length = sizeof(data) };
    if (ffRegReadData(hKey, L"Dhcpv6DNSServers", &dhcpv6Servers, nullptr)) {
        FF_DEBUG("v6 Dhcpv6DNSServers cb=%u", dhcpv6Servers.length);
        appendAddressArray(data, dhcpv6Servers.length, options->showType, results);
        return;
    }

    FFArgBuffer dhcpv6Options = { .data = data, .length = sizeof(data) };
    if (ffRegReadData(hKey, L"Dhcpv6InterfaceOptions", &dhcpv6Options, nullptr)) {
        FF_DEBUG("v6 Dhcpv6InterfaceOptions cb=%u", dhcpv6Options.length);
        appendDhcpv6Options(data, dhcpv6Options.length, options->showType, results);
        return;
    }

    FF_DEBUG("v6 no resolver");
}

const char* ffDetectDNS(FFDNSOptions* options, FFlist* results) {
    FF_DEBUG("enter, showType=%u", (unsigned) options->showType);
    // The interface that carries the default route is the one the IP stack resolves names through.
    // ffNetifGetDefaultRouteV4() already ignores the interfaces that are not operational.
    NET_IFINDEX ifIndex = (NET_IFINDEX) ffNetifGetDefaultRouteV4()->ifIndex;
    FF_DEBUG("ifIndex=%u", (unsigned) ifIndex);
    if (!ifIndex) {
        ifIndex = (NET_IFINDEX) ffNetifGetDefaultRouteV6()->ifIndex;
    }
    if (!ifIndex) {
        return "Failed to detect default route";
    }

    NET_LUID luid;
    if (ConvertInterfaceIndexToLuid(ifIndex, &luid) != NO_ERROR) {
        return "ConvertInterfaceIndexToLuid() failed";
    }

    GUID guid;
    if (ConvertInterfaceLuidToGuid(&luid, &guid) != NO_ERROR) {
        return "ConvertInterfaceLuidToGuid() failed";
    }

    wchar_t guidStr[64];
    // clang-format off
    swprintf(guidStr, ARRAY_SIZE(guidStr), L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        (unsigned long) guid.Data1,
        (unsigned int) guid.Data2,
        (unsigned int) guid.Data3,
        (unsigned int) guid.Data4[0],
        (unsigned int) guid.Data4[1],
        (unsigned int) guid.Data4[2],
        (unsigned int) guid.Data4[3],
        (unsigned int) guid.Data4[4],
        (unsigned int) guid.Data4[5],
        (unsigned int) guid.Data4[6],
        (unsigned int) guid.Data4[7]);
    // clang-format on

    // IPv4 first, so the list is grouped by address family the same way the printed output is
    if (options->showType & FF_DNS_TYPE_IPV4_BIT) {
        detectV4(guidStr, options, results);
    }

    if (options->showType & FF_DNS_TYPE_IPV6_BIT) {
        detectV6(guidStr, options, results);
    }

    FF_DEBUG("return, results=%u", results->length);
    return nullptr;
}
