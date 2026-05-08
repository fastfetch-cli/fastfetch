#include "wifi.h"
#include "common/io.h"
#include "common/debug.h"

#include <sys/time.h>
#include <sys/socket.h>
#include <linux/genetlink.h>
#include <linux/nl80211.h>
#include <net/if.h>

// Silence warning of `NLA_HDRLEN` and `NLA_ALIGN`
#pragma GCC diagnostic ignored "-Wsign-conversion"

typedef struct FFWifiNlContext {
    int sockFd;
    uint16_t nl80211FamilyId;
    uint32_t portId;
    uint32_t seq;
} FFWifiNlContext;

typedef struct FFWifiSecurityFlags {
    bool privacy : 1;
    bool wep : 1;
    bool wpa : 1;
    bool wpa2 : 1;
    bool wpa3 : 1;
    bool owe : 1;
    bool eap : 1;
} FFWifiSecurityFlags;

static inline double rssiToSignalQuality(int rssi)
{
    return (double) (rssi >= -50 ? 100 : rssi <= -100 ? 0 : (rssi + 100) * 2);
}

static inline uint32_t ffWifiGetNetlinkPortId(int sockFd) {
    struct sockaddr_nl addr = {};
    socklen_t addrLen = sizeof(addr);
    if (getsockname(sockFd, (struct sockaddr*) &addr, &addrLen) < 0) {
        FF_DEBUG("Failed to query netlink socket address (use PID instead): %s", strerror(errno));
        return instance.state.platform.pid;
    }

    return addr.nl_pid;
}

static inline bool ffWifiNlAttrOk(const struct nlattr* attr, size_t remaining) {
    return remaining >= sizeof(*attr) &&
        attr->nla_len >= sizeof(*attr) &&
        attr->nla_len <= remaining;
}

static const struct nlattr* ffWifiNlAttrNext(const struct nlattr* attr, size_t* remaining) {
    size_t alignedLen = NLA_ALIGN(attr->nla_len);
    if (alignedLen > *remaining) {
        *remaining = 0;
        return NULL;
    }

    *remaining -= alignedLen;
    return (const struct nlattr*) ((const char*) attr + alignedLen);
}

static inline size_t ffWifiNlAttrPayload(const struct nlattr* attr) {
    return attr->nla_len > NLA_HDRLEN ? attr->nla_len - NLA_HDRLEN : 0;
}

static inline const void* ffWifiNlAttrData(const struct nlattr* attr) {
    return (const uint8_t*) attr + NLA_HDRLEN;
}

static bool ffWifiNlAppendAttr(struct nlmsghdr* nlh, size_t maxLen, uint16_t type, const void* data, uint16_t dataLen) {
    size_t offset = NLMSG_ALIGN(nlh->nlmsg_len);
    size_t attrLen = NLA_HDRLEN + dataLen;
    size_t alignedLen = NLA_ALIGN(attrLen);
    size_t newLen = offset + alignedLen;
    if (newLen > maxLen || attrLen > UINT16_MAX || newLen > UINT32_MAX) {
        return false;
    }

    struct nlattr* attr = (struct nlattr*) ((char*) nlh + offset);
    attr->nla_type = type;
    attr->nla_len = (uint16_t) attrLen;
    memcpy((char*) attr + NLA_HDRLEN, data, dataLen);
    memset((char*) attr + attrLen, 0, alignedLen - attrLen);
    nlh->nlmsg_len = (uint32_t) newLen;
    return true;
}

