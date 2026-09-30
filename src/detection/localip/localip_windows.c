#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "common/netif.h"
#include "common/windows/unicode.h"
#include "common/debug.h"
#include "localip.h"

static inline void appendFlag(FFstrbuf* flags, bool enabled, const char* name) {
    if (!enabled) {
        return;
    }

    if (flags->length) {
        ffStrbufAppendC(flags, ',');
    }
    ffStrbufAppendS(flags, name);
}

static void fillInterfaceFlags(FFstrbuf* flags, const MIB_IF_ROW2* row) {
    appendFlag(flags, row->InterfaceAndOperStatusFlags.HardwareInterface, "HARDWARE");
    appendFlag(flags, row->InterfaceAndOperStatusFlags.FilterInterface, "FILTER");
    appendFlag(flags, row->InterfaceAndOperStatusFlags.ConnectorPresent, "CONNECTOR_PRESENT");
    appendFlag(flags, row->InterfaceAndOperStatusFlags.NotAuthenticated, "NOT_AUTHENTICATED");
    appendFlag(flags, row->InterfaceAndOperStatusFlags.NotMediaConnected, "NO_MEDIA");
    appendFlag(flags, row->InterfaceAndOperStatusFlags.Paused, "PAUSED");
    appendFlag(flags, row->InterfaceAndOperStatusFlags.LowPower, "LOW_POWER");
    appendFlag(flags, row->InterfaceAndOperStatusFlags.EndPointInterface, "ENDPOINT");

    switch (row->DirectionType) {
    case NET_IF_DIRECTION_SENDONLY:
        appendFlag(flags, true, "SEND_ONLY");
        break;
    case NET_IF_DIRECTION_RECEIVEONLY:
        appendFlag(flags, true, "RECEIVE_ONLY");
        break;
    case NET_IF_DIRECTION_SENDRECEIVE:
        appendFlag(flags, true, "SEND_RECEIVE");
        break;
    default:
        break;
    }

    // Exactly one value of each of these enums is reported per interface, so every value gets
    // a flag of its own
#define FF_LOCALIP_FLAG_CASE(value, name) \
    case value: appendFlag(flags, true, name); break;

    switch (row->MediaConnectState) {
        FF_LOCALIP_FLAG_CASE(MediaConnectStateUnknown, "MEDIA_UNKNOWN")
        FF_LOCALIP_FLAG_CASE(MediaConnectStateConnected, "MEDIA_CONNECTED")
        FF_LOCALIP_FLAG_CASE(MediaConnectStateDisconnected, "MEDIA_DISCONNECTED")
    }

    // The interface type, as registered with IANA and listed in ipifcons.h
    switch (row->Type) {
        FF_LOCALIP_FLAG_CASE(IF_TYPE_OTHER, "OTHER")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_REGULAR_1822, "REGULAR_1822")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HDH_1822, "HDH_1822")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DDN_X25, "DDN_X25")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_RFC877_X25, "RFC877_X25")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ETHERNET_CSMACD, "ETHERNET_CSMACD")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IS088023_CSMACD, "IS088023_CSMACD")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISO88024_TOKENBUS, "ISO88024_TOKENBUS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISO88025_TOKENRING, "ISO88025_TOKENRING")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISO88026_MAN, "ISO88026_MAN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_STARLAN, "STARLAN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROTEON_10MBIT, "PROTEON_10MBIT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROTEON_80MBIT, "PROTEON_80MBIT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HYPERCHANNEL, "HYPERCHANNEL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FDDI, "FDDI")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_LAP_B, "LAP_B")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SDLC, "SDLC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DS1, "DS1")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_E1, "E1")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_BASIC_ISDN, "BASIC_ISDN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PRIMARY_ISDN, "PRIMARY_ISDN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_POINT2POINT_SERIAL, "PROP_POINT2POINT_SERIAL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PPP, "PPP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SOFTWARE_LOOPBACK, "SOFTWARE_LOOPBACK")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_EON, "EON")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ETHERNET_3MBIT, "ETHERNET_3MBIT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_NSIP, "NSIP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SLIP, "SLIP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ULTRA, "ULTRA")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DS3, "DS3")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SIP, "SIP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FRAMERELAY, "FRAMERELAY")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_RS232, "RS232")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PARA, "PARA")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ARCNET, "ARCNET")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ARCNET_PLUS, "ARCNET_PLUS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM, "ATM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MIO_X25, "MIO_X25")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SONET, "SONET")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_X25_PLE, "X25_PLE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISO88022_LLC, "ISO88022_LLC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_LOCALTALK, "LOCALTALK")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SMDS_DXI, "SMDS_DXI")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FRAMERELAY_SERVICE, "FRAMERELAY_SERVICE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_V35, "V35")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HSSI, "HSSI")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HIPPI, "HIPPI")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MODEM, "MODEM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_AAL5, "AAL5")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SONET_PATH, "SONET_PATH")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SONET_VT, "SONET_VT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SMDS_ICIP, "SMDS_ICIP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_VIRTUAL, "PROP_VIRTUAL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_MULTIPLEXOR, "PROP_MULTIPLEXOR")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IEEE80212, "IEEE80212")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FIBRECHANNEL, "FIBRECHANNEL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HIPPIINTERFACE, "HIPPIINTERFACE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FRAMERELAY_INTERCONNECT, "FRAMERELAY_INTERCONNECT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_AFLANE_8023, "AFLANE_8023")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_AFLANE_8025, "AFLANE_8025")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_CCTEMUL, "CCTEMUL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FASTETHER, "FASTETHER")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISDN, "ISDN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_V11, "V11")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_V36, "V36")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_G703_64K, "G703_64K")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_G703_2MB, "G703_2MB")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_QLLC, "QLLC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FASTETHER_FX, "FASTETHER_FX")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_CHANNEL, "CHANNEL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IEEE80211, "IEEE80211")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IBM370PARCHAN, "IBM370PARCHAN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ESCON, "ESCON")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DLSW, "DLSW")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISDN_S, "ISDN_S")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISDN_U, "ISDN_U")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_LAP_D, "LAP_D")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IPSWITCH, "IPSWITCH")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_RSRB, "RSRB")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM_LOGICAL, "ATM_LOGICAL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DS0, "DS0")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DS0_BUNDLE, "DS0_BUNDLE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_BSC, "BSC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ASYNC, "ASYNC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_CNR, "CNR")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISO88025R_DTR, "ISO88025R_DTR")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_EPLRS, "EPLRS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ARAP, "ARAP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_CNLS, "PROP_CNLS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HOSTPAD, "HOSTPAD")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_TERMPAD, "TERMPAD")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FRAMERELAY_MPI, "FRAMERELAY_MPI")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_X213, "X213")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ADSL, "ADSL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_RADSL, "RADSL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SDSL, "SDSL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VDSL, "VDSL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISO88025_CRFPRINT, "ISO88025_CRFPRINT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MYRINET, "MYRINET")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VOICE_EM, "VOICE_EM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VOICE_FXO, "VOICE_FXO")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VOICE_FXS, "VOICE_FXS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VOICE_ENCAP, "VOICE_ENCAP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VOICE_OVERIP, "VOICE_OVERIP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM_DXI, "ATM_DXI")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM_FUNI, "ATM_FUNI")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM_IMA, "ATM_IMA")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PPPMULTILINKBUNDLE, "PPPMULTILINKBUNDLE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IPOVER_CDLC, "IPOVER_CDLC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IPOVER_CLAW, "IPOVER_CLAW")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_STACKTOSTACK, "STACKTOSTACK")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VIRTUALIPADDRESS, "VIRTUALIPADDRESS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MPC, "MPC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IPOVER_ATM, "IPOVER_ATM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISO88025_FIBER, "ISO88025_FIBER")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_TDLC, "TDLC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_GIGABITETHERNET, "GIGABITETHERNET")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HDLC, "HDLC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_LAP_F, "LAP_F")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_V37, "V37")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_X25_MLP, "X25_MLP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_X25_HUNTGROUP, "X25_HUNTGROUP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_TRANSPHDLC, "TRANSPHDLC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_INTERLEAVE, "INTERLEAVE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FAST, "FAST")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IP, "IP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DOCSCABLE_MACLAYER, "DOCSCABLE_MACLAYER")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DOCSCABLE_DOWNSTREAM, "DOCSCABLE_DOWNSTREAM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DOCSCABLE_UPSTREAM, "DOCSCABLE_UPSTREAM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_A12MPPSWITCH, "A12MPPSWITCH")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_TUNNEL, "TUNNEL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_COFFEE, "COFFEE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_CES, "CES")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM_SUBINTERFACE, "ATM_SUBINTERFACE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_L2_VLAN, "L2_VLAN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_L3_IPVLAN, "L3_IPVLAN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_L3_IPXVLAN, "L3_IPXVLAN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DIGITALPOWERLINE, "DIGITALPOWERLINE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MEDIAMAILOVERIP, "MEDIAMAILOVERIP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DTM, "DTM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DCN, "DCN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IPFORWARD, "IPFORWARD")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MSDSL, "MSDSL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IEEE1394, "IEEE1394")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IF_GSN, "IF_GSN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DVBRCC_MACLAYER, "DVBRCC_MACLAYER")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DVBRCC_DOWNSTREAM, "DVBRCC_DOWNSTREAM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DVBRCC_UPSTREAM, "DVBRCC_UPSTREAM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM_VIRTUAL, "ATM_VIRTUAL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MPLS_TUNNEL, "MPLS_TUNNEL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SRP, "SRP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VOICEOVERATM, "VOICEOVERATM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_VOICEOVERFRAMERELAY, "VOICEOVERFRAMERELAY")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IDSL, "IDSL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_COMPOSITELINK, "COMPOSITELINK")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SS7_SIGLINK, "SS7_SIGLINK")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_WIRELESS_P2P, "PROP_WIRELESS_P2P")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FR_FORWARD, "FR_FORWARD")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_RFC1483, "RFC1483")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_USB, "USB")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IEEE8023AD_LAG, "IEEE8023AD_LAG")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_BGP_POLICY_ACCOUNTING, "BGP_POLICY_ACCOUNTING")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FRF16_MFR_BUNDLE, "FRF16_MFR_BUNDLE")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_H323_GATEKEEPER, "H323_GATEKEEPER")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_H323_PROXY, "H323_PROXY")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MPLS, "MPLS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MF_SIGLINK, "MF_SIGLINK")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HDSL2, "HDSL2")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SHDSL, "SHDSL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DS1_FDL, "DS1_FDL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_POS, "POS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DVB_ASI_IN, "DVB_ASI_IN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DVB_ASI_OUT, "DVB_ASI_OUT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PLC, "PLC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_NFAS, "NFAS")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_TR008, "TR008")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_GR303_RDT, "GR303_RDT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_GR303_IDT, "GR303_IDT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ISUP, "ISUP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_DOCS_WIRELESS_MACLAYER, "PROP_DOCS_WIRELESS_MACLAYER")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_DOCS_WIRELESS_DOWNSTREAM, "PROP_DOCS_WIRELESS_DOWNSTREAM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_DOCS_WIRELESS_UPSTREAM, "PROP_DOCS_WIRELESS_UPSTREAM")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_HIPERLAN2, "HIPERLAN2")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_PROP_BWA_P2MP, "PROP_BWA_P2MP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_SONET_OVERHEAD_CHANNEL, "SONET_OVERHEAD_CHANNEL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_DIGITAL_WRAPPER_OVERHEAD_CHANNEL, "DIGITAL_WRAPPER_OVERHEAD_CHANNEL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_AAL2, "AAL2")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_RADIO_MAC, "RADIO_MAC")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM_RADIO, "ATM_RADIO")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IMT, "IMT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_MVL, "MVL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_REACH_DSL, "REACH_DSL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_FR_DLCI_ENDPT, "FR_DLCI_ENDPT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_ATM_VCI_ENDPT, "ATM_VCI_ENDPT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_OPTICAL_CHANNEL, "OPTICAL_CHANNEL")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_OPTICAL_TRANSPORT, "OPTICAL_TRANSPORT")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IEEE80216_WMAN, "IEEE80216_WMAN")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_WWANPP, "WWANPP")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_WWANPP2, "WWANPP2")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_IEEE802154, "IEEE802154")
        FF_LOCALIP_FLAG_CASE(IF_TYPE_XBOX_WIRELESS, "XBOX_WIRELESS")
    }

