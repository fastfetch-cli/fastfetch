#include "fastfetch.h"
#include "common/library.h"
#include "common/networking/networking.h"
#include "util/stringUtils.h"
#include "util/debug.h"

#ifdef FF_HAVE_ZLIB
#include <zlib.h>

struct FFZlibLibrary
{
    FF_LIBRARY_SYMBOL(inflateInit2_)
    FF_LIBRARY_SYMBOL(inflate)
    FF_LIBRARY_SYMBOL(inflateEnd)
    FF_LIBRARY_SYMBOL(inflateGetHeader)

    bool inited;
} zlibData;

const char* ffNetworkingLoadZlibLibrary(void)
{
    if (!zlibData.inited)
    {
        zlibData.inited = true;
        FF_LIBRARY_LOAD(zlib, "dlopen libz failed",
            #ifdef _WIN32
                "zlib1"
            #else
                "libz"
            #endif
            FF_LIBRARY_EXTENSION, 2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlib, zlibData, inflateInit2_)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlib, zlibData, inflate)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlib, zlibData, inflateEnd)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlib, zlibData, inflateGetHeader)
        zlib = NULL; // don't auto dlclose
    }
    return zlibData.ffinflateEnd == NULL ? "Failed to load libz" : NULL;
}

// Try to pre-read gzip header to determine uncompressed size
static uint32_t guessGzipOutputSize(const void* data, uint32_t dataSize)
{
    // gzip file format: http://www.zlib.org/rfc-gzip.html
    if (dataSize < 10 || ((const uint8_t*)data)[0] != 0x1f || ((const uint8_t*)data)[1] != 0x8b)
        return 0;

    // Uncompressed size in gzip format is stored in the last 4 bytes, but only valid if data is less than 4GB
    if (dataSize > 18) {
        // Get ISIZE value from the end of file (little endian)
        const uint8_t* tail = (const uint8_t*)data + dataSize - 4;
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
bool ffNetworkingDecompressGzip(FFstrbuf* buffer, char* headerEnd)
{
    // Calculate header size
    uint32_t headerSize = (uint32_t) (headerEnd - buffer->chars);

    *headerEnd = '\0'; // Replace delimiter with null character for easier processing
    // Ensure Content-Encoding is in response headers, not in response body
    bool hasGzip = strcasestr(buffer->chars, "\nContent-Encoding: gzip") != NULL;
    *headerEnd = '\r'; // Restore delimiter

    if (!hasGzip) {
        FF_DEBUG("No gzip compressed content detected, skipping decompression");
        return true;
    }

    FF_DEBUG("Gzip compressed content detected, preparing for decompression");

    const char* bodyStart = headerEnd + 4; // Skip delimiter

    // Calculate compressed content size
    uint32_t compressedSize = buffer->length - headerSize - 4;

    if (compressedSize <= 0) {
        // No content to decompress
        FF_DEBUG("Compressed content size is 0, skipping decompression");
        return true;
    }

    // Check if content is actually in gzip format (gzip header magic is 0x1f 0x8b)
    if (compressedSize < 2 || (uint8_t)bodyStart[0] != 0x1f || (uint8_t)bodyStart[1] != 0x8b) {
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
        .avail_in = (uInt)compressedSize,
        .next_in = (Bytef*)bodyStart,
        .avail_out = (uInt)ffStrbufGetFree(&decompressedBuffer),
        .next_out = (Bytef*)decompressedBuffer.chars,
    };

    // Initialize decompression engine
    if (zlibData.ffinflateInit2_(&zs, 16 + MAX_WBITS, ZLIB_VERSION, (int)sizeof(z_stream)) != Z_OK) {
        FF_DEBUG("Failed to initialize decompression engine");
        return false;
    }
    uInt availableOut = zs.avail_out;

    // Perform decompression
    int result = zlibData.ffinflate(&zs, Z_FINISH);

    // If output buffer is insufficient, try to extend buffer
    while (result == Z_BUF_ERROR || (result != Z_STREAM_END && zs.avail_out == 0))
    {
        FF_DEBUG("Output buffer insufficient, trying to extend");

        // Save already decompressed data amount
        uint32_t alreadyDecompressed = (uint32_t)(availableOut - zs.avail_out);
        decompressedBuffer.length += alreadyDecompressed;
        decompressedBuffer.chars[decompressedBuffer.length] = '\0'; // Ensure null-terminated string

        ffStrbufEnsureFree(&decompressedBuffer, decompressedBuffer.length / 2);

        // Set output parameters to point to new buffer
        zs.avail_out = (uInt)ffStrbufGetFree(&decompressedBuffer);
        zs.next_out = (Bytef*)(decompressedBuffer.chars + decompressedBuffer.length);
        availableOut = zs.avail_out;

        // Decompress again
        result = zlibData.ffinflate(&zs, Z_FINISH);
    }

    zlibData.ffinflateEnd(&zs);

    // Calculate decompressed size
    uint32_t decompressedSize = (uint32_t)(availableOut - zs.avail_out);
    decompressedBuffer.length += decompressedSize;
    decompressedBuffer.chars[decompressedSize] = '\0'; // Ensure null-terminated string
    FF_DEBUG("Successfully decompressed %u bytes compressed data to %u bytes", compressedSize, decompressedBuffer.length);

    // Modify Content-Length header and remove Content-Encoding header
    FF_STRBUF_AUTO_DESTROY newBuffer = ffStrbufCreateA(headerSize + decompressedSize + 64);

    char* line = NULL;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, buffer))
    {
        if (ffStrStartsWithIgnCase(line, "Content-Encoding:"))
        {
            continue;
        }
        else if (ffStrStartsWithIgnCase(line, "Content-Length:"))
        {
            ffStrbufAppendF(&newBuffer, "Content-Length: %u\r\n", decompressedSize);
            continue;
        }
        else if (line[0] == '\r')
        {
            ffStrbufAppendS(&newBuffer, "\r\n");
            ffStrbufGetlineRestore(&line, &len, buffer);
            break;
        }

        ffStrbufAppendS(&newBuffer, line);
        ffStrbufAppendC(&newBuffer, '\n');
    }

    ffStrbufAppend(&newBuffer, &decompressedBuffer);
    ffStrbufDestroy(buffer);
    ffStrbufInitMove(buffer, &newBuffer);

    return true;
}
#endif // FF_HAVE_ZLIB
