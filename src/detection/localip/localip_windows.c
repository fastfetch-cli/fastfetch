#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "common/netif.h"
#include "common/mallocHelper.h"
#include "common/windows/unicode.h"
#include "common/debug.h"
#include "localip.h"

#define FF_LOCALIP_NIFLAG(name) { IP_ADAPTER_##name, #name }

static const FFLocalIpNIFlag niFlagOptions[] = {
    FF_LOCALIP_NIFLAG(DDNS_ENABLED),
    FF_LOCALIP_NIFLAG(REGISTER_ADAPTER_SUFFIX),
    FF_LOCALIP_NIFLAG(DHCP_ENABLED),
    FF_LOCALIP_NIFLAG(RECEIVE_ONLY),
    FF_LOCALIP_NIFLAG(NO_MULTICAST),
    FF_LOCALIP_NIFLAG(IPV6_OTHER_STATEFUL_CONFIG),
    FF_LOCALIP_NIFLAG(NETBIOS_OVER_TCPIP_ENABLED),
    FF_LOCALIP_NIFLAG(IPV4_ENABLED),
    FF_LOCALIP_NIFLAG(IPV6_ENABLED),
    FF_LOCALIP_NIFLAG(IPV6_MANAGE_ADDRESS_CONFIG),
    // sentinel
    {},
};

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results) {
    FF_DEBUG("Starting local IP detection with showType=0x%X, namePrefix='%.*s'",
        options->showType,
        (int) options->namePrefix.length,
        options->namePrefix.chars);

    ADDRESS_FAMILY family = options->showType & FF_LOCALIP_TYPE_IPV4_BIT
        ? options->showType & FF_LOCALIP_TYPE_IPV6_BIT ? AF_UNSPEC : AF_INET
        : AF_INET6;

    // GetAdaptersAddresses() is slow (several ms, more with many virtual adapters),
    // so it is only used for adapter flags, which are not available elsewhere
    FF_AUTO_FREE IP_ADAPTER_ADDRESSES* adapter_addresses = nullptr;
    if (options->showType & FF_LOCALIP_TYPE_FLAGS_BIT) {
        // Multiple attempts in case interfaces change while
        // we are in the middle of querying them.
        DWORD adapter_addresses_buffer_size = 15 * 1024; // recommended by Microsoft
        for (int attempts = 0;; ++attempts) {
            FF_DEBUG("Attempt %d to get adapter addresses, buffer size: %lu", attempts + 1, adapter_addresses_buffer_size);

            adapter_addresses = (IP_ADAPTER_ADDRESSES*) realloc(adapter_addresses, adapter_addresses_buffer_size);
            assert(adapter_addresses);

            DWORD error = GetAdaptersAddresses(
                family,
                GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME,
                nullptr,
                adapter_addresses,
                &adapter_addresses_buffer_size);

            if (error == ERROR_SUCCESS) {
                FF_DEBUG("GetAdaptersAddresses succeeded on attempt %d", attempts + 1);
                break;
            } else if (ERROR_BUFFER_OVERFLOW == error && attempts < 4) {
                FF_DEBUG("Buffer overflow, need %lu bytes, retrying", adapter_addresses_buffer_size);
                continue;
            } else {
                FF_DEBUG("GetAdaptersAddresses failed with error %lu after %d attempts", error, attempts + 1);
                return "GetAdaptersAddresses() failed";
            }
        }
    }

    PMIB_UNICASTIPADDRESS_TABLE table = nullptr;
    if (!NETIO_SUCCESS(GetUnicastIpAddressTable(family, &table))) {
        FF_DEBUG("GetUnicastIpAddressTable failed");
        return "GetUnicastIpAddressTable() failed";
    }

    [[maybe_unused]] int adapterCount = 0, processedCount = 0;

    // Rows of the same interface are not adjacent (IPv4 rows come first),
    // so each interface is handled at its first row
    for (ULONG i = 0; i < table->NumEntries; ++i) {
        NET_IFINDEX ifIndex = table->Table[i].InterfaceIndex;

        bool seen = false;
        for (ULONG j = 0; j < i && !seen; ++j) {
            seen = table->Table[j].InterfaceIndex == ifIndex;
        }
        if (seen) {
            continue;
        }

        adapterCount++;

        // Checked first to avoid querying interfaces we won't show
        if (options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT) {
            if (!((options->showType & FF_LOCALIP_TYPE_IPV4_BIT) && ffNetifGetDefaultRouteV4()->ifIndex == ifIndex) &&
                !((options->showType & FF_LOCALIP_TYPE_IPV6_BIT) && ffNetifGetDefaultRouteV6()->ifIndex == ifIndex)) {
                FF_DEBUG("Skipping interface %u (not default route interface)", (unsigned) ifIndex);
                continue;
            }
        }

        MIB_IF_ROW2 ifRow = {
            .InterfaceIndex = ifIndex,
        };
        if (!NETIO_SUCCESS(GetIfEntry2(&ifRow))) {
            FF_DEBUG("GetIfEntry2 failed for adapter %u", (unsigned) ifIndex);
            continue;
        }

        FF_DEBUG("Processing adapter %d: IfIndex=%u, IfType=%u, OperStatus=%u",
            adapterCount,
            (unsigned) ifIndex,
            (unsigned) ifRow.Type,
            (unsigned) ifRow.OperStatus);

        if (ifRow.OperStatus != IfOperStatusUp) {
            FF_DEBUG("Skipping adapter %u (not operational, status=%d)", (unsigned) ifIndex, ifRow.OperStatus);
            continue;
        }

        bool isLoop = ifRow.Type == IF_TYPE_SOFTWARE_LOOPBACK;
        FF_DEBUG("Adapter %u: isLoopback=%s", (unsigned) ifIndex, isLoop ? "true" : "false");

        if (isLoop && !(options->showType & FF_LOCALIP_TYPE_LOOP_BIT)) {
            FF_DEBUG("Skipping loopback adapter %u (loopback not requested)", (unsigned) ifIndex);
            continue;
        }

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateWS(ifRow.Alias);
        FF_DEBUG("Adapter %u name: '%s'", (unsigned) ifIndex, name.chars);

        if (options->namePrefix.length && !ffStrbufStartsWith(&name, &options->namePrefix)) {
            FF_DEBUG("Skipping adapter %u (name doesn't match prefix '%.*s')",
                (unsigned) ifIndex,
                (int) options->namePrefix.length,
                options->namePrefix.chars);
            continue;
        }

        processedCount++;
        FF_DEBUG("Creating result item for adapter %u ('%s')", (unsigned) ifIndex, name.chars);

        FFLocalIpResult* item = FF_LIST_ADD(FFLocalIpResult, *results);
        ffStrbufInitMove(&item->name, &name);
        ffStrbufInit(&item->ipv4);
        ffStrbufInit(&item->ipv6);
        ffStrbufInit(&item->mac);
        ffStrbufInit(&item->flags);
        item->defaultRoute = FF_LOCALIP_TYPE_NONE;
        item->speed = -1;
        item->mtu = -1;

        uint32_t typesToAdd = options->showType & (FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_IPV6_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT);
        FF_DEBUG("Types to add for adapter %u: 0x%X", (unsigned) ifIndex, typesToAdd);

        [[maybe_unused]] int ipv4Count = 0, ipv6Count = 0;

        for (ULONG j = i; j < table->NumEntries; ++j) {
            MIB_UNICASTIPADDRESS_ROW* ifa = &table->Table[j];
            if (ifa->InterfaceIndex != ifIndex) {
                continue;
            }

            FF_DEBUG("Processing unicast address: prefix origin=%d, suffix origin=%d, family=%d, DadState=%d",
                ifa->PrefixOrigin,
                ifa->SuffixOrigin,
                ifa->Address.si_family,
                ifa->DadState);

            if (!(options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT)) {
                if (ifa->DadState != IpDadStatePreferred) {
                    FF_DEBUG("Skipping address (not preferred)");
                    continue;
                }

                if (ifa->SuffixOrigin == IpSuffixOriginRandom) {
                    FF_DEBUG("Skipping temporary address (random suffix)");
                    continue;
                }

                // MIB_UNICASTIPADDRESS_ROW::SkipAsSource
            }

            if (ifa->Address.si_family == AF_INET) {
                if (!(typesToAdd & (FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT))) {
                    FF_DEBUG("Skipping IPv4 address (not requested in typesToAdd=0x%X)", typesToAdd);
                    continue;
                }

                bool isDefaultRoute = ((options->showType & FF_LOCALIP_TYPE_IPV4_BIT) && ffNetifGetDefaultRouteV4()->ifIndex == ifIndex);
                if ((options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT) && !isDefaultRoute) {
                    FF_DEBUG("Skipping IPv4 address (not on default route interface)");
                    continue;
                }

                SOCKADDR_IN* ipv4 = &ifa->Address.Ipv4;
                char addressBuffer[INET_ADDRSTRLEN + 10];
                char* end = RtlIpv4AddressToStringA(&ipv4->sin_addr, addressBuffer);

                if ((options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT) && ifa->OnLinkPrefixLength) {
                    end += snprintf(end, 10, "/%u", (unsigned) ifa->OnLinkPrefixLength);
                }

                FF_DEBUG("Adding IPv4 address: %s (isDefaultRoute=%s)", addressBuffer, isDefaultRoute ? "true" : "false");

                if (item->ipv4.length) {
                    ffStrbufAppendC(&item->ipv4, ',');
                }
                ffStrbufAppendNS(&item->ipv4, (uint32_t) (end - addressBuffer), addressBuffer);
                if (isDefaultRoute) {
                    item->defaultRoute |= FF_LOCALIP_TYPE_IPV4_BIT;
                }

                ipv4Count++;
                typesToAdd &= ~(unsigned) FF_LOCALIP_TYPE_IPV4_BIT;
                if (typesToAdd == 0) {
                    break;
                }
            } else if (ifa->Address.si_family == AF_INET6) {
                if (!(typesToAdd & (FF_LOCALIP_TYPE_IPV6_BIT | FF_LOCALIP_TYPE_ALL_IPS_BIT))) {
                    FF_DEBUG("Skipping IPv6 address (not requested in typesToAdd=0x%X)", typesToAdd);
                    continue;
                }

                SOCKADDR_IN6* ipv6 = &ifa->Address.Ipv6;

                FFLocalIpIpv6Type ipv6Type = FF_LOCALIP_IPV6_TYPE_NONE;
                if (IN6_IS_ADDR_GLOBAL(&ipv6->sin6_addr)) {
                    ipv6Type |= FF_LOCALIP_IPV6_TYPE_GUA_BIT;
                } else if (IN6_IS_ADDR_UNIQUE_LOCAL(&ipv6->sin6_addr)) {
                    ipv6Type |= FF_LOCALIP_IPV6_TYPE_ULA_BIT;
                } else if (IN6_IS_ADDR_LINKLOCAL(&ipv6->sin6_addr)) {
                    ipv6Type |= FF_LOCALIP_IPV6_TYPE_LLA_BIT;
                } else {
                    ipv6Type |= FF_LOCALIP_IPV6_TYPE_UNKNOWN_BIT;
                }

                if (!(options->ipv6Type & ipv6Type)) {
                    FF_DEBUG("Skipping IPv6 address (doesn't match requested type 0x%X)", options->ipv6Type);
                    continue;
                }

                bool isDefaultRoute = ((options->showType & FF_LOCALIP_TYPE_IPV6_BIT) && ffNetifGetDefaultRouteV6()->ifIndex == ifIndex);
                if ((options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT) && !isDefaultRoute) {
                    FF_DEBUG("Skipping IPv6 address (not on default route interface)");
                    continue;
                }

                char addressBuffer[INET6_ADDRSTRLEN + 10];
                char* end = RtlIpv6AddressToStringA(&ipv6->sin6_addr, addressBuffer);

                if ((options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT) && ifa->OnLinkPrefixLength) {
                    end += snprintf(end, 10, "/%u", (unsigned) ifa->OnLinkPrefixLength);
                }

                FF_DEBUG("Adding IPv6 address: %s (isDefaultRoute=%s)", addressBuffer, isDefaultRoute ? "true" : "false");

                if (item->ipv6.length) {
                    ffStrbufAppendC(&item->ipv6, ',');
                }
                ffStrbufAppendNS(&item->ipv6, (uint32_t) (end - addressBuffer), addressBuffer);
                if (isDefaultRoute) {
                    item->defaultRoute |= FF_LOCALIP_TYPE_IPV6_BIT;
                }

                ipv6Count++;
                typesToAdd &= ~(unsigned) FF_LOCALIP_TYPE_IPV6_BIT;
                if (typesToAdd == 0) {
                    break;
                }
            }
        }

        FF_DEBUG("Adapter %u: collected %d IPv4 and %d IPv6 addresses", (unsigned) ifIndex, ipv4Count, ipv6Count);

        if (options->showType & FF_LOCALIP_TYPE_SPEED_BIT) {
            item->speed = (int32_t) (ifRow.ReceiveLinkSpeed / 1000000);
            FF_DEBUG("Adapter %u speed: %d Mbps (raw: %llu)", (unsigned) ifIndex, item->speed, ifRow.ReceiveLinkSpeed);
        }
        // GetIfEntry2() reports 1500 for loopback, which has no real MTU
        if (options->showType & FF_LOCALIP_TYPE_MTU_BIT && !isLoop) {
            item->mtu = (int32_t) ifRow.Mtu;
            FF_DEBUG("Adapter %u MTU: %d", (unsigned) ifIndex, item->mtu);
        }
        if (options->showType & FF_LOCALIP_TYPE_FLAGS_BIT) {
            for (IP_ADAPTER_ADDRESSES* adapter = adapter_addresses; adapter; adapter = adapter->Next) {
                if (adapter->IfIndex == ifIndex) {
                    ffLocalIpFillNIFlags(&item->flags, adapter->Flags, niFlagOptions);
                    FF_DEBUG("Adapter %u flags: 0x%lX -> '%s'", (unsigned) ifIndex, adapter->Flags, item->flags.chars);
                    break;
                }
            }
        }
        if (options->showType & FF_LOCALIP_TYPE_MAC_BIT && ifRow.PhysicalAddressLength == 6) {
            uint8_t* ptr = ifRow.PhysicalAddress;
            ffStrbufSetF(&item->mac, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            FF_DEBUG("Adapter %u MAC: %s", (unsigned) ifIndex, item->mac.chars);
        }
    }

    FreeMibTable(table);

    FF_DEBUG("Local IP detection completed: scanned %d adapters, processed %d, results count: %u",
        adapterCount,
        processedCount,
        results->length);

    return nullptr;
}
