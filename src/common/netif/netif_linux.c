#include "netif.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/debug.h"

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

bool ffNetifGetDefaultRouteImplV4(FFNetifDefaultRouteResult* result)
{
    FF_DEBUG("Starting IPv4 default route detection");

    FF_AUTO_CLOSE_FD int sock_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (sock_fd < 0)
    {
        FF_DEBUG("Failed to create netlink socket: %s", strerror(errno));
        return false;
    }
    FF_DEBUG("Created netlink socket: fd=%d", sock_fd);

    unsigned pid = (unsigned) getpid();
    FF_DEBUG("Process PID: %u", pid);

    // Bind socket
    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
        .nl_pid = 0,         // Let kernel choose PID
        .nl_groups = 0,      // No multicast groups
    };

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        FF_DEBUG("Failed to bind socket: %s", strerror(errno));
        return false;
    }
    FF_DEBUG("Successfully bound socket");

    struct __attribute__((__packed__)) {
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

    uint8_t buffer[1024 * 16]; // 16 KB buffer should be sufficient
    uint32_t minMetric = UINT32_MAX;
    int routeCount = 0;

    while (true)
    {
        ssize_t received = recvfrom(sock_fd, buffer, sizeof(buffer), 0,
            (struct sockaddr*)&src_addr, &src_addr_len);

        if (received < 0) {
            FF_DEBUG("Failed to receive netlink response: %s", strerror(errno));
            return false;
        }

        if (received >= (ssize_t)sizeof(buffer)) {
            FF_DEBUG("Received truncated message: received %zd, bufsize %zu", received, sizeof(buffer));
            return false;
        }
        FF_DEBUG("Received netlink response: %zd bytes", received);
        if (received == 0) {
            FF_DEBUG("Received zero-length netlink response, ending processing");
            break;
        }

        struct {
            uint32_t metric;
            uint32_t ifindex;
            uint32_t prefsrc;
        } entry;

        for (const struct nlmsghdr* nlh = (struct nlmsghdr*)buffer;
            NLMSG_OK(nlh, received);
            nlh = NLMSG_NEXT(nlh, received))
        {
            if (nlh->nlmsg_seq != 1 || nlh->nlmsg_pid != pid)
                continue;
            if (nlh->nlmsg_type == NLMSG_DONE)
            {
                FF_DEBUG("Received NLMSG_DONE, processed %d routes", routeCount);
                goto exit;
            }

            if (nlh->nlmsg_type == NLMSG_ERROR) {
                FF_DEBUG("Netlink reports error: %s", strerror(-((struct nlmsgerr*)NLMSG_DATA(nlh))->error));
                continue;
            }

            if (nlh->nlmsg_type != RTM_NEWROUTE) {
                FF_DEBUG("Skipping non-route message: type=%d", nlh->nlmsg_type);
                continue;
            }

            routeCount++;
            struct rtmsg* rtm = (struct rtmsg*)NLMSG_DATA(nlh);
            if (rtm->rtm_family != AF_INET) {
                FF_DEBUG("Skipping non-IPv4 route #%d (family=%d)", routeCount, rtm->rtm_family);
                continue;
            }

            if (rtm->rtm_dst_len != 0) {
                FF_DEBUG("Skipping non-default route #%d (dst_len=%d)", routeCount, rtm->rtm_dst_len);
                continue;
            }

            FF_DEBUG("Processing IPv4 default route candidate #%d", routeCount);
            entry = (__typeof__(entry)) { }; // Default to zero metric (no RTA_PRIORITY found)

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
                        FF_DEBUG("Unexpected RTA_DST: %s (len=%u)", inet_ntoa((struct in_addr) { .s_addr = rta_data }), rtm->rtm_dst_len);
                        goto next;
                    case RTA_OIF:
                        entry.ifindex = rta_data;
                        FF_DEBUG("Found interface index: %u", entry.ifindex);
                        break;
                    case RTA_GATEWAY:
                        FF_DEBUG("Found gateway: %s", inet_ntoa(*(struct in_addr*)&rta_data));
                        if (rta_data == 0) goto next;
                        break;
                    case RTA_PRIORITY:
                        FF_DEBUG("Found metric: %u", rta_data);
                        if (rta_data >= minMetric) goto next;
                        entry.metric = rta_data;
                        break;
                    case RTA_PREFSRC:
                        entry.prefsrc = rta_data;
                        FF_DEBUG("Found preferred source: %s", inet_ntoa(*(struct in_addr*)&rta_data));
                        break;
                }
            }

            if (entry.ifindex == 0 || entry.metric >= minMetric)
            {
            next:
                FF_DEBUG("Skipping route: ifindex=%u, metric=%u", entry.ifindex, entry.metric);
                continue;
            }
            minMetric = entry.metric;
            result->ifIndex = entry.ifindex;
            FF_DEBUG("Updated best route: ifindex=%u, metric=%u, prefsrc=%x", entry.ifindex, entry.metric, entry.prefsrc);
            result->preferredSourceAddrV4 = entry.prefsrc;
            if (minMetric == 0)
            {
                FF_DEBUG("Found zero metric route, stopping further processing");
                break; // Stop processing if we found a zero metric route
            }
        }
    }

