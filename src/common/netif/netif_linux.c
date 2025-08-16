#include "netif.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/debug.h"

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

bool ffNetifGetDefaultRouteImplV4(FFNetifDefaultRouteResult* result)
{
    FF_DEBUG("Starting IPv4 default route detection");

    FF_AUTO_CLOSE_FD int sock_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (sock_fd < 0)
    {
        FF_DEBUG("Failed to create netlink socket: errno=%d", errno);
        return false;
    }
    FF_DEBUG("Created netlink socket: fd=%d", sock_fd);

    unsigned pid = (unsigned) getpid();
    FF_DEBUG("Process PID: %u", pid);

    // Bind socket
    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
        .nl_pid = pid,
        .nl_groups = 0,      // No multicast groups
    };

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        FF_DEBUG("Failed to bind socket: errno=%d", errno);
        return false;
    }
    FF_DEBUG("Successfully bound socket");

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        struct rtattr rta;
        uint32_t table;
    } req = {
        // Netlink message header
        .nlh = {
            .nlmsg_len = sizeof(req),
            .nlmsg_type = RTM_GETROUTE,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
            .nlmsg_seq = 1,
            .nlmsg_pid = pid,
        },
        // Route message
        .rtm = {
            .rtm_family = AF_INET,
            .rtm_dst_len = 0,  // Match all destinations
            .rtm_src_len = 0,  // Match all sources
            .rtm_tos = 0,
            .rtm_table = RT_TABLE_UNSPEC,
            .rtm_protocol = RTPROT_UNSPEC,
            .rtm_scope = RT_SCOPE_UNIVERSE,
            .rtm_type = RTN_UNSPEC,
            .rtm_flags = 0,
        },
        // Route attribute for main table
        .rta = {
            .rta_len = RTA_LENGTH(sizeof(uint32_t)),
            .rta_type = RTA_TABLE,
        },
        .table = RT_TABLE_MAIN,
    };

    struct sockaddr_nl dest_addr = {
        .nl_family = AF_NETLINK,
        .nl_pid = 0,         // Kernel
        .nl_groups = 0,      // No multicast groups
    };

    ssize_t sent = sendto(sock_fd, &req, sizeof(req), 0,
        (struct sockaddr*)&dest_addr, sizeof(dest_addr));

    if (sent != sizeof(req)) {
        FF_DEBUG("Failed to send netlink request: sent=%zd, expected=%zu", sent, sizeof(req));
        return false;
    }
    FF_DEBUG("Sent netlink request: %zd bytes", sent);

    struct sockaddr_nl src_addr = {};
    socklen_t src_addr_len = sizeof(src_addr);

    struct iovec iov = {};
    struct msghdr msg = {
        .msg_name = &src_addr,
        .msg_namelen = sizeof(src_addr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };

    ssize_t peek_size = recvmsg(sock_fd, &msg, MSG_PEEK | MSG_TRUNC);
    if (peek_size < 0) {
        FF_DEBUG("Failed to peek message size: errno=%d", errno);
        return false;
    }
    FF_DEBUG("Message size: %zd bytes", peek_size);

    FF_AUTO_FREE uint8_t* buffer = malloc((size_t)peek_size);

    ssize_t received = recvfrom(sock_fd, buffer, (size_t)peek_size, 0,
        (struct sockaddr*)&src_addr, &src_addr_len);
    if (received != peek_size) {
        FF_DEBUG("Failed to receive complete message: received=%zd, expected=%zd", received, peek_size);
        return false;
    }
    FF_DEBUG("Received netlink response: %zd bytes", received);

    struct {
        uint32_t metric;
        uint32_t ifindex;
        uint32_t prefsrc;
    } entry;
    uint32_t minMetric = UINT32_MAX;
    int routeCount = 0;

    for (const struct nlmsghdr* nlh = (struct nlmsghdr*)buffer;
        NLMSG_OK(nlh, received);
        nlh = NLMSG_NEXT(nlh, received)) {
        if (nlh->nlmsg_seq != 1 || nlh->nlmsg_pid != pid)
            continue;
        if (nlh->nlmsg_type == NLMSG_DONE)
        {
            FF_DEBUG("Received NLMSG_DONE, processed %d routes", routeCount);
            break;
        }

        if (nlh->nlmsg_type != RTM_NEWROUTE)
            continue;

        routeCount++;
        struct rtmsg* rtm = (struct rtmsg*)NLMSG_DATA(nlh);
        if (rtm->rtm_family != AF_INET)
            continue;

        if (rtm->rtm_dst_len != 0)
            continue;

        FF_DEBUG("Processing IPv4 default route candidate #%d", routeCount);
        entry = (__typeof__(entry)) { .metric = UINT32_MAX };

        // Parse route attributes
        size_t rtm_len = RTM_PAYLOAD(nlh);
        for (struct rtattr* rta = RTM_RTA(rtm);
            RTA_OK(rta, rtm_len);
            rta = RTA_NEXT(rta, rtm_len))
        {
            if (RTA_PAYLOAD(rta) < sizeof(uint32_t))
                continue; // Skip invalid attributes

            uint32_t rta_data = *(uint32_t*) RTA_DATA(rta);
            switch (rta->rta_type) {
                case RTA_DST:
                    FF_DEBUG("Found destination: %s", inet_ntoa(*(struct in_addr*)&rta_data));
                    if (rta_data != 0) goto next;
                    break;
                case RTA_OIF:
                    entry.ifindex = rta_data;
                    FF_DEBUG("Found interface index: %u", entry.ifindex);
                    break;
                case RTA_GATEWAY:
                    if (rta_data == 0) goto next;
                    FF_DEBUG("Found gateway: %s", inet_ntoa(*(struct in_addr*)&rta_data));
                    break;
                case RTA_PRIORITY:
                    if (rta_data >= minMetric) goto next;
                    entry.metric = rta_data;
                    FF_DEBUG("Found metric: %u", entry.metric);
                    break;
                case RTA_PREFSRC:
                    entry.prefsrc = rta_data;
                    FF_DEBUG("Found preferred source: %s", inet_ntoa(*(struct in_addr*)&rta_data));
                    break;
            }
        }

        if (entry.metric >= minMetric)
        {
        next:
            continue;
        }
        minMetric = entry.metric;
        result->ifIndex = entry.ifindex;
        FF_DEBUG("Updated best route: ifindex=%u, metric=%u", entry.ifindex, entry.metric);
        // result->preferredSourceAddrV4 = entry.prefsrc;
    }

    if (minMetric < UINT32_MAX)
    {
        if_indextoname(result->ifIndex, result->ifName);
        FF_DEBUG("Found default IPv4 route: interface=%s, index=%u, metric=%u", result->ifName, result->ifIndex, minMetric);
        return true;
    }
    FF_DEBUG("No IPv4 default route found");
    return false;
}

