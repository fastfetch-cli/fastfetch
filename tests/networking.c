#include "common/networking.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void verify(bool expression, const char* expressionStr, int lineNo) {
    if (expression) {
        return;
    }

    fprintf(stderr, "[%d] %s\n", lineNo, expressionStr);
    exit(1);
}

#define VERIFY(expression) verify((expression), #expression, __LINE__)

static FFNetworkingTransferEncoding parse(const char* value) {
    return ffNetworkingParseTransferEncoding(value, (uint32_t) strlen(value));
}

int main(void) {
    {
        VERIFY(parse("chunked") == FF_NETWORKING_TE_CHUNKED);
        VERIFY(parse("Chunked") == FF_NETWORKING_TE_CHUNKED);
        VERIFY(parse("  chunked  ") == FF_NETWORKING_TE_CHUNKED);
        VERIFY(parse("chunked\t") == FF_NETWORKING_TE_CHUNKED);

        // A coding chain leaves the payload encoded, so only a lone `chunked` is decodable
        VERIFY(parse("gzip, chunked") == FF_NETWORKING_TE_UNSUPPORTED);
        VERIFY(parse("gzip,chunked") == FF_NETWORKING_TE_UNSUPPORTED);
        VERIFY(parse("chunked, gzip") == FF_NETWORKING_TE_UNSUPPORTED);
        VERIFY(parse("gzip") == FF_NETWORKING_TE_UNSUPPORTED);
        VERIFY(parse("x-chunked") == FF_NETWORKING_TE_UNSUPPORTED);
        VERIFY(parse("chunkedx") == FF_NETWORKING_TE_UNSUPPORTED);

        VERIFY(parse("") == FF_NETWORKING_TE_NONE);
        VERIFY(parse(" , ") == FF_NETWORKING_TE_NONE);
    }

    {
        // The lookup is restricted to the header block, so a body that happens to contain a
        // `Transfer-Encoding:` line can never be mistaken for a header
        const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nTransfer-Encoding: chunked\r\n";
        uint32_t headerEnd = (uint32_t) (strstr(response, "\r\n\r\n") - response);
        uint32_t valueLen = 0;

        VERIFY(ffNetworkingFindHeader(response, headerEnd, "Transfer-Encoding:", &valueLen) == nullptr);

        const char* clHeader = ffNetworkingFindHeader(response, headerEnd, "Content-Length:", &valueLen);
        VERIFY(clHeader != nullptr && valueLen == 1 && *clHeader == '4');
    }

    {
        uint32_t consumed = 0;
        const char* complete = "5\r\nhello\r\n0\r\n\r\n";
        VERIFY(ffNetworkingChunkedComplete(complete, (uint32_t) strlen(complete), &consumed) == 1);
        VERIFY(consumed == strlen(complete));

        const char* withTrailer = "5\r\nhello\r\n0\r\nX-Foo: bar\r\n\r\n";
        VERIFY(ffNetworkingChunkedComplete(withTrailer, (uint32_t) strlen(withTrailer), &consumed) == 1);
        VERIFY(consumed == strlen(withTrailer));

        const char* partial = "5\r\nhel";
        VERIFY(ffNetworkingChunkedComplete(partial, (uint32_t) strlen(partial), &consumed) == 0);

        const char* malformed = "5\r\nhelloXX0\r\n\r\n";
        VERIFY(ffNetworkingChunkedComplete(malformed, (uint32_t) strlen(malformed), &consumed) == -1);
    }

    {
        // Decoding replaces the framing header with a matching Content-Length
        FFstrbuf response = ffStrbufCreateS("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n");
        uint32_t headerEnd = (uint32_t) (strstr(response.chars, "\r\n\r\n") - response.chars);

        VERIFY(ffNetworkingDecodeChunked(&response, &headerEnd));
        VERIFY(headerEnd == (uint32_t) (strstr(response.chars, "\r\n\r\n") - response.chars));
        VERIFY(ffStrbufContainS(&response, "Content-Length: 5"));
        VERIFY(!ffStrbufContainS(&response, "Transfer-Encoding"));
        VERIFY(ffStrbufEndsWithS(&response, "hello"));

        ffStrbufDestroy(&response);
    }

    puts("All networking tests passed!");
    return 0;
}
