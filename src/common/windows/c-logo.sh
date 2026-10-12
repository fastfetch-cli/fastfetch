#!/bin/sh
# Convert logo.svg to logo.ico.
#
# Every frame is stored as PNG. ImageMagick's ICO writer only uses PNG for frames that are
# 256x256 or larger and falls back to an uncompressed DIB for everything smaller, which
# inflates the icon from ~19 KB to ~108 KB (and to ~370 KB on ImageMagick 7, which also
# stores 256x256 as a DIB). Packing the frames here keeps the icon small and predictable.
set -e

cd "$(dirname "$0")"

for size in 16 32 48 64 128 256; do
    rsvg-convert -w "$size" -h "$size" ../../logo/logo.svg > "logo$size.png"
done

python3 - <<'PY'
import struct

sizes = [16, 32, 48, 64, 128, 256]
frames = [open(f"logo{s}.png", "rb").read() for s in sizes]

header = struct.pack("<HHH", 0, 1, len(sizes))
offset = 6 + 16 * len(sizes)
entries = b""
for size, frame in zip(sizes, frames):
    dim = 0 if size == 256 else size  # 0 means 256 in the ICO directory
    entries += struct.pack("<BBBBHHII", dim, dim, 0, 0, 1, 32, len(frame), offset)
    offset += len(frame)

with open("logo.ico", "wb") as f:
    f.write(header + entries + b"".join(frames))
PY

rm logo16.png logo32.png logo48.png logo64.png logo128.png logo256.png
