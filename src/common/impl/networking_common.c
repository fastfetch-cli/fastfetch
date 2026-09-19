#include "fastfetch.h"
#include "common/library.h"
#include "common/networking.h"
#include "common/strutil.h"
#include "common/debug.h"

#ifdef FF_HAVE_ZLIB
    #include <zlib.h>

struct FFZlibLibrary {
    FF_LIBRARY_SYMBOL(inflateInit2_)
    FF_LIBRARY_SYMBOL(inflate)
    FF_LIBRARY_SYMBOL(inflateEnd)

    bool inited;
} zlibData;

const char* ffNetworkingLoadZlibLibrary(void) {
    if (!zlibData.inited) {
        zlibData.inited = true;
        FF_LIBRARY_LOAD_MESSAGE(zlib,
    #ifdef _WIN32
            "zlib1"
    #else
            "libz"
    #endif
            FF_LIBRARY_EXTENSION,
            2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlib, zlibData, inflateInit2_)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlib, zlibData, inflate)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlib, zlibData, inflateEnd)
        zlib = nullptr; // don't auto dlclose
    }
    return zlibData.ffinflateEnd == nullptr ? "Failed to load libz" : nullptr;
}

// Try to pre-read gzip header to determine uncompressed size
static uint32_t guessGzipOutputSize(const void* data, uint32_t dataSize) {
    // gzip file format: http://www.zlib.org/rfc-gzip.html
    if (dataSize < 10 || ((const uint8_t*) data)[0] != 0x1f || ((const uint8_t*) data)[1] != 0x8b) {
        return 0;
    }

    // Uncompressed size in gzip format is stored in the last 4 bytes, but only valid if data is less than 4GB
    if (dataSize > 18) {
        // Get ISIZE value from the end of file (little endian)
        const uint8_t* tail = (const uint8_t*) data + dataSize - 4;
        uint32_t uncompressedSize = (uint32_t) tail[0] | ((uint32_t) tail[1] << 8u) | ((uint32_t) tail[2] << 16u) | ((uint32_t) tail[3] << 24u);

        // For valid gzip files, this value is the length of the uncompressed data modulo 2^32
        if (uncompressedSize > 0) {
            FF_DEBUG("Read uncompressed size from GZIP trailer: %u bytes", uncompressedSize);
            // Add some margin to the estimated size for safety
            return uncompressedSize + 64;
        }
    }

    // If unable to get size from trailer or size is 0, use estimated value
    // Typically, text data compression ratio is between 3-5x, we use the larger value
    uint32_t estimatedSize = dataSize * 5;
    FF_DEBUG("Unable to read exact uncompressed size, estimated as 5x of compressed data: %u bytes", estimatedSize);
    return estimatedSize;
}