bool ffNetifGetDefaultRouteImplV6(FFNetifDefaultRouteResult* result)
{
    FF_DEBUG("Starting IPv6 default route detection");

    FF_AUTO_CLOSE_FD int sock_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (sock_fd < 0)
    {
        FF_DEBUG("Failed to create netlink socket: errno=%d", errno);
        return false;
    }
    FF_DEBUG("Created netlink socket: fd=%d", sock_fd);

    unsigned pid = (unsigned) getpid();
    FF_DEBUG("Process PID: %u", pid);

    // Bind socket
    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
        .nl_pid = pid,
        .nl_groups = 0,      // No multicast groups
    };

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        FF_DEBUG("Failed to bind socket: errno=%d", errno);
        return false;
    }
    FF_DEBUG("Successfully bound socket");

    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
        struct rtattr rta;
        uint32_t table;
    } req = {
        // Netlink message header
        .nlh = {
            .nlmsg_len = sizeof(req),
            .nlmsg_type = RTM_GETROUTE,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
            .nlmsg_seq = 1,
            .nlmsg_pid = pid,
        },
        // Route message
        .rtm = {
            .rtm_family = AF_INET6,    // IPv6 instead of IPv4
            .rtm_dst_len = 0,  // Match all destinations
            .rtm_src_len = 0,  // Match all sources
            .rtm_tos = 0,
            .rtm_table = RT_TABLE_UNSPEC,
            .rtm_protocol = RTPROT_UNSPEC,
            .rtm_scope = RT_SCOPE_UNIVERSE,
            .rtm_type = RTN_UNSPEC,
            .rtm_flags = 0,
        },
        // Route attribute for main table
        .rta = {
            .rta_len = RTA_LENGTH(sizeof(uint32_t)),
            .rta_type = RTA_TABLE,
        },
        .table = RT_TABLE_MAIN,
    };

    struct sockaddr_nl dest_addr = {
        .nl_family = AF_NETLINK,
        .nl_pid = 0,         // Kernel
        .nl_groups = 0,      // No multicast groups
    };

    ssize_t sent = sendto(sock_fd, &req, sizeof(req), 0,
        (struct sockaddr*)&dest_addr, sizeof(dest_addr));

    if (sent != sizeof(req)) {
        FF_DEBUG("Failed to send netlink request: sent=%zd, expected=%zu", sent, sizeof(req));
        return false;
    }
    FF_DEBUG("Sent netlink request: %zd bytes", sent);

    struct sockaddr_nl src_addr = {};
    socklen_t src_addr_len = sizeof(src_addr);

    struct iovec iov = {};
    struct msghdr msg = {
        .msg_name = &src_addr,
        .msg_namelen = sizeof(src_addr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };

    ssize_t peek_size = recvmsg(sock_fd, &msg, MSG_PEEK | MSG_TRUNC);
    if (peek_size < 0) {
        FF_DEBUG("Failed to peek message size: errno=%d", errno);
        return false;
    }
    FF_DEBUG("Message size: %zd bytes", peek_size);

    FF_AUTO_FREE uint8_t* buffer = malloc((size_t)peek_size);

    ssize_t received = recvfrom(sock_fd, buffer, (size_t)peek_size, 0,
        (struct sockaddr*)&src_addr, &src_addr_len);
    if (received != peek_size) {
        FF_DEBUG("Failed to receive complete message: received=%zd, expected=%zd", received, peek_size);
        return false;
    }
    FF_DEBUG("Received netlink response: %zd bytes", received);

    struct {
        uint32_t metric;
        uint32_t ifindex;
    } entry;
    uint32_t minMetric = UINT32_MAX;
    int routeCount = 0;

    for (const struct nlmsghdr* nlh = (struct nlmsghdr*)buffer;
        NLMSG_OK(nlh, received);
        nlh = NLMSG_NEXT(nlh, received)) {
        if (nlh->nlmsg_seq != 1 || nlh->nlmsg_pid != pid)
            continue;
        if (nlh->nlmsg_type == NLMSG_DONE)
        {
            FF_DEBUG("Received NLMSG_DONE, processed %d routes", routeCount);
            break;
        }

        if (nlh->nlmsg_type != RTM_NEWROUTE)
            continue;

        routeCount++;
        struct rtmsg* rtm = (struct rtmsg*)NLMSG_DATA(nlh);
        if (rtm->rtm_family != AF_INET6)
            continue;

        if (rtm->rtm_dst_len != 0)
            continue;

        FF_DEBUG("Processing IPv6 default route candidate #%d", routeCount);
        entry = (__typeof__(entry)) { .metric = UINT32_MAX };

        // Parse route attributes
        size_t rtm_len = RTM_PAYLOAD(nlh);
        for (struct rtattr* rta = RTM_RTA(rtm);
            RTA_OK(rta, rtm_len);
            rta = RTA_NEXT(rta, rtm_len))
        {
            switch (rta->rta_type) {
                case RTA_DST:
                    if (RTA_PAYLOAD(rta) >= sizeof(struct in6_addr)) {
                        FF_MAYBE_UNUSED char str[INET6_ADDRSTRLEN];
                        FF_DEBUG("Found destination: %s", inet_ntop(AF_INET6, RTA_DATA(rta), str, sizeof(str)));
                        struct in6_addr* dst = (struct in6_addr*) RTA_DATA(rta);
                        if (!IN6_IS_ADDR_UNSPECIFIED(dst)) goto next;
                    }
                    break;
                case RTA_OIF:
                    if (RTA_PAYLOAD(rta) >= sizeof(uint32_t)) {
                        entry.ifindex = *(uint32_t*) RTA_DATA(rta);
                        FF_DEBUG("Found interface index: %u", entry.ifindex);
                    }
                    break;
                case RTA_GATEWAY:
                    if (RTA_PAYLOAD(rta) >= sizeof(struct in6_addr)) {
                        struct in6_addr* gw = (struct in6_addr*) RTA_DATA(rta);
                        if (IN6_IS_ADDR_UNSPECIFIED(gw)) goto next;
                        FF_MAYBE_UNUSED char str[INET6_ADDRSTRLEN];
                        FF_DEBUG("Found gateway: %s", inet_ntop(AF_INET6, gw, str, sizeof(str)));
                    }
                    break;
                case RTA_PRIORITY:
                    if (RTA_PAYLOAD(rta) >= sizeof(uint32_t)) {
                        uint32_t metric = *(uint32_t*) RTA_DATA(rta);
                        if (metric >= minMetric) goto next;
                        entry.metric = metric;
                        FF_DEBUG("Found metric: %u", entry.metric);
                    }
                    break;
            }
        }

        if (entry.metric >= minMetric)
        {
        next:
            continue;
        }
        minMetric = entry.metric;
        result->ifIndex = entry.ifindex;
        FF_DEBUG("Updated best route: ifindex=%u, metric=%u", entry.ifindex, entry.metric);
    }

    if (minMetric < UINT32_MAX)
    {
        if_indextoname(result->ifIndex, result->ifName);
        FF_DEBUG("Found default IPv6 route: interface=%s, index=%u, metric=%u", result->ifName, result->ifIndex, minMetric);
        return true;
    }
    FF_DEBUG("No IPv6 default route found");
    return false;
}
