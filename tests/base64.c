#include "common/base64.h"
#include "common/strutil.h"
#include "common/textModifier.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void verify(bool expression, const char* expressionStr, int lineNo) {
    if (expression) {
        return;
    }

    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, expressionStr);
    exit(1);
}

#define VERIFY(expression) verify((expression), #expression, __LINE__)

// The empty string is a valid case and yields no bytes.
static uint32_t fromHex(const char* hex, uint8_t* out) {
    uint32_t length = 0;
    for (const char* p = hex; p[0] != '\0' && p[1] != '\0'; p += 2) {
        unsigned value = 0;
        if (sscanf(p, "%2x", &value) != 1) {
            return 0;
        }
        out[length++] = (uint8_t) value;
    }
    return length;
}

typedef struct FFBase64EncodingCase {
    const char* name;
    const char* hex;
    const char* expected;
} FFBase64EncodingCase;

// The RFC 4648 vectors cover the whole groups. The rest cover the partial group at the end of a
// source whose length is not a multiple of three, which is where the encoder used to go wrong: a
// plain `char` is signed on the supported platforms, so widening a byte with the high bit set into
// the 64 bit accumulator sign extended it, and the six bit fields read back out of that accumulator
// no longer held the byte's own bits.
//
// Only the two byte partial group was affected. One byte left over is shifted into place and never
// OR-ed into, so its sign extension lands above the fields that are read; two bytes left over OR the
// second byte in after shifting it by 8, which drags the sign bits down into them.
static const FFBase64EncodingCase ENCODING_CASES[] = {
    // RFC 4648 test vectors
    { "rfc4648-empty", "", "" },
    { "rfc4648-f", "66", "Zg==" },
    { "rfc4648-fo", "666f", "Zm8=" },
    { "rfc4648-foo", "666f6f", "Zm9v" },
    { "rfc4648-foob", "666f6f62", "Zm9vYg==" },
    { "rfc4648-fooba", "666f6f6261", "Zm9vYmE=" },
    { "rfc4648-foobar", "666f6f626172", "Zm9vYmFy" },

    // One byte left over (length % 3 == 1)
    { "single-80", "41414180", "QUFBgA==" },
    { "single-b9", "414141b9", "QUFBuQ==" },
    { "single-c6", "414141c6", "QUFBxg==" },
    { "single-ff", "414141ff", "QUFB/w==" },

    // Two bytes left over (length % 3 == 2), with the high bit on the second byte
    { "pair-second-80", "4141414180", "QUFBQYA=" },
    { "pair-second-9c", "414141419c", "QUFBQZw=" },
    { "pair-second-b9", "41414141b9", "QUFBQbk=" },
    { "pair-second-c6", "41414141c6", "QUFBQcY=" },
    { "pair-second-ff", "41414141ff", "QUFBQf8=" },

    // Two bytes left over (length % 3 == 2), with the high bit on the first byte
    { "pair-first-80", "4141418041", "QUFBgEE=" },
    { "pair-first-9c", "4141419c41", "QUFBnEE=" },
    { "pair-first-b9", "414141b941", "QUFBuUE=" },
    { "pair-first-c6", "414141c641", "QUFBxkE=" },
    { "pair-first-ff", "414141ff41", "QUFB/0E=" },

    // Two bytes left over (length % 3 == 2), with the high bit on both
    { "pair-both-80", "4141418080", "QUFBgIA=" },
    { "pair-both-b9", "414141b9b9", "QUFBubk=" },
    { "pair-both-c6", "414141c6c6", "QUFBxsY=" },
    { "pair-both-ff", "414141ffff", "QUFB//8=" },

    // The shape the kitty animation encoder hands over: a zlib stream for one 100x100 RGBA frame.
    // Its last two bytes (0x79 0xb9) are what the sign extension used to mangle. The terminal then
    // rejected the frame with a zlib checksum error, so the animation was transmitted correctly but
    // never played.
    { "kitty-animation-frame", "78daedd1310d00000cc3b0f227dd91a876f9308124490300000000000000000000000000000000000000231dd3d40f3ffcf0c30f3ffcf0c30f3ff0c30ffcf0c30f3ffcf0c30f3ffcf0033ffcc00f0000000000000000000000f876fa5e79b9", "eNrt0TENAAAMw7DyJ92RqHb5MIEkSQMAAAAAAAAAAAAAAAAAAAAAAAAAIx3T1A8//PDDDz/88MMPP/DDD/zwww8//PDDDz/88AM//MAPAAAAAAAAAAAAAAD4dvpeebk=" },
};

static void verifyEncodingCase(const FFBase64EncodingCase* testCase, int lineNo) {
    static uint8_t source[512];
    const uint32_t sourceLength = fromHex(testCase->hex, source);
    VERIFY(sourceLength <= sizeof(source));

    static char encoded[1024];
    uint32_t encodedLength = 0;
    memset(encoded, 0, sizeof(encoded));
    ffBase64EncodeRaw(sourceLength, (const char*) source, &encodedLength, encoded);

    if (!ffStrEquals(encoded, testCase->expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s: expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, testCase->name, testCase->expected, encoded);
        exit(1);
    }

    // The strbuf wrapper has to agree with the raw function
    FF_STRBUF_AUTO_DESTROY input = ffStrbufCreateNS(sourceLength, (const char*) source);
    FF_STRBUF_AUTO_DESTROY wrapped = ffBase64EncodeStrbuf(&input);
    if (!ffStrbufEqualS(&wrapped, testCase->expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s: strbuf wrapper got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, testCase->name, wrapped.chars);
        exit(1);
    }
}

// Covers every length, so a break in any of the three code paths (whole groups, one byte left
// over, two bytes left over) shows up at the length that reaches it. The pattern puts the high bit
// on roughly half of the bytes, which is what the partial groups are sensitive to.
static void verifyRoundTrip(void) {
    static uint8_t source[512];
    static char encoded[1024];
    static char decoded[512];

    for (uint32_t length = 0; length <= 400; ++length) {
        for (uint32_t i = 0; i < length; ++i) {
            source[i] = (uint8_t) (i * 37 + 0x80);
        }

        uint32_t encodedLength = 0;
        memset(encoded, 0, sizeof(encoded));
        ffBase64EncodeRaw(length, (const char*) source, &encodedLength, encoded);
        VERIFY(encodedLength == (length + 2) / 3 * 4);

        uint32_t decodedLength = 0;
        memset(decoded, 0, sizeof(decoded));
        ffBase64DecodeRaw(encodedLength, encoded, &decodedLength, decoded);
        VERIFY(decodedLength == length);
        VERIFY(memcmp(decoded, source, length) == 0);
    }
}

int main(void) {
    for (size_t i = 0; i < sizeof(ENCODING_CASES) / sizeof(ENCODING_CASES[0]); ++i) {
        verifyEncodingCase(&ENCODING_CASES[i], __LINE__);
    }

    verifyRoundTrip();

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
