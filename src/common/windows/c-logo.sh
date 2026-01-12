#!/bin/sh
# Convert logo.svg to logo.ico
rsvg-convert -w 16 -h 16 logo.svg > logo16.png
rsvg-convert -w 32 -h 32 logo.svg > logo32.png
rsvg-convert -w 48 -h 48 logo.svg > logo48.png
rsvg-convert -w 64 -h 64 logo.svg > logo64.png
rsvg-convert -w 128 -h 128 logo.svg > logo128.png
rsvg-convert -w 256 -h 256 logo.svg > logo256.png
convert logo16.png logo32.png logo48.png logo64.png logo128.png logo256.png logo.ico
rm logo16.png logo32.png logo48.png logo64.png logo128.png logo256.png
