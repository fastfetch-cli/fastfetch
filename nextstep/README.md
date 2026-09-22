# fastfetch for NeXTSTEP 3.3 / OPENSTEP 4.2

This directory is a small, standalone, ANSI C (C89) reimplementation of a
subset of fastfetch, written specifically for NeXTSTEP 3.3 and OPENSTEP 4.2.
It is **not** built by the main project's CMake build and shares no code
with `src/` on purpose:

- The main fastfetch requires a C17/C23 compiler and CMake >= 3.21. Neither
  exists on these systems: NeXTSTEP 3.3 ships GCC 2.5.8, OPENSTEP 4.2 ships
  GCC 2.7.2.1, and there is no CMake port for either.
- Most of the main project's detection backends target Linux/BSD/macOS APIs
  (`/proc`, `/sys`, modern `sysctl`, pthreads) that don't exist here.
- NeXTSTEP has no pthreads (it has Mach cthreads instead), so this port is
  single-threaded and does detection sequentially.

## Building

On the target machine (or under emulation - [Previous](https://previous.alt.org/)
for m68k NeXT hardware, QEMU for the x86 build):

```sh
cd nextstep
make
./fastfetch
```

There is no cross-compilation path provided. A maintained cross-toolchain
for `m68k-next-nextstep`/`i386-next-nextstep` targeting modern host machines
doesn't really exist anymore; building natively with the OS's own bundled
`cc` is the reliable option and is how all NeXTSTEP/OPENSTEP software was
built historically.

## What's covered

Title, OS, Kernel, Host, CPU, Memory, Uptime, Shell, Terminal, and a
cosmetic ANSI Colors swatch. Package managers, GPU, and display/resolution
detection are intentionally not implemented (resolution in particular would
require linking against the Display PostScript / AppKit libraries and
opening a window-server connection, which is a much bigger undertaking than
everything else here combined).

## Things that were written from documented Mach/4.3BSD behavior but not
## verified against a real system (I don't have one to test against)

Each of these degrades to printing "unknown" on failure rather than being
load-bearing, so a failure in one shouldn't stop the rest from working. If
one of them doesn't compile at all, the fix is almost always to just
comment out that one `print*()` call in `main()`.

- **Host/CPU model naming**: `host_info(HOST_BASIC_INFO)` gives a raw
  `cpu_subtype` integer. The exact NeXT-specific subtype values that map to
  "Cube" vs "Slab" vs "Turbo" vs "Color Slab" are not decoded - the program
  just prints the raw number (`Host: ... cpu_subtype=N`). If you can run
  this on known hardware, note down what N is for each machine and the
  mapping can be filled in.
- **Uptime**: uses the classic 4.3BSD `nlist(3)` + `/dev/kmem` technique
  (look up the kernel's `_boottime` symbol, read it directly out of kernel
  memory) - this is what `uptime(1)`/`w(1)` did on systems of this vintage.
  It needs read access to `/dev/kmem` (usually via the `kmem` group), and
  assumes the kernel image is at `/mach` (NeXTSTEP's kernel path) with a
  leading-underscore symbol name, both of which the code falls back through
  a couple of alternatives for but can't guarantee.
- **NeXTSTEP vs OPENSTEP branding**: decided purely from the major version
  number reported by `uname()` (<=3 -> "NeXTSTEP", >=4 -> "OPENSTEP"), which
  matches the well-documented 3.x -> 4.x rebranding, without relying on any
  version-marker file whose exact path/format I couldn't confirm.

## If linking fails

Mach IPC calls (`host_info`, `host_statistics`, `mach_host_self`) are part
of the base system library on every Mach-derived OS (this is the same API
family Darwin/macOS still uses today - see `src/detection/memory/memory_apple.c`
and `src/detection/cpu/cpu_apple.c` in the main project for the modern
equivalent). They should link with no extra flags. If the linker reports
undefined Mach symbols, try adding `-lsys_s` to `LIBS` in the Makefile.

## If colors look like garbage escape codes

NeXTSTEP/OPENSTEP's bundled Terminal.app has uncertain ANSI SGR color
support in this era. If `printColors()` doesn't render as colored blocks,
just remove the call from `main()`.
