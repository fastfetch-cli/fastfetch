#include "logo/spin/render.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// Terminal cells are about twice as tall as wide. World units are cell
// heights, so one column is half a unit wide.
#define FF_SPIN_CELL_ASPECT 0.5f
// Camera distance in bounding-sphere radii. Smaller means stronger perspective.
#define FF_SPIN_CAMERA_RADII 4.0f

// Shading for the slab's edges, dark to bright. The faces keep the logo's glyphs.
static const uint32_t FFSpinWallRamp[] = { 0x2591, 0x2592, 0x2593, 0x2588 }; // ░▒▓█
static const float FFSpinLight[3] = { -0.48f, 0.6f, 0.64f }; // unit length, camera space

typedef struct SpinGeometry {
    float hx, hy, hz; // half extents of the slab
    float camera;     // camera distance from the slab center
    float focal;      // maps the front face 1:1 onto cells at yaw 0
} SpinGeometry;

static SpinGeometry spinGeometry(const FFSpinArt* art) {
    SpinGeometry g;
    g.hx = (float) art->width * FF_SPIN_CELL_ASPECT * 0.5f;
    g.hy = (float) art->height * 0.5f;
    g.hz = fmaxf(0.3f, fmaxf(g.hx, g.hy) * 0.055f);
    float radius = sqrtf(g.hx * g.hx + g.hy * g.hy + g.hz * g.hz);
    g.camera = radius * FF_SPIN_CAMERA_RADII;
    g.focal = g.camera - g.hz;
    return g;
}

static uint32_t utf8Decode(const char** cursor) {
    const unsigned char* s = (const unsigned char*) *cursor;
    uint32_t length, glyph;
    if (s[0] < 0x80) {
        *cursor += 1;
        return s[0];
    }
    if ((s[0] & 0xE0) == 0xC0) {
        length = 2;
        glyph = s[0] & 0x1F;
    } else if ((s[0] & 0xF0) == 0xE0) {
        length = 3;
        glyph = s[0] & 0x0F;
    } else if ((s[0] & 0xF8) == 0xF0) {
        length = 4;
        glyph = s[0] & 0x07;
    } else {
        *cursor += 1;
        return '?';
    }
    for (uint32_t i = 1; i < length; ++i) {
        if ((s[i] & 0xC0) != 0x80) {
            *cursor += i;
            return '?';
        }
        glyph = (glyph << 6) | (s[i] & 0x3F);
    }
    *cursor += length;
    return glyph;
}

static bool glyphIsBlank(uint32_t glyph) {
    return glyph <= ' ' || glyph == 0x7F || glyph == 0xA0 || glyph == 0x2800;
}

static uint8_t paletteAdd(FFSpinArt* art, const char* params, uint32_t length) {
    if (length == 0 || (length == 1 && params[0] == '0')) {
        return 0;
    }
    for (uint32_t i = FF_SPIN_PALETTE_LOGO_SLOTS; i < art->paletteCount; ++i) {
        if (art->palette[i].length == length && memcmp(art->palette[i].chars, params, length) == 0) {
            return (uint8_t) i;
        }
    }
    if (art->paletteCount >= FF_SPIN_PALETTE_MAX) {
        return 0;
    }
    ffStrbufSetNS(&art->palette[art->paletteCount], length, params);
    return (uint8_t) art->paletteCount++;
}

// Walks fastfetch logo text ($1..$9 colors, `$$`, tabs, SGR escapes). With
// `fill` false it only measures the grid.
static void artWalk(FFSpinArt* art, const char* text, bool fill, uint32_t* outWidth, uint32_t* outHeight) {
    uint32_t row = 0, col = 0, width = 0;
    uint8_t color = 1; // fastfetch starts every logo in $1
    const char* p = text;
    while (*p) {
        if (*p == '\r') {
            ++p;
            continue;
        }
        if (*p == '\n') {
            if (col > width) width = col;
            ++row;
            col = 0;
            ++p;
            continue;
        }
        if (*p == '\t') {
            col += 4;
            ++p;
            continue;
        }
        if (*p == '\e') {
            if (p[1] != '[') {
                ++p;
                continue;
            }
            const char* params = p + 2;
            const char* end = params;
            while ((*end >= '0' && *end <= '9') || *end == ';') {
                ++end;
            }
            if (*end == 'm' && fill) {
                color = paletteAdd(art, params, (uint32_t) (end - params));
            }
            p = *end ? end + 1 : end;
            continue;
        }
        if (*p == '$') {
            if (p[1] >= '1' && p[1] <= '9') {
                color = (uint8_t) (p[1] - '0');
                p += 2;
                continue;
            }
            if (p[1] == '$') {
                ++p; // `$$` is a literal dollar sign
            }
        }
        uint32_t glyph = utf8Decode(&p);
        if (fill && !glyphIsBlank(glyph)) {
            size_t index = (size_t) row * art->width + col;
            art->glyphs[index] = glyph;
            art->colors[index] = color;
        }
        ++col;
    }
    if (col > width) width = col;
    if (col > 0) ++row;
    *outWidth = width;
    *outHeight = row;
}

