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

Confirmed building and running on real OPENSTEP 4.2 (Intel) hardware. The
Makefile builds with `cc -posix`, which is required, not optional: this
system gates POSIX declarations (`isatty()`, `getuid()`, `open()`/`lseek()`/
`read()`/`close()`) behind `_POSIX_SOURCE`, and only `-posix` makes them
actually link too.

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

## Confirmed on real hardware, but with known limitations

This has been built and run on real OPENSTEP 4.2 (Intel) hardware. A few
things that were originally guesses from documented Mach/4.3BSD behavior
turned out to need adjustment once tested for real; a couple of remaining
limitations are permanent rather than "unverified":

- **NeXTSTEP vs OPENSTEP branding**: there's no `uname()` on this system (see
  "If linking fails" below), so there's no runtime way to read an exact
  release number. `printOS()` prints the generic `NeXTSTEP/OPENSTEP` plus
  the architecture (`m68k`/`i386`) from `host_info(HOST_BASIC_INFO)` instead
  of picking a specific branding/version.
- **Memory**: this Mach vintage predates Darwin's `HOST_VM_INFO` flavor of
  `host_statistics()` - confirmed against a period `host_info.h` that only
  defines `HOST_BASIC_INFO`/`HOST_PROCESSOR_SLOTS`/`HOST_SCHED_INFO`/
  `HOST_LOAD_INFO`, no VM flavor at all. VM stats instead come from the
  older, dedicated `vm_statistics(task, &stats)` RPC, whose struct carries
  `pagesize` directly (no separate `host_page_size()` call needed).
- **`struct nlist`**: this system nests the symbol name behind a union
  (`n_un.n_name`) rather than exposing it directly, the standard
  post-4.3BSD-Reno layout.
- **Host/CPU model naming**: `host_info(HOST_BASIC_INFO)` gives a raw
  `cpu_subtype` integer. The exact NeXT-specific subtype values that map to
  "Cube" vs "Slab" vs "Turbo" vs "Color Slab" are still not decoded - the
  program just prints the raw number (`Host: ... cpu_subtype=N`). If you can
  run this on known hardware, note down what N is for each machine and the
  mapping can be filled in.
- **Uptime**: uses the classic 4.3BSD `nlist(3)` + `/dev/kmem` technique
  (look up the kernel's `_boottime` symbol, read it directly out of kernel
  memory) - this is what `uptime(1)`/`w(1)` did on systems of this vintage.
  It needs read access to `/dev/kmem` (usually via the `kmem` group), and
  assumes the kernel image is at `/mach` (NeXTSTEP's kernel path) with a
  leading-underscore symbol name, both of which the code falls back through
  a couple of alternatives for but can't guarantee. Still unverified.

Each of these degrades to printing "unknown" on failure rather than being
load-bearing, so a failure in one shouldn't stop the rest from working. If
one of them doesn't compile at all, the fix is almost always to just
comment out that one `print*()` call in `main()`.

## If linking fails

Mach IPC calls (`host_info`, `mach_host_self`, `mach_task_self`,
`vm_statistics`) are part of the base system library on every Mach-derived
OS (this is the same API family Darwin/macOS still uses today - see
`src/detection/memory/memory_apple.c` and `src/detection/cpu/cpu_apple.c`
in the main project for the modern equivalent). They link with no extra
flags. If the linker reports undefined Mach symbols, try adding `-lsys_s`
to `LIBS` in the Makefile.

`uname()` is a confirmed exception: this system's headers declare it, but on
real OPENSTEP 4.2 (Intel) hardware `_uname` is undefined at link time even
when building with `cc -posix` (every other POSIX call in this file resolves
fine with that flag). Since there's no known workaround, this port doesn't
call `uname()` at all - see the note above `printOS()` in `fastfetch.c`.

## If colors look like garbage escape codes

NeXTSTEP/OPENSTEP's bundled Terminal.app has uncertain ANSI SGR color
support in this era. If `printColors()` doesn't render as colored blocks,
just remove the call from `main()`.
