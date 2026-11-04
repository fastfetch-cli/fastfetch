#pragma once

#include "common/FFstrbuf.h"

#include <stdbool.h>
#include <stdint.h>

#define FF_SPIN_PALETTE_MAX 32
// Palette slots 1..9 hold the logo's $1..$9 colors; raw SGR sequences found in
// custom logo files get the slots after that. Slot 0 is the terminal default.
#define FF_SPIN_PALETTE_LOGO_SLOTS 10

// A logo's ASCII art as a grid of cells. The renderer extrudes it into a slab.
typedef struct FFSpinArt {
    uint32_t width;
    uint32_t height;
    uint32_t* glyphs; // Unicode code point per cell, 0 = empty
    uint8_t* colors;  // palette index per cell
    uint32_t paletteCount;
    FFstrbuf palette[FF_SPIN_PALETTE_MAX]; // SGR parameters, e.g. "1;34"
} FFSpinArt;

typedef struct FFSpinFrame {
    uint32_t width;
    uint32_t height;
    uint32_t* glyphs;
    uint8_t* colors;
} FFSpinFrame;

// `colors` holds the logo's $1..$9 colors (may be nullptr). Returns false for
// empty art or when memory runs out.
bool ffSpinArtParse(FFSpinArt* art, const char* text, const FFstrbuf* colors, uint32_t colorCount);
void ffSpinArtDestroy(FFSpinArt* art);

bool ffSpinFrameInit(FFSpinFrame* frame, uint32_t width, uint32_t height);
void ffSpinFrameDestroy(FFSpinFrame* frame);

// Frame size that keeps the art inside the frame at every yaw for this tilt.
// The art is centered; the extra space is the same on both sides.
void ffSpinFrameSize(const FFSpinArt* art, float tilt, uint32_t* width, uint32_t* height);
// Copies the art face-on into the center of the frame, exactly as authored.
void ffSpinFramePutArt(FFSpinFrame* frame, const FFSpinArt* art);
void ffSpinRenderArt(const FFSpinArt* art, float yaw, float tilt, FFSpinFrame* frame);

void ffSpinRotationMatrix(float yaw, float tilt, float out[3][3], float outInverse[3][3]);
bool ffSpinRotationIsOrthonormal(const float matrix[3][3], float epsilon);
uint32_t ffSpinMirrorGlyph(uint32_t glyph);

// Appends `count` cells starting at `index` with colors, ending in a reset.
void ffSpinAppendCells(FFstrbuf* out, const FFSpinFrame* frame, const FFSpinArt* art, uint32_t index, uint32_t count, bool color, bool bold);
// One line per frame row, each ending in '\n'. `trimRight` drops the empty cells at the end of a
// row, which is what the other text logo types print; a frame that is redrawn as it turns must
// keep its full width instead, or the information column would move with the angle.
void ffSpinFrameToAnsi(const FFSpinFrame* frame, const FFSpinArt* art, bool color, bool bold, bool trimRight, FFstrbuf* out);