// Drops blank rows and columns around the art, so it spins around its center.
static bool artCrop(FFSpinArt* art) {
    uint32_t minX = UINT32_MAX, minY = UINT32_MAX, maxX = 0, maxY = 0;
    for (uint32_t y = 0; y < art->height; ++y) {
        for (uint32_t x = 0; x < art->width; ++x) {
            if (art->glyphs[(size_t) y * art->width + x] == 0) continue;
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
    }
    if (minX == UINT32_MAX) {
        return false;
    }
    uint32_t width = maxX - minX + 1, height = maxY - minY + 1;
    if (width == art->width && height == art->height) {
        return true;
    }
    uint32_t* glyphs = (uint32_t*) malloc((size_t) width * height * sizeof(uint32_t));
    uint8_t* colors = (uint8_t*) malloc((size_t) width * height);
    if (!glyphs || !colors) {
        free(glyphs);
        free(colors);
        return false;
    }
    for (uint32_t y = 0; y < height; ++y) {
        size_t from = (size_t) (minY + y) * art->width + minX;
        memcpy(glyphs + (size_t) y * width, art->glyphs + from, width * sizeof(uint32_t));
        memcpy(colors + (size_t) y * width, art->colors + from, width);
    }
    free(art->glyphs);
    free(art->colors);
    art->glyphs = glyphs;
    art->colors = colors;
    art->width = width;
    art->height = height;
    return true;
}

bool ffSpinArtParse(FFSpinArt* art, const char* text, const FFstrbuf* colors, uint32_t colorCount) {
    memset(art, 0, sizeof(*art));
    art->paletteCount = FF_SPIN_PALETTE_LOGO_SLOTS;
    for (uint32_t i = 0; colors && i < colorCount && i + 1 < FF_SPIN_PALETTE_LOGO_SLOTS; ++i) {
        ffStrbufSet(&art->palette[i + 1], &colors[i]);
    }

    uint32_t width, height;
    artWalk(art, text, false, &width, &height);
    if (width == 0 || height == 0) {
        return false;
    }
    art->glyphs = (uint32_t*) calloc((size_t) width * height, sizeof(uint32_t));
    art->colors = (uint8_t*) calloc((size_t) width * height, 1);
    if (!art->glyphs || !art->colors) {
        return false;
    }
    art->width = width;
    art->height = height;
    artWalk(art, text, true, &width, &height);
    return artCrop(art);
}

void ffSpinArtDestroy(FFSpinArt* art) {
    free(art->glyphs);
    free(art->colors);
    for (uint32_t i = 0; i < FF_SPIN_PALETTE_MAX; ++i) {
        ffStrbufDestroy(&art->palette[i]);
    }
    memset(art, 0, sizeof(*art));
}

bool ffSpinFrameInit(FFSpinFrame* frame, uint32_t width, uint32_t height) {
    memset(frame, 0, sizeof(*frame));
    if (width == 0 || height == 0) {
        return false;
    }
    frame->glyphs = (uint32_t*) calloc((size_t) width * height, sizeof(uint32_t));
    frame->colors = (uint8_t*) calloc((size_t) width * height, 1);
    if (!frame->glyphs || !frame->colors) {
        ffSpinFrameDestroy(frame);
        return false;
    }
    frame->width = width;
    frame->height = height;
    return true;
}

void ffSpinFrameDestroy(FFSpinFrame* frame) {
    free(frame->glyphs);
    free(frame->colors);
    memset(frame, 0, sizeof(*frame));
}

static void frameClear(FFSpinFrame* frame) {
    memset(frame->glyphs, 0, (size_t) frame->width * frame->height * sizeof(uint32_t));
    memset(frame->colors, 0, (size_t) frame->width * frame->height);
}

void ffSpinRotationMatrix(float yaw, float tilt, float out[3][3], float outInverse[3][3]) {
    float cy = cosf(yaw), sy = sinf(yaw);
    float cx = cosf(tilt), sx = sinf(tilt);
    // Rx(tilt) * Ry(yaw): spin around the logo's vertical axis, then lean it.
    const float m[3][3] = {
        { cy, 0, sy },
        { sx * sy, cx, -sx * cy },
        { -cx * sy, sx, cx * cy },
    };
    for (uint32_t i = 0; i < 3; ++i) {
        for (uint32_t j = 0; j < 3; ++j) {
            out[i][j] = m[i][j];
            outInverse[j][i] = m[i][j];
        }
    }
}

bool ffSpinRotationIsOrthonormal(const float m[3][3], float epsilon) {
    for (uint32_t i = 0; i < 3; ++i) {
        for (uint32_t j = 0; j < 3; ++j) {
            float dot = m[0][i] * m[0][j] + m[1][i] * m[1][j] + m[2][i] * m[2][j];
            if (fabsf(dot - (i == j ? 1.0f : 0.0f)) > epsilon) {
                return false;
            }
        }
    }
    float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
        - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
        + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    return fabsf(det - 1.0f) <= epsilon;
}

uint32_t ffSpinMirrorGlyph(uint32_t glyph) {
    static const uint32_t pairs[][2] = {
        { '/', '\\' }, { '(', ')' }, { '[', ']' }, { '{', '}' }, { '<', '>' },
        { 'b', 'd' }, { 'p', 'q' },
        { 0x258C, 0x2590 }, { 0x2598, 0x259D }, { 0x2596, 0x2597 }, { 0x259B, 0x259C },
        { 0x2599, 0x259F }, { 0x259A, 0x259E }, { 0x25E2, 0x25E3 }, { 0x25E4, 0x25E5 },
        { 0x2571, 0x2572 }, { 0x250C, 0x2510 }, { 0x2514, 0x2518 }, { 0x251C, 0x2524 },
        { 0x256D, 0x256E }, { 0x2570, 0x256F }, { 0x2554, 0x2557 }, { 0x255A, 0x255D },
    };
    for (uint32_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        if (glyph == pairs[i][0]) return pairs[i][1];
        if (glyph == pairs[i][1]) return pairs[i][0];
    }
    if (glyph >= 0x2800 && glyph <= 0x28FF) {
        // Braille: swap the left dot column (1,2,3,7) with the right one (4,5,6,8).
        uint32_t dots = glyph - 0x2800;
        return 0x2800 + (((dots & 0x07) << 3) | ((dots & 0x38) >> 3) | ((dots & 0x40) << 1) | ((dots & 0x80) >> 1));
    }
    return glyph;
}

void ffSpinFrameSize(const FFSpinArt* art, float tilt, uint32_t* width, uint32_t* height) {
    SpinGeometry g = spinGeometry(art);
    float maxX = (float) art->width * 0.5f, maxY = (float) art->height * 0.5f;
    for (uint32_t step = 0; step < 96; ++step) {
        float rot[3][3], inv[3][3];
        ffSpinRotationMatrix((float) step * 2.0f * (float) M_PI / 96.0f, tilt, rot, inv);
        for (uint32_t corner = 0; corner < 8; ++corner) {
            float p[3] = { corner & 1 ? g.hx : -g.hx, corner & 2 ? g.hy : -g.hy, corner & 4 ? g.hz : -g.hz };
            float c[3];
            for (uint32_t i = 0; i < 3; ++i) {
                c[i] = rot[i][0] * p[0] + rot[i][1] * p[1] + rot[i][2] * p[2];
            }
            float scale = g.focal / (g.camera - c[2]);
            maxX = fmaxf(maxX, fabsf(c[0]) * scale / FF_SPIN_CELL_ASPECT);
            maxY = fmaxf(maxY, fabsf(c[1]) * scale);
        }
    }
    // A cell is drawn when the ray through its center hits, and the outermost
    // cell centers sit half a cell inside the frame edge.
    float marginX = ceilf(maxX - (float) art->width * 0.5f - 0.5f);
    float marginY = ceilf(maxY - (float) art->height * 0.5f - 0.5f);
    *width = art->width + 2 * (marginX > 0 ? (uint32_t) marginX : 0);
    *height = art->height + 2 * (marginY > 0 ? (uint32_t) marginY : 0);
}

void ffSpinFramePutArt(FFSpinFrame* frame, const FFSpinArt* art) {
    frameClear(frame);
    if (frame->width < art->width || frame->height < art->height) {
        return;
    }
    uint32_t left = (frame->width - art->width) / 2;
    uint32_t top = (frame->height - art->height) / 2;
    for (uint32_t y = 0; y < art->height; ++y) {
        size_t to = (size_t) (top + y) * frame->width + left;
        size_t from = (size_t) y * art->width;
        memcpy(frame->glyphs + to, art->glyphs + from, art->width * sizeof(uint32_t));
        memcpy(frame->colors + to, art->colors + from, art->width);
    }
}

typedef enum SpinHit {
    SPIN_HIT_NONE,
    SPIN_HIT_FRONT,
    SPIN_HIT_BACK,
    SPIN_HIT_WALL,
} SpinHit;

// Casts one ray (object space) through the extruded art. On a hit, `cell` is
// the art cell and `normal` the object-space normal of the surface.
static SpinHit castRay(const FFSpinArt* art, const SpinGeometry* g, const float o[3], const float d[3], uint32_t* cell, float normal[3]) {
    const float lo[3] = { -g->hx, -g->hy, -g->hz };
    const float hi[3] = { g->hx, g->hy, g->hz };
    float tNear = 0.0f, tFar = INFINITY;
    int axis = -1;
    for (int a = 0; a < 3; ++a) {
        if (fabsf(d[a]) < 1e-12f) {
            if (o[a] < lo[a] || o[a] > hi[a]) return SPIN_HIT_NONE;
            continue;
        }
        float t1 = (lo[a] - o[a]) / d[a];
        float t2 = (hi[a] - o[a]) / d[a];
        if (t1 > t2) {
            float t = t1;
            t1 = t2;
            t2 = t;
        }
        if (t1 > tNear) {
            tNear = t1;
            axis = a;
        }
        if (t2 < tFar) tFar = t2;
        if (tNear > tFar) return SPIN_HIT_NONE;
    }
    if (axis < 0) return SPIN_HIT_NONE;

    // Grid coordinates: x in columns from the left, y in rows from the top.
    const int32_t w = (int32_t) art->width, h = (int32_t) art->height;
    float gx = (o[0] + d[0] * tNear + g->hx) / FF_SPIN_CELL_ASPECT;
    float gy = g->hy - (o[1] + d[1] * tNear);
    float dgx = d[0] / FF_SPIN_CELL_ASPECT;
    float dgy = -d[1];
    int32_t cx = (int32_t) floorf(gx), cy = (int32_t) floorf(gy);
    if (cx < 0) cx = 0;
    if (cx >= w) cx = w - 1;
    if (cy < 0) cy = 0;
    if (cy >= h) cy = h - 1;

    normal[0] = normal[1] = normal[2] = 0.0f;
    if (art->glyphs[(size_t) cy * art->width + (uint32_t) cx]) {
        *cell = (uint32_t) cy * art->width + (uint32_t) cx;
        normal[axis] = d[axis] > 0 ? -1.0f : 1.0f;
        if (axis == 2) return d[2] < 0 ? SPIN_HIT_FRONT : SPIN_HIT_BACK;
        return SPIN_HIT_WALL;
    }

    // Walk the grid cell by cell (Amanatides-Woo) until the ray leaves the slab.
    int32_t stepX = dgx > 0 ? 1 : -1, stepY = dgy > 0 ? 1 : -1;
    float tMaxX = dgx != 0 ? tNear + ((float) (dgx > 0 ? cx + 1 : cx) - gx) / dgx : INFINITY;
    float tMaxY = dgy != 0 ? tNear + ((float) (dgy > 0 ? cy + 1 : cy) - gy) / dgy : INFINITY;
    float tDeltaX = dgx != 0 ? fabsf(1.0f / dgx) : INFINITY;
    float tDeltaY = dgy != 0 ? fabsf(1.0f / dgy) : INFINITY;
    while (true) {
        bool alongX = tMaxX < tMaxY;
        if ((alongX ? tMaxX : tMaxY) > tFar) return SPIN_HIT_NONE;
        if (alongX) {
            cx += stepX;
            tMaxX += tDeltaX;
            if (cx < 0 || cx >= w) return SPIN_HIT_NONE;
        } else {
            cy += stepY;
            tMaxY += tDeltaY;
            if (cy < 0 || cy >= h) return SPIN_HIT_NONE;
        }
        if (art->glyphs[(size_t) cy * art->width + (uint32_t) cx]) {
            *cell = (uint32_t) cy * art->width + (uint32_t) cx;
            if (alongX) {
                normal[0] = (float) -stepX;
            } else {
                normal[1] = (float) stepY; // grid rows grow downward
            }
            return SPIN_HIT_WALL;
        }
    }
}

void ffSpinRenderArt(const FFSpinArt* art, float yaw, float tilt, FFSpinFrame* frame) {
    frameClear(frame);
    SpinGeometry g = spinGeometry(art);
    float rot[3][3], inv[3][3];
    ffSpinRotationMatrix(yaw, tilt, rot, inv);
    const float origin[3] = { inv[0][2] * g.camera, inv[1][2] * g.camera, inv[2][2] * g.camera };
    const uint32_t rampLast = (uint32_t) (sizeof(FFSpinWallRamp) / sizeof(FFSpinWallRamp[0])) - 1;

    for (uint32_t y = 0; y < frame->height; ++y) {
        float v = (float) frame->height * 0.5f - ((float) y + 0.5f);
        for (uint32_t x = 0; x < frame->width; ++x) {
            float u = ((float) x + 0.5f - (float) frame->width * 0.5f) * FF_SPIN_CELL_ASPECT;
            float d[3];
            for (uint32_t i = 0; i < 3; ++i) {
                d[i] = inv[i][0] * u + inv[i][1] * v - inv[i][2] * g.focal;
            }
            uint32_t cell = 0;
            float n[3];
            SpinHit hit = castRay(art, &g, origin, d, &cell, n);
            if (hit == SPIN_HIT_NONE) continue;

            size_t index = (size_t) y * frame->width + x;
            frame->colors[index] = art->colors[cell];
            if (hit == SPIN_HIT_FRONT) {
                frame->glyphs[index] = art->glyphs[cell];
            } else if (hit == SPIN_HIT_BACK) {
                frame->glyphs[index] = ffSpinMirrorGlyph(art->glyphs[cell]);
            } else {
                float diffuse = 0.0f;
                for (uint32_t i = 0; i < 3; ++i) {
                    diffuse += (rot[i][0] * n[0] + rot[i][1] * n[1] + rot[i][2] * n[2]) * FFSpinLight[i];
                }
                float luminance = 0.15f + 0.85f * fmaxf(diffuse, 0.0f);
                frame->glyphs[index] = FFSpinWallRamp[(uint32_t) (luminance * (float) rampLast + 0.5f)];
            }
        }
    }
}

static void appendSgr(FFstrbuf* out, const FFSpinArt* art, uint8_t color, bool bold) {
    ffStrbufAppendS(out, bold ? "\e[0;1" : "\e[0");
    if (color < art->paletteCount && art->palette[color].length > 0) {
        ffStrbufAppendC(out, ';');
        ffStrbufAppend(out, &art->palette[color]);
    }
    ffStrbufAppendC(out, 'm');
}

void ffSpinAppendCells(FFstrbuf* out, const FFSpinFrame* frame, const FFSpinArt* art, uint32_t index, uint32_t count, bool color, bool bold) {
    int32_t active = -1;
    for (uint32_t i = index; i < index + count; ++i) {
        uint32_t glyph = frame->glyphs[i];
        if (glyph == 0) {
            ffStrbufAppendC(out, ' ');
            continue;
        }
        if (color && (int32_t) frame->colors[i] != active) {
            active = frame->colors[i];
            appendSgr(out, art, frame->colors[i], bold);
        }
        ffStrbufAppendUtf32CodePoint(out, glyph);
    }
    if (active >= 0) {
        ffStrbufAppendS(out, "\e[0m");
    }
}

void ffSpinFrameToAnsi(const FFSpinFrame* frame, const FFSpinArt* art, bool color, bool bold, bool trimRight, FFstrbuf* out) {
    for (uint32_t y = 0; y < frame->height; ++y) {
        uint32_t length = frame->width;
        if (trimRight) {
            // Empty cells at the end of a line are not part of the art. Printing
            // them would pad every line to the frame width and make the output
            // differ from the other text logo types.
            while (length > 0 && frame->glyphs[(size_t) y * frame->width + length - 1] == 0) {
                --length;
            }
        }
        ffSpinAppendCells(out, frame, art, y * frame->width, length, color, bold);
        ffStrbufAppendC(out, '\n');
    }
}
