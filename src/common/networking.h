#pragma once

#include "common/thread.h"
#include "common/FFstrbuf.h"

#ifdef _WIN32
    #include <minwindef.h>
#endif

struct addrinfo;

typedef struct FFNetworkingState {
#ifdef _WIN32
    uintptr_t sockfd;
    OVERLAPPED overlapped;
#else
    int sockfd;
    struct addrinfo* addr;

    #ifdef FF_HAVE_THREADS
    FFThreadType thread;
    #endif
#endif

    FFstrbuf command;
    uint32_t timeout;
    bool ipv6;
    bool compression; // if true, HTTP content compression will be enabled if supported
    bool tfo;         // if true, TCP Fast Open will be attempted first, and fallback to traditional connection if it fails
} FFNetworkingState;

// `headers` is optional and may be null; the other pointer arguments are dereferenced unconditionally.
// Both functions report failure through their return value, which must be checked.
[[gnu::nonnull(1, 2, 4), nodiscard]] const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, uint16_t port, const char* path, const char* headers);
[[gnu::nonnull(1, 2), nodiscard]] const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer);

// Case-insensitive header lookup restricted to the header block [0, headerEnd).
// Restricting the range matters because the body may already share the same buffer.
// Returns a pointer to the first character of the value; `valueLen` receives its
// length excluding the terminating CRLF. Returns nullptr when the header is absent.
[[gnu::nonnull(1, 3, 4), gnu::pure, nodiscard]] const char* ffNetworkingFindHeader(const char* headers, uint32_t headerEnd, const char* name, uint32_t* valueLen);

// Checks whether a `Transfer-Encoding: chunked` body has been received in full, so that
// framing does not have to rely on the server closing the connection.
// Returns 1 when complete (`consumed` receives the body length including trailers),
// 0 when more data is needed, and -1 when the body is malformed.
// The caller must not wait for a fixed amount of data (e.g. `MSG_WAITALL`) while the
// response length is still unknown, otherwise this check never gets to run.
// Not `pure`: the `strtoul` it calls writes `errno`.
[[gnu::nonnull(1, 3), nodiscard]] int ffNetworkingChunkedComplete(const char* body, uint32_t bodyLen, uint32_t* consumed);

// Decodes a `Transfer-Encoding: chunked` body in place and rewrites the response with a
// `Content-Length` header in place of `Transfer-Encoding`.
[[gnu::nonnull(1), nodiscard]] bool ffNetworkingDecodeChunked(FFstrbuf* buffer, uint32_t headerEnd);

// Result of parsing a `Transfer-Encoding` header value
typedef enum FFNetworkingTransferEncoding {
    FF_NETWORKING_TE_NONE,        // the value holds no coding at all
    FF_NETWORKING_TE_CHUNKED,     // exactly `chunked`, the only framing this client decodes
    FF_NETWORKING_TE_UNSUPPORTED, // another coding or a chain of them, e.g. `gzip, chunked`
} FFNetworkingTransferEncoding;

// Parses a `Transfer-Encoding` header value. Codings are applied in the order they are
// listed, so `chunked` has to be the last one for the framing to be readable at all --
// and any other coding (e.g. `gzip, chunked`) leaves the payload encoded, which this
// client cannot decode. Only a lone `chunked` is accepted; everything else is reported
// as unsupported so that the caller fails the response instead of returning garbage.
[[gnu::nonnull(1), gnu::pure, nodiscard]] FFNetworkingTransferEncoding ffNetworkingParseTransferEncoding(const char* value, uint32_t valueLen);

#ifdef FF_HAVE_ZLIB
const char* ffNetworkingLoadZlibLibrary(void);
[[gnu::nonnull(1, 2), nodiscard]] bool ffNetworkingDecompressGzip(FFstrbuf* buffer, char* headerEnd);
#endif