static bool ffWifiNlGetFamilyId(FFWifiNlContext* ctx) {
    struct {
        struct nlmsghdr nlh;
        struct genlmsghdr genl;
        char attrs[64];
    } req = {
        .nlh = {
            .nlmsg_len = NLMSG_LENGTH(sizeof(struct genlmsghdr)),
            .nlmsg_type = GENL_ID_CTRL,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
            .nlmsg_seq = ++ctx->seq,
            .nlmsg_pid = ctx->portId,
        },
        .genl = {
            .cmd = CTRL_CMD_GETFAMILY,
            .version = 1,
        },
    };

    if (!ffWifiNlAppendAttr(&req.nlh, sizeof(req), CTRL_ATTR_FAMILY_NAME, "nl80211", sizeof("nl80211"))) {
        FF_DEBUG("Failed to append CTRL_ATTR_FAMILY_NAME attribute");
        return false;
    }

    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
    };

    ssize_t sent = sendto(ctx->sockFd, &req, req.nlh.nlmsg_len, 0, (struct sockaddr*) &addr, sizeof(addr));
    if (sent != (ssize_t) req.nlh.nlmsg_len) {
        FF_DEBUG("Failed to send nl80211 family request: sent=%zd expected=%u", sent, req.nlh.nlmsg_len);
        return false;
    }

    uint8_t buffer[8192];
    while (true) {
        ssize_t received = recvfrom(ctx->sockFd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (received < 0) {
            FF_DEBUG("Failed to receive nl80211 family reply: %s", strerror(errno));
            return false;
        }

        for (const struct nlmsghdr* nlh = (const struct nlmsghdr*) buffer;
            NLMSG_OK(nlh, received);
            nlh = NLMSG_NEXT(nlh, received)) {
            if (nlh->nlmsg_seq != req.nlh.nlmsg_seq) {
                continue;
            }

            if (nlh->nlmsg_type == NLMSG_ERROR) {
                const struct nlmsgerr* err = (const struct nlmsgerr*) NLMSG_DATA(nlh);
                if (err->error != 0) {
                    FF_DEBUG("nl80211 family query failed: %s", strerror(-err->error));
                    return false;
                }
                continue;
            }

            if (nlh->nlmsg_type != GENL_ID_CTRL) {
                continue;
            }

            const struct genlmsghdr* genl = (const struct genlmsghdr*) NLMSG_DATA(nlh);
            if (genl->cmd != CTRL_CMD_NEWFAMILY) {
                continue;
            }

            size_t attrRemaining = nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN;
            for (const struct nlattr* attr = (const struct nlattr*) ((const char*) genl + GENL_HDRLEN);
                ffWifiNlAttrOk(attr, attrRemaining);
                attr = ffWifiNlAttrNext(attr, &attrRemaining)) {
                if ((attr->nla_type & NLA_TYPE_MASK) != CTRL_ATTR_FAMILY_ID || ffWifiNlAttrPayload(attr) < sizeof(uint16_t)) {
                    continue;
                }

                ctx->nl80211FamilyId = *(const uint16_t*) ffWifiNlAttrData(attr);
                return true;
            }
        }
    }
}

static bool ffWifiNlInit(FFWifiNlContext* ctx) {
    FF_AUTO_CLOSE_FD int _ = ctx->sockFd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_GENERIC);
    if (ctx->sockFd < 0) {
        FF_DEBUG("Failed to create generic netlink socket: %s", strerror(errno));
        return false;
    }

    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
    };

    if (bind(ctx->sockFd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        FF_DEBUG("Failed to bind generic netlink socket: %s", strerror(errno));
        return false;
    }

    if (setsockopt(
            ctx->sockFd,
            SOL_SOCKET,
            SO_RCVTIMEO,
            &(struct timeval) { .tv_sec = 0, .tv_usec = 250000 },
            sizeof(struct timeval)) < 0) {
        FF_DEBUG("Failed to set netlink receive timeout: %s", strerror(errno));
        return false;
    }

    ctx->portId = ffWifiGetNetlinkPortId(ctx->sockFd);
    if (!ffWifiNlGetFamilyId(ctx)) {
        return false;
    }

    _ = -1; // We are ok now
    return true;
}