// Decompress gzip content
bool ffNetworkingDecompressGzip(FFstrbuf* buffer, char* headerEnd) {
    // `headerEnd` itself is non-null per the `nonnull(2)` contract; what it points to is not expressible as an attribute
    assert(*headerEnd == '\r');

    // Calculate header size
    uint32_t headerSize = (uint32_t) (headerEnd - buffer->chars);

    *headerEnd = '\0'; // Replace delimiter with null character for easier processing
    // Ensure Content-Encoding is in response headers, not in response body
    bool hasGzip = strcasestr(buffer->chars, "\nContent-Encoding: gzip") != nullptr;
    *headerEnd = '\r'; // Restore delimiter

    if (!hasGzip) {
        FF_DEBUG("No gzip compressed content detected, skipping decompression");
        return true;
    }

    FF_DEBUG("Gzip compressed content detected, preparing for decompression");

    const char* bodyStart = headerEnd + 4; // Skip delimiter

    if (buffer->length <= headerSize + 4) {
        // No content to decompress
        FF_DEBUG("Compressed content size is 0, skipping decompression");
        return true;
    }

    // Calculate compressed content size
    uint32_t compressedSize = buffer->length - headerSize - 4;

    // Check if content is actually in gzip format (gzip header magic is 0x1f 0x8b)
    if (compressedSize < 2 || (uint8_t) bodyStart[0] != 0x1f || (uint8_t) bodyStart[1] != 0x8b) {
        FF_DEBUG("Content is not valid gzip format, skipping decompression");
        return false;
    }

    // Predict uncompressed size
    uint32_t estimatedSize = guessGzipOutputSize(bodyStart, compressedSize);

    // Create decompression buffer with estimated size
    FF_STRBUF_AUTO_DESTROY decompressedBuffer = ffStrbufCreateA(estimatedSize > 0 ? estimatedSize : compressedSize * 5);
    FF_DEBUG("Created decompression buffer: %u bytes", decompressedBuffer.allocated);

    // Initialize decompression
    z_stream zs = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
        .avail_in = (uInt) compressedSize,
        .next_in = (Bytef*) bodyStart,
        .avail_out = (uInt) ffStrbufGetFree(&decompressedBuffer),
        .next_out = (Bytef*) decompressedBuffer.chars,
    };

    // Initialize decompression engine
    if (zlibData.ffinflateInit2_(&zs, 16 + MAX_WBITS, ZLIB_VERSION, (int) sizeof(z_stream)) != Z_OK) {
        FF_DEBUG("Failed to initialize decompression engine");
        return false;
    }
    uInt availableOut = zs.avail_out;

    // Perform decompression
    int result = zlibData.ffinflate(&zs, Z_FINISH);

    // If output buffer is insufficient, try to extend buffer
    while (result == Z_BUF_ERROR || (result != Z_STREAM_END && zs.avail_out == 0)) {
        FF_DEBUG("Output buffer insufficient, trying to extend");

        // Save already decompressed data amount
        uint32_t alreadyDecompressed = (uint32_t) (availableOut - zs.avail_out);
        decompressedBuffer.length += alreadyDecompressed;
        decompressedBuffer.chars[decompressedBuffer.length] = '\0';

        ffStrbufEnsureFree(&decompressedBuffer, decompressedBuffer.length / 2);

        // Set output parameters to point to new buffer
        zs.avail_out = (uInt) ffStrbufGetFree(&decompressedBuffer);
        zs.next_out = (Bytef*) (decompressedBuffer.chars + decompressedBuffer.length);
        availableOut = zs.avail_out;

        // Decompress again
        result = zlibData.ffinflate(&zs, Z_FINISH);
    }

    // Check for decompression errors before using result
    if (result != Z_STREAM_END) {
        FF_DEBUG("Decompression failed with zlib error: %d", result);
        zlibData.ffinflateEnd(&zs);
        return false;
    }

    zlibData.ffinflateEnd(&zs);

    // Calculate decompressed size (from the last inflate call)
    uint32_t decompressedSize = (uint32_t) (availableOut - zs.avail_out);
    decompressedBuffer.length += decompressedSize;
    decompressedBuffer.chars[decompressedBuffer.length] = '\0';
    FF_DEBUG("Successfully decompressed %u bytes compressed data to %u bytes", compressedSize, decompressedBuffer.length);

    // Modify Content-Length header and remove Content-Encoding header
    // Use decompressedBuffer.length (total) not decompressedSize (last chunk only)
    FF_STRBUF_AUTO_DESTROY newBuffer = ffStrbufCreateA(headerSize + decompressedBuffer.length + 64);

    char* line = nullptr;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, buffer)) {
        if (ffStrStartsWithIgnCase(line, "Content-Encoding:")) {
            continue;
        } else if (ffStrStartsWithIgnCase(line, "Content-Length:")) {
            ffStrbufAppendF(&newBuffer, "Content-Length: %u\r\n", decompressedBuffer.length);
            continue;
        } else if (line[0] == '\r') {
            ffStrbufAppendS(&newBuffer, "\r\n");
            ffStrbufGetlineRestore(&line, &len, buffer);
            break;
        }

        ffStrbufAppendS(&newBuffer, line); // Including the trailing \r
        ffStrbufAppendC(&newBuffer, '\n');
    }

    ffStrbufAppend(&newBuffer, &decompressedBuffer);
    ffStrbufDestroy(buffer);
    ffStrbufInitMove(buffer, &newBuffer);

    return true;
}
#endif // FF_HAVE_ZLIB

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
static void rebuildResponse(FFstrbuf* buffer, uint32_t headerEnd, const char* body, uint32_t bodyLen, const char* dropHeader) {
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
}

bool ffNetworkingDecodeChunked(FFstrbuf* buffer, uint32_t headerEnd) {
    assert(buffer->allocated > 0);

    // The header block is terminated by CR LF CR LF
    if (headerEnd + 4 > buffer->length) {
        return false;
    }

    char* body = buffer->chars + headerEnd + 4;
    uint32_t bodyLen = buffer->length - headerEnd - 4;

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
    rebuildResponse(buffer, headerEnd, out, outLen, "Transfer-Encoding:");
    return true;
}