exit:
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
        FF_DEBUG("Failed to create netlink socket: %s", strerror(errno));
        return false;
    }
    FF_DEBUG("Created netlink socket: fd=%d", sock_fd);

    unsigned pid = (unsigned) getpid();
    FF_DEBUG("Process PID: %u", pid);

    // Bind socket
    struct sockaddr_nl addr = {
        .nl_family = AF_NETLINK,
        .nl_pid = 0,         // Let kernel choose PID
        .nl_groups = 0,      // No multicast groups
    };

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        FF_DEBUG("Failed to bind socket: %s", strerror(errno));
        return false;
    }
    FF_DEBUG("Successfully bound socket");

    struct __attribute__((__packed__)) {
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

    uint8_t buffer[1024 * 16]; // 16 KB buffer should be sufficient
    uint32_t minMetric = UINT32_MAX;
    int routeCount = 0;

    while (true)
    {
        ssize_t received = recvfrom(sock_fd, buffer, sizeof(buffer), 0,
            (struct sockaddr*)&src_addr, &src_addr_len);

        if (received < 0) {
            FF_DEBUG("Failed to receive netlink response: %s", strerror(errno));
            return false;
        }

        if (received >= (ssize_t)sizeof(buffer)) {
            FF_DEBUG("Received truncated message: received %zd, bufsize %zu", received, sizeof(buffer));
            return false;
        }
        FF_DEBUG("Received netlink response: %zd bytes", received);
        if (received == 0) {
            FF_DEBUG("Received zero-length netlink response, ending processing");
            break;
        }

        struct {
            uint32_t metric;
            uint32_t ifindex;
        } entry;

        for (const struct nlmsghdr* nlh = (struct nlmsghdr*)buffer;
            NLMSG_OK(nlh, received);
            nlh = NLMSG_NEXT(nlh, received))
        {
            if (nlh->nlmsg_seq != 1 || nlh->nlmsg_pid != pid)
                continue;
            if (nlh->nlmsg_type == NLMSG_DONE)
            {
                FF_DEBUG("Received NLMSG_DONE, processed %d routes", routeCount);
                goto exit;
            }

            if (nlh->nlmsg_type == NLMSG_ERROR) {
                FF_DEBUG("Netlink reports error: %s", strerror(-((struct nlmsgerr*)NLMSG_DATA(nlh))->error));
                continue;
            }

            if (nlh->nlmsg_type != RTM_NEWROUTE) {
                FF_DEBUG("Skipping non-route message: type=%d", nlh->nlmsg_type);
                continue;
            }

            routeCount++;
            struct rtmsg* rtm = (struct rtmsg*)NLMSG_DATA(nlh);
            if (rtm->rtm_family != AF_INET6) {
                FF_DEBUG("Skipping non-IPv6 route #%d (family=%d)", routeCount, rtm->rtm_family);
                continue;
            }

            if (rtm->rtm_dst_len != 0) {
                FF_DEBUG("Skipping non-default route #%d (dst_len=%d)", routeCount, rtm->rtm_dst_len);
                continue;
            }

            FF_DEBUG("Processing IPv6 default route candidate #%d", routeCount);
            entry = (__typeof__(entry)) { }; // Default to zero metric (no RTA_PRIORITY found)

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
                            FF_DEBUG("Unexpected RTA_DST: %s", inet_ntop(AF_INET6, RTA_DATA(rta), str, sizeof(str)));
                            goto next;
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
                            FF_DEBUG("Found metric: %u", metric);
                            if (metric >= minMetric) goto next;
                            entry.metric = metric;
                        }
                        break;
                }
            }

            if (entry.ifindex == 0 || entry.metric >= minMetric)
            {
            next:
                FF_DEBUG("Skipping route: ifindex=%u, metric=%u", entry.ifindex, entry.metric);
                continue;
            }
            minMetric = entry.metric;
            result->ifIndex = entry.ifindex;
            FF_DEBUG("Updated best route: ifindex=%u, metric=%u", entry.ifindex, entry.metric);

            if (minMetric == 0)
            {
                FF_DEBUG("Found zero metric route, stopping further processing");
                break; // Stop processing if we found a zero metric route
            }
        }
    }

exit:
    if (minMetric < UINT32_MAX)
    {
        if_indextoname(result->ifIndex, result->ifName);
        FF_DEBUG("Found default IPv6 route: interface=%s, index=%u, metric=%u", result->ifName, result->ifIndex, minMetric);
        return true;
    }
    FF_DEBUG("No IPv6 default route found");
    return false;
}
