#include "common/networking.h"
#include "common/strutil.h"
#include "common/debug.h"

const char* ffNetworkingFindHeader(const char* headers, uint32_t headerEnd, const char* name, uint32_t* valueLen) {
    uint32_t nameLen = (uint32_t) strlen(name);
    uint32_t pos = 0;

    while (pos < headerEnd) {
        uint32_t eol = pos;
        while (eol < headerEnd && headers[eol] != '\n') {
            ++eol;
        }
        uint32_t lineEnd = (eol > pos && headers[eol - 1] == '\r') ? (eol - 1) : eol;

        // Only match at the beginning of a line, so that a value can never be mistaken
        // for a field name (obs-fold continuation lines included)
        if (lineEnd - pos >= nameLen && strncasecmp(headers + pos, name, nameLen) == 0) {
            uint32_t valueStart = pos + nameLen;
            while (valueStart < lineEnd && (headers[valueStart] == ' ' || headers[valueStart] == '\t')) {
                ++valueStart;
            }
            *valueLen = lineEnd - valueStart;
            return headers + valueStart;
        }

        if (eol >= headerEnd) {
            break;
        }
        pos = eol + 1;
    }

    return nullptr;
}

FFNetworkingTransferEncoding ffNetworkingParseTransferEncoding(const char* value, uint32_t valueLen) {
    // The value is a comma-separated list of transfer codings (RFC 9112 6.1)
    uint32_t codingCount = 0;
    const char* coding = nullptr;
    uint32_t codingLen = 0;

    for (uint32_t i = 0, start = 0; i <= valueLen; ++i) {
        if (i < valueLen && value[i] != ',') {
            continue;
        }

        // trim the optional whitespace around the coding
        uint32_t from = start;
        uint32_t to = i;
        while (from < to && (value[from] == ' ' || value[from] == '\t')) {
            ++from;
        }
        while (to > from && (value[to - 1] == ' ' || value[to - 1] == '\t')) {
            --to;
        }

        if (to > from) {
            ++codingCount;
            coding = value + from;
            codingLen = to - from;
        }
        start = i + 1;
    }

    if (codingCount == 0) {
        return FF_NETWORKING_TE_NONE;
    }

    if (codingCount == 1 && codingLen == 7 && strncasecmp(coding, "chunked", 7) == 0) {
        return FF_NETWORKING_TE_CHUNKED;
    }

    return FF_NETWORKING_TE_UNSUPPORTED;
}

int ffNetworkingChunkedComplete(const char* body, uint32_t bodyLen, uint32_t* consumed) {
    uint32_t pos = 0;

    for (;;) {
        // chunk-size [ chunk-ext ] CRLF
        uint32_t eol = pos;
        while (eol < bodyLen && body[eol] != '\n') {
            ++eol;
        }
        if (eol >= bodyLen) {
            return 0; // the chunk-size line is not complete yet
        }

        if (eol == pos || !isxdigit((unsigned char) body[pos])) {
            return -1;
        }

        char* stop = nullptr;
        unsigned long size = strtoul(body + pos, &stop, 16);
        if (stop == body + pos) {
            return -1;
        }
        pos = eol + 1; // skips the chunk extension, which ends at the CRLF

        if (size == 0) {
            // last-chunk, followed by an optional trailer section and an empty line
            uint32_t p = pos;
            while (p < bodyLen) {
                uint32_t e = p;
                while (e < bodyLen && body[e] != '\n') {
                    ++e;
                }
                if (e >= bodyLen) {
                    return 0;
                }
                if (e == p || (e == p + 1 && body[p] == '\r')) {
                    *consumed = e + 1;
                    return 1;
                }
                p = e + 1;
            }
            return 0;
        }

        if (size > (unsigned long) (bodyLen - pos)) {
            return 0; // the chunk data is not complete yet
        }
        pos += (uint32_t) size;

        if (bodyLen - pos < 2) {
            return 0;
        }
        if (body[pos] != '\r' || body[pos + 1] != '\n') {
            return -1;
        }
        pos += 2;
    }
}

// Keeps the status line and every header except `dropHeader` and `Content-Length`,
// then emits a `Content-Length` matching the (already decoded) body.
// `body` may point into `buffer->chars`; it is copied before `buffer` is released.
//
// Returns the offset of the `\r` that opens the terminating CRLF CRLF of the new response. The
// rebuild drops a header line and rewrites another, so the header it produces is shorter than the
// one that went in: the offset the caller held before is no longer the one that describes this
// buffer, and handing it on would point into the middle of the body.
static uint32_t rebuildResponse(FFstrbuf* buffer, uint32_t headerEnd, const char* body, uint32_t bodyLen, const char* dropHeader) {
    FF_STRBUF_AUTO_DESTROY newBuffer = ffStrbufCreateA(headerEnd + bodyLen + 64);

    uint32_t pos = 0;
    while (pos < headerEnd) {
        uint32_t eol = pos;
        while (eol < headerEnd && buffer->chars[eol] != '\n') {
            ++eol;
        }
        uint32_t lineLen = (eol < headerEnd) ? (eol - pos + 1) : (headerEnd - pos);

        if (!ffStrStartsWithIgnCase(buffer->chars + pos, "Content-Length:") &&
            !ffStrStartsWithIgnCase(buffer->chars + pos, dropHeader)) {
            ffStrbufAppendNS(&newBuffer, lineLen, buffer->chars + pos);
        }

        pos += lineLen;
    }

    ffStrbufAppendF(&newBuffer, "Content-Length: %u\r\n\r\n", bodyLen);
    ffStrbufAppendNS(&newBuffer, bodyLen, body);

    ffStrbufDestroy(buffer);
    ffStrbufInitMove(buffer, &newBuffer);

    // The terminating CRLF CRLF is the four bytes in front of the body
    return buffer->length - bodyLen - 4;
}

bool ffNetworkingDecodeChunked(FFstrbuf* buffer, uint32_t* headerEnd) {
    assert(buffer->allocated > 0);

    // Held in a local because the rebuild below hands a different value back
    const uint32_t headerLength = *headerEnd;

    // The header block is terminated by CR LF CR LF
    if (headerLength + 4 > buffer->length) {
        return false;
    }

    char* body = buffer->chars + headerLength + 4;
    uint32_t bodyLen = buffer->length - headerLength - 4;

    uint32_t consumed = 0;
    if (ffNetworkingChunkedComplete(body, bodyLen, &consumed) != 1) {
        FF_DEBUG("Incomplete or malformed chunked body");
        return false;
    }

    // Decoding in place is safe: the encoded form is never shorter than the payload
    char* out = body;
    uint32_t outLen = 0;
    uint32_t pos = 0;

    while (pos < bodyLen) {
        uint32_t eol = pos;
        while (eol < bodyLen && body[eol] != '\n') {
            ++eol;
        }

        unsigned long size = strtoul(body + pos, nullptr, 16);
        pos = eol + 1;
        if (size == 0) {
            break; // last-chunk; trailers are dropped
        }

        memmove(out + outLen, body + pos, size);
        outLen += (uint32_t) size;
        pos += (uint32_t) size + 2; // trailing CRLF
    }

    FF_DEBUG("Decoded chunked body: %u bytes encoded, %u bytes decoded", bodyLen, outLen);
    *headerEnd = rebuildResponse(buffer, headerLength, out, outLen, "Transfer-Encoding:");
    return true;
}
