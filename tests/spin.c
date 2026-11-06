#include "fastfetch.h"
#include "logo/logo.h"
#include "logo/spin/render.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Unlike assert(), this also checks in release builds.
#define VERIFY(expression)                                                    \
    do {                                                                      \
        if (!(expression)) {                                                  \
            fprintf(stderr, "spin test failed [%d]: %s\n", __LINE__, #expression); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static bool isWallGlyph(uint32_t glyph) {
    return glyph >= 0x2591 && glyph <= 0x2593 ? true : glyph == 0x2588; // ░▒▓█
}

static void testRotation(void) {
    const float angles[] = { 0.0f, 0.25f, (float) M_PI / 2.0f, (float) M_PI, 5.0f };
    const float tilts[] = { 0.0f, 0.2f, -0.4f, 1.0f };
    for (uint32_t i = 0; i < ARRAY_SIZE(angles); ++i) {
        for (uint32_t j = 0; j < ARRAY_SIZE(tilts); ++j) {
            float rotation[3][3], inverse[3][3];
            ffSpinRotationMatrix(angles[i], tilts[j], rotation, inverse);
            VERIFY(ffSpinRotationIsOrthonormal(rotation, 1e-4f));
            for (uint32_t r = 0; r < 3; ++r) {
                for (uint32_t c = 0; c < 3; ++c) {
                    VERIFY(fabsf(rotation[r][c] - inverse[c][r]) < 1e-5f);
                }
            }
        }
    }
}

static void testParse(void) {
    FFstrbuf colors[2];
    ffStrbufInitStatic(&colors[0], "31");
    ffStrbufInitStatic(&colors[1], "1;34");

    FFSpinArt art;
    // Leading blank rows and columns are cropped; `$$` is a literal dollar.
    VERIFY(ffSpinArtParse(&art, "\n   $1ab\n   $2c$$\xe2\x96\x88\n", colors, 2));
    VERIFY(art.width == 3 && art.height == 2);
    VERIFY(art.glyphs[0] == 'a' && art.colors[0] == 1);
    VERIFY(art.glyphs[1] == 'b' && art.colors[1] == 1);
    VERIFY(art.glyphs[2] == 0);
    VERIFY(art.glyphs[3] == 'c' && art.colors[3] == 2);
    VERIFY(art.glyphs[4] == '$');
    VERIFY(art.glyphs[5] == 0x2588);
    VERIFY(ffStrbufEqualS(&art.palette[2], "1;34"));
    ffSpinArtDestroy(&art);

    // Raw SGR escapes in custom logo files get their own palette slots.
    VERIFY(ffSpinArtParse(&art, "\e[38;5;208mx\e[0my", nullptr, 0));
    VERIFY(art.width == 2 && art.height == 1);
    VERIFY(art.colors[0] >= FF_SPIN_PALETTE_LOGO_SLOTS);
    VERIFY(ffStrbufEqualS(&art.palette[art.colors[0]], "38;5;208"));
    VERIFY(art.colors[1] == 0);
    ffSpinArtDestroy(&art);

    VERIFY(!ffSpinArtParse(&art, "  \n \t \n", nullptr, 0));
    ffSpinArtDestroy(&art);
    VERIFY(!ffSpinArtParse(&art, "", nullptr, 0));
    ffSpinArtDestroy(&art);
}

static void testMirror(void) {
    VERIFY(ffSpinMirrorGlyph('/') == '\\');
    VERIFY(ffSpinMirrorGlyph('\\') == '/');
    VERIFY(ffSpinMirrorGlyph('(') == ')');
    VERIFY(ffSpinMirrorGlyph('o') == 'o');
    VERIFY(ffSpinMirrorGlyph(0x2801) == 0x2808); // braille dot 1 -> dot 4
    VERIFY(ffSpinMirrorGlyph(0x2840) == 0x2880); // braille dot 7 -> dot 8
    for (uint32_t glyph = 0x2800; glyph <= 0x28FF; ++glyph) {
        VERIFY(ffSpinMirrorGlyph(ffSpinMirrorGlyph(glyph)) == glyph);
    }
}

static const char testLogo[] =
    "$1    /\\\n"
    "$1   /  \\\n"
    "$1  /\\   \\\n"
    "$2 /  __  \\\n"
    "$2/__/  \\__\\";

static void testRenderFaces(void) {
    FFstrbuf colors[2];
    ffStrbufInitStatic(&colors[0], "36");
    ffStrbufInitStatic(&colors[1], "34");
    FFSpinArt art;
    VERIFY(ffSpinArtParse(&art, testLogo, colors, 2));

    uint32_t width, height;
    ffSpinFrameSize(&art, 0.0f, &width, &height);
    VERIFY(width >= art.width && height >= art.height);
    VERIFY((width - art.width) % 2 == 0 && (height - art.height) % 2 == 0);
    uint32_t left = (width - art.width) / 2, top = (height - art.height) / 2;

    FFSpinFrame frame;
    VERIFY(ffSpinFrameInit(&frame, width, height));

    // Face-on, every cell of the art shows up unchanged at its place.
    ffSpinRenderArt(&art, 0.0f, 0.0f, &frame);
    for (uint32_t y = 0; y < art.height; ++y) {
        for (uint32_t x = 0; x < art.width; ++x) {
            uint32_t glyph = art.glyphs[y * art.width + x];
            if (!glyph) continue;
            size_t index = (size_t) (top + y) * width + left + x;
            VERIFY(frame.glyphs[index] == glyph);
            VERIFY(frame.colors[index] == art.colors[y * art.width + x]);
        }
    }

    // Turned around, the back face shows the art mirrored, glyphs included.
    ffSpinRenderArt(&art, (float) M_PI, 0.0f, &frame);
    for (uint32_t y = 0; y < art.height; ++y) {
        for (uint32_t x = 0; x < art.width; ++x) {
            uint32_t glyph = art.glyphs[y * art.width + x];
            if (!glyph) continue;
            size_t index = (size_t) (top + y) * width + left + (art.width - 1 - x);
            VERIFY(frame.glyphs[index] == ffSpinMirrorGlyph(glyph));
        }
    }

    // Edge-on, only a thin band of edge shading is left.
    ffSpinRenderArt(&art, (float) M_PI / 2.0f, 0.0f, &frame);
    uint32_t usedColumns = 0;
    for (uint32_t x = 0; x < width; ++x) {
        bool used = false;
        for (uint32_t y = 0; y < height; ++y) {
            uint32_t glyph = frame.glyphs[y * width + x];
            if (glyph) {
                used = true;
                VERIFY(isWallGlyph(glyph));
            }
        }
        usedColumns += used;
    }
    VERIFY(usedColumns > 0 && usedColumns <= 4);

    ffSpinFrameDestroy(&frame);
    ffSpinArtDestroy(&art);
}

// Renders into a frame one cell larger on every side than ffSpinFrameSize
// asks for; the extra ring must stay empty at every angle.
static void verifyFits(const FFSpinArt* art, float tilt) {
    uint32_t width, height;
    ffSpinFrameSize(art, tilt, &width, &height);
    FFSpinFrame frame;
    VERIFY(ffSpinFrameInit(&frame, width + 2, height + 2));
    for (uint32_t step = 0; step < 48; ++step) {
        ffSpinRenderArt(art, (float) step * 2.0f * (float) M_PI / 48.0f, tilt, &frame);
        for (uint32_t x = 0; x < frame.width; ++x) {
            VERIFY(frame.glyphs[x] == 0);
            VERIFY(frame.glyphs[(size_t) (frame.height - 1) * frame.width + x] == 0);
        }
        for (uint32_t y = 0; y < frame.height; ++y) {
            VERIFY(frame.glyphs[(size_t) y * frame.width] == 0);
            VERIFY(frame.glyphs[(size_t) y * frame.width + frame.width - 1] == 0);
        }
    }
    ffSpinFrameDestroy(&frame);
}

static void testBuiltinLogos(void) {
    uint32_t count = 0;
    for (uint32_t letter = 0; letter < 26; ++letter) {
        for (const FFlogo* logo = ffLogoBuiltins[letter]; *logo->names; ++logo) {
            FFSpinArt art;
            VERIFY(ffSpinArtParse(&art, logo->lines, nullptr, 0));
            VERIFY(art.width > 0 && art.height > 0);
            verifyFits(&art, 0.0f);
            if (count % 16 == 0) {
                verifyFits(&art, 0.4f);
            }
            ffSpinArtDestroy(&art);
            ++count;
        }
    }
    VERIFY(count > 400);
}

static void testAnsiOutput(void) {
    FFstrbuf colors[1];
    ffStrbufInitStatic(&colors[0], "35");
    FFSpinArt art;
    VERIFY(ffSpinArtParse(&art, "a\xe2\x96\x88 b", colors, 1));
    FFSpinFrame frame;
    VERIFY(ffSpinFrameInit(&frame, art.width, art.height));
    ffSpinFramePutArt(&frame, &art);

    FF_STRBUF_AUTO_DESTROY plain = ffStrbufCreate();
    ffSpinFrameToAnsi(&frame, &art, false, false, true, &plain);
    VERIFY(ffStrbufEqualS(&plain, "a\xe2\x96\x88 b\n"));

    FF_STRBUF_AUTO_DESTROY colored = ffStrbufCreate();
    ffSpinFrameToAnsi(&frame, &art, true, true, true, &colored);
    VERIFY(strstr(colored.chars, "\e[0;1;35ma") != nullptr);
    VERIFY(strstr(colored.chars, "\e[0m\n") != nullptr);

    ffSpinFrameDestroy(&frame);
    ffSpinArtDestroy(&art);
}

int main(void) {
    testRotation();
    testParse();
    testMirror();
    testRenderFaces();
    testBuiltinLogos();
    testAnsiOutput();
    puts("spin tests passed");
    return 0;
}