static double ffWifiParseBitrateFromRateInfo(const struct nlattr* rateAttr, FFstrbuf* protocol) {
    double rate = -DBL_MAX;
    size_t remaining = ffWifiNlAttrPayload(rateAttr);

    for (const struct nlattr* info = (const struct nlattr*) ffWifiNlAttrData(rateAttr);
        ffWifiNlAttrOk(info, remaining);
        info = ffWifiNlAttrNext(info, &remaining)) {
        uint16_t type = (uint16_t) (info->nla_type & NLA_TYPE_MASK);
        size_t payload = ffWifiNlAttrPayload(info);

        switch (type) {
            case 30 /* NL80211_RATE_INFO_UHR_MCS*/:
                ffStrbufSetStatic(protocol, "802.11bn (Wi-Fi 8)");
                break;
            case 23 /*NL80211_RATE_INFO_S1G_MCS*/:
                ffStrbufSetStatic(protocol, "802.11ah (Wi-Fi HaLow)");
                break;
            case 19 /* NL80211_RATE_INFO_EHT_MCS */:
                ffStrbufSetStatic(protocol, "802.11be (Wi-Fi 7)");
                break;
            case 13 /* NL80211_RATE_INFO_HE_MCS */:
                ffStrbufSetStatic(protocol, "802.11ax (Wi-Fi 6)");
                break;
            case NL80211_RATE_INFO_VHT_MCS:
                ffStrbufSetStatic(protocol, "802.11ac (Wi-Fi 5)");
                break;
            case NL80211_RATE_INFO_MCS:
                ffStrbufSetStatic(protocol, "802.11n (Wi-Fi 4)");
                break;
            case NL80211_RATE_INFO_BITRATE32:
                if (payload >= sizeof(uint32_t)) {
                    rate = *(uint32_t*) ffWifiNlAttrData(info) / 10.0;
                }
                break;
            case NL80211_RATE_INFO_BITRATE:
                if (payload >= sizeof(uint16_t) && rate == -DBL_MAX) {
                    rate = *(uint16_t*) ffWifiNlAttrData(info) / 10.0;
                }
                break;
        }
    }

    return rate;
}

static void ffWifiApplySecurityFlags(FFWifiResult* item, const FFWifiSecurityFlags* sec) {
    ffStrbufClear(&item->conn.security);

    if (sec->wep) {
        ffStrbufAppendS(&item->conn.security, "WEP/");
    }
    if (sec->wpa) {
        ffStrbufAppendS(&item->conn.security, "WPA/");
    }
    if (sec->wpa2) {
        ffStrbufAppendS(&item->conn.security, "WPA2/");
    }
    if (sec->wpa3) {
        ffStrbufAppendS(&item->conn.security, "WPA3/");
    }
    if (sec->owe) {
        ffStrbufAppendS(&item->conn.security, "OWE/");
    }
    if (sec->eap) {
        ffStrbufAppendS(&item->conn.security, "802.1X/");
    }

    if (!item->conn.security.length) {
        if (sec->privacy) {
            ffStrbufSetStatic(&item->conn.security, "WEP");
        } else {
            ffStrbufSetStatic(&item->conn.security, "Insecure");
        }
    } else {
        ffStrbufTrimRight(&item->conn.security, '/');
    }
}

static void ffWifiParseRsnIe(const uint8_t* ie, size_t len, FFWifiSecurityFlags* sec) {
    if (len < 8) {
        return;
    }

    sec->wpa2 = true;
    size_t pos = 0;

    pos += 2;
    if (pos + 4 > len) {
        return;
    }
    pos += 4;

    if (pos + 2 > len) {
        return;
    }
    uint16_t pairwiseCount = *(uint16_t*) (ie + pos);
    pos += 2;

    size_t pairwiseLen = (size_t) pairwiseCount * 4;
    if (pos + pairwiseLen > len) {
        return;
    }
    pos += pairwiseLen;

    if (pos + 2 > len) {
        return;
    }
    uint16_t akmCount = *(uint16_t*) (ie + pos);
    pos += 2;

    for (uint16_t i = 0; i < akmCount && pos + 4 <= len; ++i, pos += 4) {
        const uint8_t* akm = ie + pos;
        if (akm[0] != 0x00 || akm[1] != 0x0f || akm[2] != 0xac) {
            continue;
        }

        switch (akm[3]) {
            case 1:
            case 5:
            case 11:
            case 12:
                sec->eap = true;
                break;
            case 8:
                sec->wpa3 = true;
                break;
            case 18:
                sec->owe = true;
                break;
            default:
                break;
        }
    }

    if (sec->owe) {
        sec->wpa2 = false;
    }
}

