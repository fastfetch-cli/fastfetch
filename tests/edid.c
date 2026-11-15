#include "common/edidHelper.h"
#include "common/textModifier.h"

#include <stdbool.h>
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

// Build a zeroed EDID with a valid base block. The buffer is allocated with the
// exact EDID length, so an ASan build also catches reads past the end of it.
static uint8_t* newEdid(uint32_t length) {
    uint8_t* edid = (uint8_t*) calloc(length, 1);
    VERIFY(edid != NULL);

    static const uint8_t header[] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
    memcpy(edid, header, sizeof(header));
    edid[126] = (uint8_t) (length / 128 - 1);
    uint8_t sum = 0;
    for (uint32_t i = 0; i < 127; ++i) {
        sum += edid[i];
    }
    edid[127] = (uint8_t) (0u - sum);
    return edid;
}

static uint8_t* addCta(uint8_t* edid, uint32_t base, uint8_t revision, uint8_t dtdOffset) {
    uint8_t* cta = edid + base;
    cta[0] = 0x02;
    cta[1] = revision;
    cta[2] = dtdOffset;
    return cta;
}

static void testNoExtension(void) {
    uint8_t* edid = newEdid(128);
    VERIFY(!ffEdidGetHdrCompatible(edid, 128));
    free(edid);
}

static void testPartialBlocks(void) {
    // A trailing block shorter than 128 bytes must be ignored entirely; even
    // reading its tag / revision / DTD offset used to be out of bounds.
    const uint32_t lengths[] = { 129, 130, 200, 255, 257, 300 };
    for (uint32_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); ++i) {
        uint32_t length = lengths[i];
        uint8_t* edid = newEdid(length);
        uint8_t* cta = edid + 128;
        cta[0] = 0x02;
        if (length >= 130) {
            cta[1] = 0x03;
        }
        if (length >= 131) {
            cta[2] = 0xFF;
        }
        VERIFY(!ffEdidGetHdrCompatible(edid, length));
        free(edid);
    }
}

static void testHdrDetection(void) {
    uint8_t* edid = newEdid(256);
    uint8_t* cta = addCta(edid, 128, 3, 7);
    cta[4] = 0xE2; // extended tag, length 2
    cta[5] = 0x06; // HDR SMDB
    cta[6] = 0x00;
    VERIFY(ffEdidGetHdrCompatible(edid, 256));

    cta[5] = 0x07; // HDR DMDB
    VERIFY(ffEdidGetHdrCompatible(edid, 256));

    cta[5] = 0x01; // unrelated extended tag
    VERIFY(!ffEdidGetHdrCompatible(edid, 256));

    cta[5] = 0x06;
    cta[1] = 0x02; // data block collection requires CTA revision 3
    VERIFY(!ffEdidGetHdrCompatible(edid, 256));

    cta[1] = 0x03;
    cta[0] = 0x70; // not a CTA extension
    VERIFY(!ffEdidGetHdrCompatible(edid, 256));
    free(edid);
}

static void testMultipleExtensions(void) {
    uint8_t* edid = newEdid(384);
    edid[128] = 0x70; // some other extension type
    uint8_t* cta = addCta(edid, 256, 3, 7);
    cta[4] = 0xE2;
    cta[5] = 0x06;
    VERIFY(ffEdidGetHdrCompatible(edid, 384));
    free(edid);
}

static void testEmptyDataBlockCollection(void) {
    // CTA-861 version 3: d == 0 means neither DTDs nor data blocks are present,
    // and d == 4 means the data block collection is empty.
    uint8_t* edid = newEdid(256);
    uint8_t* cta = addCta(edid, 128, 3, 0);
    cta[4] = 0xE2;
    cta[5] = 0x06;
    VERIFY(!ffEdidGetHdrCompatible(edid, 256));

    cta[2] = 4;
    VERIFY(!ffEdidGetHdrCompatible(edid, 256));
    free(edid);
}

static void testOutOfRangeOffset(void) {
    // An out-of-range DTD offset must be clamped to the CTA block. A valid HDR
    // block inside that block is still detected.
    uint8_t* edid = newEdid(256);
    uint8_t* cta = addCta(edid, 128, 3, 0xFF);
    cta[4] = 0xE2;
    cta[5] = 0x06;
    VERIFY(ffEdidGetHdrCompatible(edid, 256));

    // ... but bytes past the block must not be used as data blocks. These two
    // bytes live in the next extension block, 44 bytes into it.
    uint8_t* large = newEdid(512);
    addCta(large, 128, 3, 0xFF);
    large[300] = 0xE2;
    large[301] = 0x06;
    VERIFY(!ffEdidGetHdrCompatible(large, 512));
    free(large);
    free(edid);
}

static void testExtendedTagReadBounded(void) {
    // A block header at i == d - 1 has no room for its extended tag inside the
    // collection; byte d is the first DTD byte and must not be read as a tag.
    uint8_t* edid = newEdid(256);
    uint8_t* cta = addCta(edid, 128, 3, 10);
    cta[9] = 0xE2;
    cta[10] = 0x06;
    VERIFY(!ffEdidGetHdrCompatible(edid, 256));
    free(edid);
}

static void testNoCounterWrap(void) {
    // The old loop counter was uint8_t: at i == 254 a block length of 1 wrapped
    // it back to 0, making the scan loop forever.
    uint8_t* edid = newEdid(512);
    addCta(edid, 128, 3, 0xFF);
    edid[382] = 0x01; // cta[254] of the first extension
    VERIFY(!ffEdidGetHdrCompatible(edid, 512));
    free(edid);
}

int main(void) {
    testNoExtension();
    testPartialBlocks();
    testHdrDetection();
    testMultipleExtensions();
    testEmptyDataBlockCollection();
    testOutOfRangeOffset();
    testExtendedTagReadBounded();
    testNoCounterWrap();

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