#undef FF_LOCALIP_FLAG_CASE
}

static bool isDefaultRouteInterface(const FFLocalIpOptions* options, NET_IFINDEX ifIndex) {
    // Both route lookups are cached, so this is cheap even inside the per interface loop
    return ((options->showType & FF_LOCALIP_TYPE_IPV4_BIT) && ffNetifGetDefaultRouteV4()->ifIndex == ifIndex) ||
        ((options->showType & FF_LOCALIP_TYPE_IPV6_BIT) && ffNetifGetDefaultRouteV6()->ifIndex == ifIndex);
}

const char* ffDetectLocalIps(const FFLocalIpOptions* options, FFlist* results) {
    FF_DEBUG("Starting local IP detection with showType=0x%X, namePrefix='%.*s'",
        options->showType,
        (int) options->namePrefix.length,
        options->namePrefix.chars);

    ADDRESS_FAMILY family = options->showType & FF_LOCALIP_TYPE_IPV4_BIT
        ? options->showType & FF_LOCALIP_TYPE_IPV6_BIT ? AF_UNSPEC : AF_INET
        : AF_INET6;
    bool queryV4 = (options->showType & FF_LOCALIP_TYPE_IPV4_BIT) != 0;
    bool queryV6 = (options->showType & FF_LOCALIP_TYPE_IPV6_BIT) != 0;

    // Every interface is queried with GetIfEntry2() rather than by walking a table of all of them:
    // GetIfTable2() costs about 0.95 ms on a machine with a few dozen adapters however few of them
    // are reported, because it marshals every registered one, while GetIfEntry2() costs about 24 us
    // per interface. With 52 adapters registered and 8 interfaces carrying an address the per
    // interface API is 3.6 times faster overall; the table only wins past roughly 40 interfaces with
    // an address, and with FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT (the default) at most one interface
    // can pass the filter below anyway.
    bool defaultRouteOnly = (options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT) != 0;
    if (defaultRouteOnly) {
        if (queryV4) {
            FF_DEBUG("Querying the IPv4 default route interface only: ifIndex=%u", (unsigned) ffNetifGetDefaultRouteV4()->ifIndex);
        }
        if (queryV6) {
            FF_DEBUG("Querying the IPv6 default route interface only: ifIndex=%u", (unsigned) ffNetifGetDefaultRouteV6()->ifIndex);
        }
    }

    PMIB_UNICASTIPADDRESS_TABLE table = nullptr;
    const char* errorMessage = nullptr;

    // The unicast address table is the only API that reports per interface addresses and it cannot
    // be filtered by interface, so it is queried once and only the rows of the interfaces which pass
    // the filter below are read
    if (!NETIO_SUCCESS(GetUnicastIpAddressTable(family, &table))) {
        FF_DEBUG("GetUnicastIpAddressTable failed");
        errorMessage = "GetUnicastIpAddressTable() failed";
        goto cleanup;
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
        if (defaultRouteOnly && !isDefaultRouteInterface(options, ifIndex)) {
            FF_DEBUG("Skipping interface %u (not default route interface)", (unsigned) ifIndex);
            continue;
        }

        MIB_IF_ROW2 ifRow = { .InterfaceIndex = ifIndex };
        if (!NETIO_SUCCESS(GetIfEntry2(&ifRow))) {
            FF_DEBUG("GetIfEntry2(%u) failed", (unsigned) ifIndex);
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

        if (options->showType & FF_LOCALIP_TYPE_FLAGS_BIT) {
            fillInterfaceFlags(&item->flags, &ifRow);
            FF_DEBUG("Adapter %u flags: '%s'", (unsigned) ifIndex, item->flags.chars);
        }

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
                if (defaultRouteOnly && !isDefaultRoute) {
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
                if (defaultRouteOnly && !isDefaultRoute) {
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
        if (options->showType & FF_LOCALIP_TYPE_MTU_BIT) {
            item->mtu = (int32_t) ifRow.Mtu;
            FF_DEBUG("Adapter %u MTU: %d", (unsigned) ifIndex, item->mtu);
        }
        if (options->showType & FF_LOCALIP_TYPE_MAC_BIT && ifRow.PhysicalAddressLength == 6) {
            const uint8_t* ptr = ifRow.PhysicalAddress;
            ffStrbufSetF(&item->mac, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            FF_DEBUG("Adapter %u MAC: %s", (unsigned) ifIndex, item->mac.chars);
        }
    }

    FF_DEBUG("Local IP detection completed: scanned %d adapters, processed %d, results count: %u",
        adapterCount,
        processedCount,
        results->length);

cleanup:
    if (table) {
        FreeMibTable(table);
    }
    return errorMessage;
}