static void ffWifiParseWpaVendorIe(const uint8_t* ie, size_t len, FFWifiSecurityFlags* sec) {
    if (len < 8) {
        return;
    }

    if (!(ie[0] == 0x00 && ie[1] == 0x50 && ie[2] == 0xf2 && ie[3] == 0x01)) {
        return;
    }

    sec->wpa = true;

    size_t pos = 4;
    if (pos + 2 > len) {
        return;
    }
    pos += 2;

    if (pos + 4 > len) {
        return;
    }
    pos += 4;

    if (pos + 2 > len) {
        return;
    }
    uint16_t pairwiseCount = *(uint16_t*) (ie + pos);
    pos += 2 + (size_t) pairwiseCount * 4;

    if (pos + 2 > len) {
        return;
    }
    uint16_t akmCount = *(uint16_t*) (ie + pos);
    pos += 2;

    for (uint16_t i = 0; i < akmCount && pos + 4 <= len; ++i, pos += 4) {
        const uint8_t* akm = ie + pos;
        if (!(akm[0] == 0x00 && akm[1] == 0x50 && akm[2] == 0xf2)) {
            continue;
        }
        if (akm[3] == 1) {
            sec->eap = true;
        }
    }
}

static void ffWifiParseInformationElements(const uint8_t* ies, size_t length, FFWifiResult* item, FFWifiSecurityFlags* sec) {
    size_t pos = 0;
    while (pos + 2 <= length) {
        uint8_t id = ies[pos];
        uint8_t len = ies[pos + 1];
        pos += 2;
        if (pos + len > length) {
            break;
        }

        const uint8_t* ie = ies + pos;
        if (id == 0 && !item->conn.ssid.length) {
            ffStrbufSetNS(&item->conn.ssid, len, (const char*) ie);
        } else if (id == 48) {
            ffWifiParseRsnIe(ie, len, sec);
        } else if (id == 221) {
            ffWifiParseWpaVendorIe(ie, len, sec);
        }

        pos += len;
    }
}

