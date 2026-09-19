# libsixel (encoder subset, vendored)

Encoder-only subset of [libsixel](https://github.com/saitoha/libsixel) `1.8.7-r2`,
used by `src/logo/image/` to render the `sixel` logo type **on Windows and macOS**.

## Why this is vendored

Neither MSYS2 nor vcpkg packages libsixel, so on Windows there is nothing to
`dlopen()` or to link against. The encoder is small and has no external
dependencies, so the necessary files are embedded instead. The intent is to upstream
a libsixel package to MSYS2 later and switch to it; until then this directory is the
source of truth.

macOS *does* have a package — Homebrew's `libsixel` is this very version, `1.8.7-r2`
(`brew info libsixel`) — but it is deliberately not used. It drags in `jpeg-turbo` /
`libpng` for the decoder side we never call, and linking it would mean a different
encoder on each platform. Since Windows needs an embedded copy regardless, one shared
copy is simpler than two.

## Why only Windows and macOS

Windows loses ImageMagick entirely — its image backend moves to WIC, which decodes
and scales but cannot *encode* sixel — so it needs an encoder of its own.

macOS does not get ImageMagick either: `ENABLE_IMAGEMAGICK6` / `ENABLE_IMAGEMAGICK7`
are gated on `LINUX OR FreeBSD OR OpenBSD OR NetBSD OR ANDROID OR SunOS OR Haiku OR
GNU`, with no `APPLE`, and its image backend is ImageIO. So it shares the same
requirement.

On every other platform ImageMagick stays, and it already produces sixel through
its own SIXEL coder. Adding libsixel there would mean a second encoder for a
capability that already works: one more dependency for zero gain. So `ENABLE_SIXEL`
is gated on `WIN32 OR APPLE`, and these sources are **not compiled at all** on
Linux / the BSDs.

Consequence: sixel bytes differ between Windows / macOS (libsixel) and the rest
(ImageMagick). That is deliberate and accepted — see
`doc/windows-image-backend.md` decision 9.

## Transparency

The subset is used for its keycolor support. `sixel_dither_set_transparent()` marks a
palette index as the transparent one; `output_rgb_palette_definition()` then leaves that
entry out of the palette definition, and `sixel_encode_body()` never puts a keycolor
pixel into the node map, so nothing is drawn there and the terminal background shows
through. `src/logo/image/sixel.c` drives this for sources that contain fully transparent
pixels — it maps those pixels onto a dedicated index one past the quantized palette,
because keying a *content* colour would also drop every opaque pixel that shares it.

Two things have to line up for the background to actually be transparent: the caller has to
mark a keycolor, so the keyed pixels are never painted (above), **and** the encoder has to ask
for a transparent background, so the terminal leaves those unpainted pixels alone instead of
filling them. The second half is the DCS `P2` parameter — see
[Local modifications](#local-modifications) item 3.

Note what this is and is not: it is a **palette-index** keycolor, not alpha support. A
partially transparent pixel has to be drawn in its unblended colour, since a sixel has
no way to blend with whatever the terminal has behind it.

⚠️ Do not vendor a *newer* libsixel in the belief that this version cannot do
transparency. It can, and it was measured: the official `1.8.7-r2` omits the keycolor
entry and leaves the keyed pixels unpainted, with the remaining pixels correctly placed.
When a transparent background is not showing up, check the caller first — historically
the caller simply never called `sixel_dither_set_transparent()`, and an image logo only
renders correctly once it does. If the caller *is* setting a keycolor and the background is
still solid, check `P2` instead: the keyed pixels are being left unpainted correctly, but the
terminal is filling them.

## What was kept

Starting from `sixel_dither_new` / `sixel_dither_initialize` / `sixel_output_new` /
`sixel_encode` and following the dependency closure, only these files are reachable:

| File | Provides |
|---|---|
| `tosixel.c` | `sixel_encode`, `sixel_putc`, `sixel_node_*`, DCS envelope handling |
| `dither.c` / `dither.h` | `sixel_dither_new`, `sixel_dither_initialize`, `sixel_dither_unref` |
| `output.c` / `output.h` | `sixel_output_new`, `sixel_output_unref` |
| `quant.c` / `quant.h` | Median-cut palette construction |
| `pixelformat.c` | `sixel_helper_compute_depth`, `sixel_helper_normalize_pixelformat` |
| `allocator.c` / `allocator.h` | `sixel_allocator_*` |
| `status.c` / `status.h` | `sixel_helper_set_additional_message` |
| `malloc_stub.h` | Included by `allocator.c`; compiles to nothing when `HAVE_MALLOC` |
| `sixel.h` | Public API (upstream `include/sixel.h`) |

## What was removed

Everything else in the upstream tree, in particular:

- **Decoding**: `decoder.c`, `fromsixel.c`, `frompnm.c`, `fromgif.c`, `frame.c`,
  `loader.c`, `stb_image.h`, `stb_image_write.*`. Image decoding is done by the
  platform backend (WIC on Windows, ImageIO on macOS, ImageMagick elsewhere), so
  libsixel only ever sees an RGBA8 buffer here.
- **The `sixel_encoder_*` / `sixel_decoder_*` high-level API**: `encoder.c`,
  `writer.c`, `tty.c`, `scale.c`. These pull in file I/O, terminal probing and the
  loader registry. We call the dither/output/encode trio directly.
- **`rgblookup.h` / `rgblookup.gperf`**: a gperf-generated color-name table used
  only by `encoder.c` for `--builtin-palette` parsing.
- **`converters/`** (`img2sixel`, `sixel2png`), **`python/`**, **`tools/`**,
  **`tests/`**, **`images/`** (216 test images), and the packaging templates
  (`package.json.in.in`, `libsixel.pc.in`).
- **The autotools machinery**: `configure`, `Makefile.in`, `aclocal.m4`, `m4/`,
  `ltmain.sh`, `config.h.in`, … See item 2 under
  [Local modifications](#local-modifications).
- **Six of the nine `LICENSE.*` files**: `LICENSE.images` / `.mesa` / `.pngsuite` /
  `.sdump` / `.stb` cover only material that was dropped (test images, the OpenGL
  example, the `sdump` tool, stb). The three that remain are the complete set
  required by the files above — verified against each file's own header, see
  [License](#license).

Result: **20 files / 256 KB**, down from 319 files / 11.3 MB. Note that 8.0 MB of
that 11.3 MB was `images/` fixtures, so the meaningful comparison is code:
`src/` + `include/` went from 50 files / 1.17 MB to 20 files / 256 KB.

## Local modifications

Three deliberate divergences from upstream. Re-apply all three when re-syncing.

1. **`sixel.h` — `SIXELAPI` is empty.** Upstream defines it as
   `__declspec(dllexport)` on Windows. This subset is compiled into `libfastfetch`
   statically rather than into a standalone DLL, so the attribute would (a) add
   every libsixel symbol to fastfetch's export table and (b) make `sixel.h`'s
   declarations disagree with the ones in `dither.h` / `output.h` / `quant.h` /
   `allocator.h`, which do not carry it (clang warns:
   `-Wdll-attribute-on-redeclaration`).

2. **`config.h` is hand-written.** Upstream generates it with `configure`. Only
   these vendored files read it, and they reference a small fixed set of macros,
   so the values are hard-coded. Two things to watch:
   - `HAVE_TESTS` must stay **undefined**, not `0`: `sixel.h` gates its test-only
     declarations with `#ifdef HAVE_TESTS`, so `#define HAVE_TESTS 0` would still
     pull in declarations for functions we do not compile.
   - `HAVE_MEMORY_H` / `HAVE_STRING_H` must be `1`. fastfetch builds with
     `-Werror=implicit-function-declaration`, so a missing `<memory.h>` /
     `<string.h>` include in `pixelformat.c` / `status.c` is a hard error.

3. **`tosixel.c` — the DCS `P2` parameter is initialised to 1.** `sixel_encode_header()`
   declares `int p[3] = {0, 0, 0}`, and the trailing-zero trimming right below it then drops
   all three, so the envelope is a bare `ESC P q`. That means `P2 = 0`, which tells the
   terminal to fill every unset pixel with **colour-table entry 0** — and that entry is
   defined by the image's own palette, so a keycolor image comes out with a solid rectangle
   behind it rather than transparency. Setting `p[1] = 1` leaves unset pixels untouched.

   The trimming still drops the trailing aspect-ratio parameter, so the envelope is
   `ESC P 0;1 q`. `P3` defaults to 0 and the two spellings are equivalent — verified by
   decoding both with libsixel's own decoder and with ImageMagick's. ImageMagick's SIXEL
   coder and chafa both write `0;1;0` explicitly.

   This only shows up on a terminal that implements the VT340 background rule. Windows
   Terminal does (`_backgroundFillRequired = (_conformanceLevel == 1 || !transparent)` in
   `src/terminal/adapter/SixelParser.cpp`), which is where it was reproduced; WezTerm does
   not fill, so the identical bytes look correct there. It is also why Linux never showed
   the problem: that path goes through ImageMagick, which already sends `P2 = 1`.

The vendored sources are also exempted from `-Wconversion` in `CMakeLists.txt`:
upstream has ~160 implicit int→`unsigned char` narrowing warnings. Everything else
(`-Wall -Wextra` and fastfetch's `-Werror=` set) applies unchanged, and the subset
compiles clean.

## Re-syncing with upstream

```sh
VER=1.8.7-r2
git clone --depth 1 --branch "$VER" https://github.com/saitoha/libsixel /tmp/libsixel

# 1. copy the closure (paths change from src/foo.c to foo.c, include/sixel.h to sixel.h)
for f in tosixel.c dither.c dither.h output.c output.h quant.c quant.h \
         pixelformat.c allocator.c allocator.h status.c status.h malloc_stub.h; do
    cp "/tmp/libsixel/src/$f" .
done
cp /tmp/libsixel/include/sixel.h .
cp /tmp/libsixel/LICENSE /tmp/libsixel/LICENSE.sixel /tmp/libsixel/LICENSE.pnmcolormap .

# 2. re-apply the three local modifications listed above
# 3. verify
```

Verification (must be 0 diagnostics, and the encoder must emit a DCS-wrapped stream).
Both platforms compile this subset, so run it on both:

```sh
# Windows: CC=/c/msys64/clang64/bin/cc.exe
# macOS:   CC=cc   (verified with Apple clang 21: 0 diagnostics)
CC=cc
FLAGS="-I. -Wall -Wextra -Wconversion -Wno-conversion -Werror=uninitialized \
 -Werror=return-type -Werror=vla -Werror=incompatible-pointer-types \
 -Werror=implicit-function-declaration -Werror=int-conversion -std=gnu23"
for f in tosixel dither output quant allocator pixelformat status; do
    $CC $FLAGS -c $f.c -o /dev/null || echo "FAILED: $f"
done
```

If upstream adds a call from the closure to a new file, the build fails at link
time with an undefined `sixel_*` symbol — add that file and repeat.

## License

MIT-compatible, but three separate notices apply and all three files must be kept.
Mapping verified against each source file's own copyright header:

| File | Applies to |
|---|---|
| `LICENSE` | MIT, Copyright (c) 2014-2016 Hayaki Saito — `dither.c`, `output.c`, `pixelformat.c`, `allocator.c`, `status.c` (their headers say 2014-2018/2019; same MIT terms) |
| `LICENSE.sixel` | `tosixel.c` — derived from kmiya's original `sixel` (2014-3-2), permissive, re-licensed MIT by Hayaki Saito |
| `LICENSE.pnmcolormap` | `quant.c` — derived from `ppmquant` by Jef Poskanzer / Bryan Henderson |

Upstream also ships `LICENSE.images`, `.mesa`, `.pngsuite`, `.sdump` and `.stb`;
none of them cover a file in this subset, so they were not carried over. If a
re-sync ever pulls in a file from the decoder side (`fromgif.c` → `.stb`,
`fromsixel.c` → `.sixel`), re-check this table.
