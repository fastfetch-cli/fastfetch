# libsixel (encoder subset, vendored)

Encoder-only subset of [libsixel](https://github.com/saitoha/libsixel) `1.8.7-r2`,
used by `src/logo/image/` to render the `sixel` logo type **on Windows only**.

## Why this is vendored

Neither MSYS2 nor vcpkg packages libsixel, so there is nothing to `dlopen()` or to
link against. The encoder is small and has no external dependencies, so the
necessary files are embedded instead. The intent is to upstream a libsixel package
to MSYS2 later and switch to it; until then this directory is the source of truth.

## Why Windows only

Windows loses ImageMagick entirely — its image backend moves to WIC, which decodes
and scales but cannot *encode* sixel — so it needs an encoder of its own.

On every other platform ImageMagick stays, and it already produces sixel through
its own SIXEL coder. Adding libsixel there would mean a second encoder for a
capability that already works: one more dependency for zero gain. So `ENABLE_SIXEL`
is gated on `WIN32`, and these sources are **not compiled at all** on Linux / macOS /
BSD.

Consequence: sixel bytes differ between Windows (libsixel) and the rest
(ImageMagick). That is deliberate and accepted — see
`doc/windows-image-backend.md` decision 9.

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
  platform backend (WIC on Windows, ImageMagick elsewhere), so libsixel only ever
  sees an RGBA8 buffer here.
- **The `sixel_encoder_*` / `sixel_decoder_*` high-level API**: `encoder.c`,
  `writer.c`, `tty.c`, `scale.c`. These pull in file I/O, terminal probing and the
  loader registry. We call the dither/output/encode trio directly.
- **`rgblookup.h` / `rgblookup.gperf`**: a gperf-generated color-name table used
  only by `encoder.c` for `--builtin-palette` parsing.
- **`converters/`** (`img2sixel`, `sixel2png`), **`python/`**, **`tools/`**,
  **`tests/`**, **`images/`** (216 test images), and the packaging templates
  (`package.json.in.in`, `libsixel.pc.in`).
- **The autotools machinery**: `configure`, `Makefile.in`, `aclocal.m4`, `m4/`,
  `ltmain.sh`, `config.h.in`, … See `config.h` below.
- **Six of the nine `LICENSE.*` files**: `LICENSE.images` / `.mesa` / `.pngsuite` /
  `.sdump` / `.stb` cover only material that was dropped (test images, the OpenGL
  example, the `sdump` tool, stb). The three that remain are the complete set
  required by the files above — verified against each file's own header, see
  [License](#license).

Result: **20 files / 255 KB**, down from 319 files / 11.3 MB. Note that 8.0 MB of
that 11.3 MB was `images/` fixtures, so the meaningful comparison is code:
`src/` + `include/` went from 50 files / 1.17 MB to 20 files / 255 KB.

## Local modifications

Two deliberate divergences from upstream. Re-apply both when re-syncing.

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

# 2. re-apply the two local modifications listed above
# 3. verify
```

Verification (must be 0 diagnostics, and the encoder must emit a DCS-wrapped stream):

```sh
CC=/c/msys64/clang64/bin/cc.exe
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
