#include "netif.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

struct req_t {
    struct nlmsghdr nlh;
    struct rtmsg rtm;
    struct rtattr rta;
    uint32_t table;
};

struct Route4Entry {
    uint32_t dest;
    uint32_t gateway;
    uint32_t src;
    uint8_t prefix_length;
    uint32_t metric;
    uint32_t ifindex;
};

static bool getDefaultRouteIPv4(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex, uint32_t* preferredSourceAddr)
{
    FF_AUTO_CLOSE_FD int sock_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (sock_fd < 0)
        return false;

    // Bind socket
    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = 0;  // Let kernel assign PID
    addr.nl_groups = 0;

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }

    struct req_t req;
    memset(&req, 0, sizeof(req));

    // Netlink message header
    req.nlh.nlmsg_len = sizeof(req);
    req.nlh.nlmsg_type = RTM_GETROUTE;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.nlh.nlmsg_seq = 0;
    req.nlh.nlmsg_pid = 0;

    // Route message
    req.rtm.rtm_family = AF_INET;
    req.rtm.rtm_dst_len = 0;
    req.rtm.rtm_src_len = 0;
    req.rtm.rtm_tos = 0;
    req.rtm.rtm_table = RT_TABLE_UNSPEC;
    req.rtm.rtm_protocol = RTPROT_UNSPEC;
    req.rtm.rtm_scope = RT_SCOPE_UNIVERSE;
    req.rtm.rtm_type = RTN_UNSPEC;
    req.rtm.rtm_flags = 0;

    // Route attribute for main table
    req.rta.rta_len = RTA_LENGTH(sizeof(uint32_t));
    req.rta.rta_type = RTA_TABLE;
    req.table = RT_TABLE_MAIN;

    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    ssize_t sent = sendto(sock_fd, &req, sizeof(req), 0,
                          (struct sockaddr*)&dest_addr, sizeof(dest_addr));

    if (sent != sizeof(req)) {
        return false;
    }

    struct sockaddr_nl src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    struct iovec iov = {NULL, 0};
    struct msghdr msg;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &src_addr;
    msg.msg_namelen = sizeof(src_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    ssize_t peek_size = recvmsg(sock_fd, &msg, MSG_PEEK | MSG_TRUNC);
    if (peek_size < 0) {
        return false;
    }

    FF_AUTO_FREE uint8_t* buffer = malloc((size_t)peek_size);

    ssize_t received = recvfrom(sock_fd, buffer, (size_t)peek_size, 0,
                                (struct sockaddr*)&src_addr, &src_addr_len);

    struct Route4Entry best_gw;
    memset(&best_gw, 0, sizeof(best_gw));

    for (const struct nlmsghdr* nlh = (struct nlmsghdr*)buffer;
        NLMSG_OK(nlh, received);
        nlh = NLMSG_NEXT(nlh, received)) {

        if (nlh->nlmsg_type == NLMSG_DONE)
            break;

        if (nlh->nlmsg_type != RTM_NEWROUTE)
            continue;

        struct rtmsg* rtm = (struct rtmsg*)NLMSG_DATA(nlh);
        if (rtm->rtm_family != AF_INET)
            continue;

        struct Route4Entry entry;
        memset(&entry, 0, sizeof(struct Route4Entry));
        entry.prefix_length = rtm->rtm_dst_len;

        // Parse route attributes
        uint64_t rtm_len = RTM_PAYLOAD(nlh);
        for (struct rtattr* rta = RTM_RTA(rtm);
            RTA_OK(rta, rtm_len);
            rta = RTA_NEXT(rta, rtm_len)) {

            switch (rta->rta_type) {
                case RTA_DST:
                    entry.dest = *(uint32_t*)RTA_DATA(rta);
                    break;
                case RTA_GATEWAY:
                    entry.gateway = *(uint32_t*)RTA_DATA(rta);
                    break;
                case RTA_PREFSRC:
                    entry.src = *(uint32_t*)RTA_DATA(rta);
                    break;
                case RTA_PRIORITY:
                    entry.metric = *(uint32_t*)RTA_DATA(rta);
                    break;
                case RTA_OIF:
                    entry.ifindex = *(uint32_t*)RTA_DATA(rta);
                    break;
            }
        }

        if (entry.gateway == 0 || entry.dest != 0 || entry.prefix_length != 0)
            continue;

        if (best_gw.gateway == 0 || entry.metric < best_gw.metric) {
            memcpy(&best_gw, &entry, sizeof(struct Route4Entry));
        }
    }

    if (best_gw.gateway != 0) {
        if (ifIndex) {
            *ifIndex = best_gw.ifindex;
        }
        if (iface) {
            if (if_indextoname(best_gw.ifindex, iface) == NULL) {
                iface[0] = '\0';
            }
        }
        if (preferredSourceAddr) {
            *preferredSourceAddr = best_gw.src;
        }
        return true;
    }
    
    iface[0] = '\0';
    return false;
}

static bool getDefaultRouteIPv6(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex)
{
    FILE* FF_AUTO_CLOSE_FILE netRoute = fopen("/proc/net/ipv6_route", "r");
    if (!netRoute) return false;

    uint32_t prefixLen;
    //destination, dest_prefix_len, source, src_prefix_len, next hop, metric, ref counter, use counter, flags, iface
    while (fscanf(netRoute, "%*s %x %*s %*s %*s %*s %*s %*s %*s %" FF_STR(IF_NAMESIZE) "s", &prefixLen, iface) == 2)
    {
        if (prefixLen != 0) continue;
        *ifIndex = if_nametoindex(iface);
        return true;
    }
    iface[0] = '\0';
    return false;
}

bool ffNetifGetDefaultRouteImpl(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex, uint32_t* preferredSourceAddr)
{
    if (getDefaultRouteIPv4(iface, ifIndex, preferredSourceAddr))
        return true;

    return getDefaultRouteIPv6(iface, ifIndex);
}
