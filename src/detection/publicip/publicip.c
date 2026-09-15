#include "publicip.h"
#include "common/networking.h"

#define FF_UNINITIALIZED ((const char*) (uintptr_t) -1)
static FFNetworkingState states[2];
static const char* statuses[2] = { FF_UNINITIALIZED, FF_UNINITIALIZED };

// Reads the port that follows the colon at `colonIndex`. The port has to be the last thing in
// `host`, so this must run before any part of the host is trimmed off.
static uint16_t ffPublicIpParseUrlPort(const FFstrbuf* host, uint32_t colonIndex) {
    const char* portStr = host->chars + colonIndex + 1;
    char* portEnd = nullptr;
    unsigned long portValue = strtoul(portStr, &portEnd, 10);
    if (portEnd == portStr || *portEnd != '\0' || portValue == 0 || portValue > 65535) {
        fputs("Error: invalid port in the PublicIp module URL\n", stderr);
        exit(1);
    }

    return (uint16_t) portValue;
}

void ffPreparePublicIp(FFPublicIPOptions* options) {
    FFNetworkingState* state = &states[options->ipv6];
    const char** status = &statuses[options->ipv6];
    if (*status != FF_UNINITIALIZED) {
        fputs("Error: PublicIp module can only be used once due to internal limitations\n", stderr);
        exit(1);
    }

    state->timeout = options->timeout;
    state->ipv6 = options->ipv6;

    if (options->url.length == 0) {
        state->compression = true;
        state->tfo = true;
        *status = ffNetworkingSendHttpRequest(state, options->ipv6 ? "v6.ipinfo.io" : "ipinfo.io", 80, "/json", nullptr);
    } else {
        FF_STRBUF_AUTO_DESTROY host = ffStrbufCreateCopy(&options->url);
        uint32_t hostStartIndex = ffStrbufFirstIndexS(&host, "://");
        if (hostStartIndex < host.length) {
            if (hostStartIndex != 4 || !ffStrbufStartsWithIgnCaseS(&host, "http")) {
                fputs("Error: only http: protocol is supported. Use `Command` module with `curl` if needed\n", stderr);
                exit(1);
            }
            ffStrbufSubstrAfter(&host, hostStartIndex + (uint32_t) (strlen("://") - 1));
        }
        uint32_t pathStartIndex = ffStrbufFirstIndexC(&host, '/');

        FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
        if (pathStartIndex != host.length) {
            ffStrbufAppendNS(&path, host.length - pathStartIndex, host.chars + pathStartIndex);
            ffStrbufSubstrBefore(&host, pathStartIndex);
        }

        // An optional `:port` must be split off the host, otherwise getaddrinfo() is asked to
        // resolve a host name that still carries the port. Only a single colon can separate a
        // port: a bare IPv6 literal (`::1`) holds several of them and has no port at all,
        // which is why a bracketed literal (`[::1]:8080`) is the only unambiguous spelling.
        uint16_t port = 0;
        if (ffStrbufStartsWithC(&host, '[')) {
            uint32_t bracketEnd = ffStrbufFirstIndexC(&host, ']');
            if (bracketEnd == host.length) {
                fputs("Error: unmatched '[' in the PublicIp module URL\n", stderr);
                exit(1);
            }

            if (bracketEnd + 1 < host.length) {
                if (host.chars[bracketEnd + 1] != ':') {
                    fputs("Error: unexpected characters after the IPv6 literal in the PublicIp module URL\n", stderr);
                    exit(1);
                }
                // Read the port while the string is still intact: trimming the brackets first
                // would invalidate the index it was found at.
                port = ffPublicIpParseUrlPort(&host, bracketEnd + 1);
            }

            ffStrbufSubstrBefore(&host, bracketEnd);
            ffStrbufSubstrAfter(&host, 0); // drop the leading '['
        } else {
            uint32_t firstColon = ffStrbufFirstIndexC(&host, ':');
            if (firstColon < host.length && firstColon == ffStrbufLastIndexC(&host, ':')) {
                port = ffPublicIpParseUrlPort(&host, firstColon);
                ffStrbufSubstrBefore(&host, firstColon);
            }
        }

        *status = ffNetworkingSendHttpRequest(state, host.chars, port ?: 80, path.length == 0 ? "/" : path.chars, nullptr);
    }
}

static inline void wrapYyjsonFree(yyjson_doc** doc) {
    assert(doc);
    if (*doc) {
        yyjson_doc_free(*doc);
    }
}

const char* ffDetectPublicIp(FFPublicIPOptions* options, FFPublicIpResult* result) {
    FFNetworkingState* state = &states[options->ipv6];
    const char** status = &statuses[options->ipv6];
    if (*status == FF_UNINITIALIZED) {
        ffPreparePublicIp(options);
    }

    if (*status != nullptr) {
        return *status;
    }

    FF_STRBUF_AUTO_DESTROY response = ffStrbufCreateA(4096);
    const char* error = ffNetworkingRecvHttpResponse(state, &response);

    *state = (FFNetworkingState) {};
    *status = FF_UNINITIALIZED;

    if (error == nullptr) {
        ffStrbufSubstrAfterFirstS(&response, "\r\n\r\n");
    } else {
        return error;
    }

    if (response.length == 0) {
        return "Empty server response received";
    }

    if (options->url.length == 0) {
        [[gnu::cleanup(wrapYyjsonFree)]] yyjson_doc* doc = yyjson_read_opts(response.chars, response.length, 0, nullptr, nullptr);
        if (doc) {
            yyjson_val* root = yyjson_doc_get_root(doc);
            ffStrbufAppendJsonVal(&result->ip, yyjson_obj_get(root, "ip"));
            ffStrbufDestroy(&result->location);
            ffStrbufInitF(&result->location, "%s, %s", yyjson_get_str(yyjson_obj_get(root, "city")), yyjson_get_str(yyjson_obj_get(root, "country")));
            return nullptr;
        }
    }

    ffStrbufDestroy(&result->ip);
    ffStrbufInitMove(&result->ip, &response);
    ffStrbufTrimRightSpace(&result->ip);
    return nullptr;
}