static bool ffWifiParseBssAttr(const struct nlattr* bssAttr, FFWifiResult* item, bool* associated) {
    FFWifiSecurityFlags sec = {};
    size_t remaining = ffWifiNlAttrPayload(bssAttr);

    for (const struct nlattr* attr = (const struct nlattr*) ffWifiNlAttrData(bssAttr);
        ffWifiNlAttrOk(attr, remaining);
        attr = ffWifiNlAttrNext(attr, &remaining)) {
        uint16_t type = (uint16_t) (attr->nla_type & NLA_TYPE_MASK);
        size_t payload = ffWifiNlAttrPayload(attr);

        if (type == NL80211_BSS_STATUS && payload >= sizeof(uint32_t)) {
            if (*(uint32_t*) ffWifiNlAttrData(attr) == NL80211_BSS_STATUS_ASSOCIATED) {
                *associated = true;
            }
        } else if (type == NL80211_BSS_BSSID && payload >= 6) {
            const uint8_t* mac = (const uint8_t*) ffWifiNlAttrData(attr);
            ffStrbufSetF(&item->conn.bssid, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        } else if (type == NL80211_BSS_FREQUENCY && payload >= sizeof(uint32_t)) {
            item->conn.frequency = (uint16_t) *(uint32_t*) ffWifiNlAttrData(attr);
            item->conn.channel = ffWifiFreqToChannel(item->conn.frequency);
        } else if (type == NL80211_BSS_SIGNAL_MBM && payload >= sizeof(int32_t)) {
            int rssi = *(int32_t*) ffWifiNlAttrData(attr) / 100;
            item->conn.signalQuality = rssiToSignalQuality(rssi);
        } else if (type == NL80211_BSS_CAPABILITY && payload >= sizeof(uint16_t)) {
            uint16_t capability = *(uint16_t*) ffWifiNlAttrData(attr);
            sec.privacy = (capability & (1u << 4u)) != 0;
        } else if (type == NL80211_BSS_INFORMATION_ELEMENTS || type == NL80211_BSS_BEACON_IES) {
            ffWifiParseInformationElements((const uint8_t*) ffWifiNlAttrData(attr), payload, item, &sec);
        }
    }

    if (!*associated) {
        return false;
    }

    ffWifiApplySecurityFlags(item, &sec);
    return true;
}

static bool ffWifiFetchScanInfo(FFWifiNlContext* ctx, FFWifiResult* item, uint32_t ifIndex) {
    struct {
        struct nlmsghdr nlh;
        struct genlmsghdr genl;
        char attrs[32];
    } req = {
        .nlh = {
            .nlmsg_len = NLMSG_LENGTH(sizeof(struct genlmsghdr)),
            .nlmsg_type = ctx->nl80211FamilyId,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP | NLM_F_ACK,
            .nlmsg_seq = ++ctx->seq,
            .nlmsg_pid = ctx->portId,
        },
        .genl = {
            .cmd = NL80211_CMD_GET_SCAN,
            .version = 0,
        },
    };

    if (!ffWifiNlAppendAttr(&req.nlh, sizeof(req), NL80211_ATTR_IFINDEX, &ifIndex, sizeof(ifIndex))) {
        FF_DEBUG("Failed to build nl80211 scan request");
        return false;
    }

    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
    };

    ssize_t sent = sendto(ctx->sockFd, &req, req.nlh.nlmsg_len, 0, (struct sockaddr*) &addr, sizeof(addr));
    if (sent != (ssize_t) req.nlh.nlmsg_len) {
        FF_DEBUG("Failed to send nl80211 scan request");
        return false;
    }

    uint8_t buffer[1024 * 16];
    bool associated = false;
    while (true) {
        ssize_t received = recvfrom(ctx->sockFd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (received < 0) {
            FF_DEBUG("Failed to receive nl80211 scan reply: %s", strerror(errno));
            return false;
        }

        for (const struct nlmsghdr* nlh = (const struct nlmsghdr*) buffer;
            NLMSG_OK(nlh, received);
            nlh = NLMSG_NEXT(nlh, received)) {
            if (nlh->nlmsg_seq != req.nlh.nlmsg_seq) {
                continue;
            }

            if (nlh->nlmsg_type == NLMSG_DONE) {
                return associated;
            }

            if (nlh->nlmsg_type == NLMSG_ERROR) {
                const struct nlmsgerr* err = (const struct nlmsgerr*) NLMSG_DATA(nlh);
                if (err->error == 0) {
                    continue;
                }
                FF_DEBUG("nl80211 scan request failed: %s", strerror(-err->error));
                return associated;
            }

            if (nlh->nlmsg_type != ctx->nl80211FamilyId) {
                continue;
            }

            const struct genlmsghdr* genl = (const struct genlmsghdr*) NLMSG_DATA(nlh);
            size_t attrRemaining = nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN;
            for (const struct nlattr* attr = (const struct nlattr*) ((const char*) genl + GENL_HDRLEN);
                ffWifiNlAttrOk(attr, attrRemaining);
                attr = ffWifiNlAttrNext(attr, &attrRemaining)) {
                if ((attr->nla_type & NLA_TYPE_MASK) != NL80211_ATTR_BSS) {
                    continue;
                }

                bool thisAssociated = false;
                if (ffWifiParseBssAttr(attr, item, &thisAssociated) && thisAssociated) {
                    associated = true;
                }
            }
        }
    }
}

static void ffWifiParseStationInfo(const struct nlattr* staInfoAttr, FFWifiResult* item) {
    size_t remaining = ffWifiNlAttrPayload(staInfoAttr);
    for (const struct nlattr* attr = (const struct nlattr*) ffWifiNlAttrData(staInfoAttr);
        ffWifiNlAttrOk(attr, remaining);
        attr = ffWifiNlAttrNext(attr, &remaining)) {
        uint16_t type = (uint16_t) (attr->nla_type & NLA_TYPE_MASK);
        size_t payload = ffWifiNlAttrPayload(attr);

        if (type == NL80211_STA_INFO_SIGNAL && payload >= sizeof(uint8_t) && item->conn.signalQuality == -DBL_MAX) {
            int rssi = (int8_t) *(const uint8_t*) ffWifiNlAttrData(attr);
            item->conn.signalQuality = rssiToSignalQuality(rssi);
        } else if (type == NL80211_STA_INFO_TX_BITRATE) {
            double tx = ffWifiParseBitrateFromRateInfo(attr, &item->conn.protocol);
            if (tx != -DBL_MAX) {
                item->conn.txRate = tx;
            }
        } else if (type == NL80211_STA_INFO_RX_BITRATE) {
            double rx = ffWifiParseBitrateFromRateInfo(attr, &item->conn.protocol);
            if (rx != -DBL_MAX) {
                item->conn.rxRate = rx;
            }
        }
    }
}

static bool ffWifiFetchStationInfo(FFWifiNlContext* ctx, FFWifiResult* item, uint32_t ifIndex) {
    struct {
        struct nlmsghdr nlh;
        struct genlmsghdr genl;
        char attrs[32];
    } req = {
        .nlh = {
            .nlmsg_len = NLMSG_LENGTH(sizeof(struct genlmsghdr)),
            .nlmsg_type = ctx->nl80211FamilyId,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP | NLM_F_ACK,
            .nlmsg_seq = ++ctx->seq,
            .nlmsg_pid = ctx->portId,
        },
        .genl = {
            .cmd = NL80211_CMD_GET_STATION,
            .version = 0,
        },
    };

    if (!ffWifiNlAppendAttr(&req.nlh, sizeof(req), NL80211_ATTR_IFINDEX, &ifIndex, sizeof(ifIndex))) {
        FF_DEBUG("Failed to build nl80211 station request");
        return false;
    }

    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
    };

    ssize_t sent = sendto(ctx->sockFd, &req, req.nlh.nlmsg_len, 0, (struct sockaddr*) &addr, sizeof(addr));
    if (sent != (ssize_t) req.nlh.nlmsg_len) {
        FF_DEBUG("Failed to send nl80211 station request");
        return false;
    }

    uint8_t buffer[8192];
    bool gotStation = false;
    while (true) {
        ssize_t received = recvfrom(ctx->sockFd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (received < 0) {
            FF_DEBUG("Failed to receive nl80211 station reply: %s", strerror(errno));
            return gotStation;
        }

        for (const struct nlmsghdr* nlh = (const struct nlmsghdr*) buffer;
            NLMSG_OK(nlh, received);
            nlh = NLMSG_NEXT(nlh, received)) {
            if (nlh->nlmsg_seq != req.nlh.nlmsg_seq) {
                continue;
            }

            if (nlh->nlmsg_type == NLMSG_DONE) {
                return gotStation;
            }

            if (nlh->nlmsg_type == NLMSG_ERROR) {
                const struct nlmsgerr* err = (const struct nlmsgerr*) NLMSG_DATA(nlh);
                if (err->error != 0) {
                    FF_DEBUG("nl80211 station request failed: %s", strerror(-err->error));
                }
                return gotStation;
            }

            if (nlh->nlmsg_type != ctx->nl80211FamilyId) {
                continue;
            }

            const struct genlmsghdr* genl = (const struct genlmsghdr*) NLMSG_DATA(nlh);
            size_t attrRemaining = nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN;
            for (const struct nlattr* attr = (const struct nlattr*) ((const char*) genl + GENL_HDRLEN);
                ffWifiNlAttrOk(attr, attrRemaining);
                attr = ffWifiNlAttrNext(attr, &attrRemaining)) {
                if ((attr->nla_type & NLA_TYPE_MASK) != NL80211_ATTR_STA_INFO) {
                    continue;
                }

                ffWifiParseStationInfo(attr, item);
                gotStation = true;
            }
        }
    }
}

static void ffWifiDetectWithNetlink(FFWifiNlContext* ctx, FFWifiResult* item, uint32_t ifIndex) {
    bool associated = ffWifiFetchScanInfo(ctx, item, ifIndex);
    if (associated) {
        ffStrbufSetStatic(&item->conn.status, "connected");
    } else if (!item->conn.status.length) {
        ffStrbufSetStatic(&item->conn.status, "disconnected");
    }

    if (associated) {
        ffWifiFetchStationInfo(ctx, item, ifIndex);
    }

    if (!item->conn.protocol.length && item->conn.txRate != -DBL_MAX) {
        FF_DEBUG("nl80211 station info did not include MCS family fields");
    }
}

const char* ffDetectWifi(FFlist* result) {
    FF_DEBUG("Starting wifi detection");

    FFWifiNlContext nl = { .sockFd = -1 };

    struct if_nameindex* infs = if_nameindex();

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    for (struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i) {
        FF_DEBUG("Checking interface: %s (index: %u)", i->if_name, i->if_index);
        ffStrbufSetF(&buffer, "/sys/class/net/%s/phy80211/", i->if_name);
        if (!ffPathExists(buffer.chars, FF_PATHTYPE_DIRECTORY)) {
            FF_DEBUG("Not a wifi interface (no phy80211 directory)");
            continue;
        }

        if (nl.sockFd < 0) {
            if (!ffWifiNlInit(&nl)) {
                FF_DEBUG("Failed to initialize netlink context, skipping wifi detection");
                break;
            }
        }

        FFWifiResult* item = FF_LIST_ADD(FFWifiResult, *result);
        ffStrbufInitS(&item->inf.description, i->if_name);
        ffStrbufInit(&item->inf.status);
        ffStrbufInit(&item->conn.status);
        ffStrbufInit(&item->conn.ssid);
        ffStrbufInit(&item->conn.bssid);
        ffStrbufInit(&item->conn.protocol);
        ffStrbufInit(&item->conn.security);
        item->conn.signalQuality = -DBL_MAX;
        item->conn.rxRate = -DBL_MAX;
        item->conn.txRate = -DBL_MAX;
        item->conn.channel = 0;
        item->conn.frequency = 0;

        char operstate;
        ffStrbufSetF(&buffer, "/sys/class/net/%s/operstate", i->if_name);
        if (!ffReadFileData(buffer.chars, 1, &operstate)) {
            ffStrbufSetStatic(&item->inf.status, "unknown");
            ffStrbufSetStatic(&item->conn.status, "disconnected");
            continue;
        }

        if (operstate == 'u') {
            ffStrbufSetStatic(&item->inf.status, "up");
            ffWifiDetectWithNetlink(&nl, item, i->if_index);
            if (!item->conn.status.length) {
                ffStrbufSetStatic(&item->conn.status, "disconnected");
            }
        } else {
            ffStrbufSetStatic(&item->conn.status, "disconnected");

            ffStrbufSetF(&buffer, "/sys/class/net/%s/flags", i->if_name);
            char flags[16];
            ssize_t len = ffReadFileData(buffer.chars, sizeof(flags) - 1, flags);
            if (len <= 0) {
                ffStrbufSetStatic(&item->inf.status, "unknown");
                continue;
            }
            flags[len] = '\0';

            unsigned flagsVal = (unsigned) strtoul(flags, NULL, 16);
            if (flagsVal & IFF_UP) {
                ffStrbufSetStatic(&item->inf.status, "up");
            } else {
                ffStrbufSetStatic(&item->inf.status, "down");
            }
        }
    }

    if_freenameindex(infs);
    if (nl.sockFd >= 0) {
        close(nl.sockFd);
    }

    FF_DEBUG("Wifi detection completed, found %u wifi interfaces", result->length);
    return NULL;
}
