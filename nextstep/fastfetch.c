/*
 * fastfetch - minimal NeXTSTEP 3.3 / OPENSTEP 4.2 port
 *
 * This is a small, self-contained, ANSI C (C89) reimplementation covering a
 * subset of the modules from the main fastfetch project. It is built and
 * maintained separately from src/ because:
 *
 *   - The main project requires a C17/C23 compiler; NeXTSTEP 3.3 ships GCC
 *     2.5.8 and OPENSTEP 4.2 ships GCC 2.7.2.1, neither of which understand
 *     modern C.
 *   - The main project's build (CMake >= 3.21) cannot run on these systems
 *     at all.
 *   - Most of the main project's detection code targets Linux/BSD/macOS
 *     APIs (/proc, /sys, modern sysctl, pthreads) that don't exist here.
 *
 * Build with the compiler NeXTSTEP/OPENSTEP already ships (see Makefile):
 *     cd nextstep && make
 *
 * Known-uncertain areas are marked "NOTE:" below - they are written from
 * documented Mach/4.3BSD behavior but have not been compiled or run on a
 * real NeXTSTEP/OPENSTEP system. If one of these fails to build, the
 * simplest fix is to comment out the offending block; each one degrades to
 * printing "unknown" rather than being load-bearing for the rest.
 */

/*
 * This system's headers only expose ANSI-C declarations by default; POSIX
 * declarations (isatty(), getuid(), open()/lseek()/read()/close(), ...)
 * are gated behind _POSIX_SOURCE and must be requested before any header
 * is included. The Makefile also builds with `cc -posix` so these actually
 * link (see Makefile) - except uname(), which this system declares but
 * doesn't provide a linkable implementation of even so; see the NOTE above
 * printOS() below for how that's worked around.
 */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <nlist.h>

#include <mach/mach.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <mach/vm_statistics.h>

/*
 * gethostname() is a BSD extension, not POSIX.1, so _POSIX_SOURCE above
 * hides its declaration even though the symbol is still in libc. Declare
 * it ourselves with the classic BSD (int namelen) signature this system
 * uses.
 */
extern int gethostname(char *name, int namelen);

#define FF_LINE_MAX 256

/* ---- small helpers -------------------------------------------------- */

static void ffTruncate(char *buf, size_t bufSize)
{
    buf[bufSize - 1] = '\0';
}

static int ffIsTty(void)
{
    return isatty(1);
}

/* ---- title: user@host + separator ------------------------------------ */

static void printTitle(void)
{
    char user[FF_LINE_MAX];
    char host[FF_LINE_MAX];
    char title[FF_LINE_MAX * 2];
    size_t len;
    size_t i;
    struct passwd *pw;

    pw = getpwuid(getuid());
    if (pw != NULL && pw->pw_name != NULL) {
        strncpy(user, pw->pw_name, sizeof(user) - 1);
        ffTruncate(user, sizeof(user));
    } else {
        char *envUser = getenv("USER");
        if (envUser == NULL) envUser = getenv("LOGNAME");
        strncpy(user, envUser != NULL ? envUser : "unknown", sizeof(user) - 1);
        ffTruncate(user, sizeof(user));
    }

    if (gethostname(host, (int) sizeof(host)) != 0) {
        strcpy(host, "unknown");
    }
    ffTruncate(host, sizeof(host));

    sprintf(title, "%s@%s", user, host);
    printf("%s\n", title);

    len = strlen(title);
    for (i = 0; i < len; ++i) putchar('-');
    putchar('\n');
}

/* ---- os / kernel: uname()'s prototype is declared by this system's     */
/* headers, but the symbol isn't in the linkable libposix.a - confirmed by */
/* a real build: `_uname` is the *only* undefined symbol at link time even */
/* with `cc -posix`, while every other call in this file (including the   */
/* other POSIX ones - isatty, getuid, open/lseek/read/close) resolves      */
/* fine. So there's no runtime way here to read an exact release number;  */
/* this reports architecture via the same Mach host_info() call used      */
/* elsewhere in this file instead. -------------------------------------- */

static void printOS(void)
{
    kern_return_t kr;
    struct host_basic_info info;
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
    const char *arch = "unknown architecture";

    kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t) &info, &count);
    if (kr == KERN_SUCCESS) {
#ifdef CPU_TYPE_MC680X0
        if (info.cpu_type == CPU_TYPE_MC680X0) arch = "m68k";
#endif
#ifdef CPU_TYPE_I386
        if (info.cpu_type == CPU_TYPE_I386) arch = "i386";
#endif
    }

    printf("OS: NeXTSTEP/OPENSTEP (%s)\n", arch);
}

/* ---- kernel ------------------------------------------------------------ */

static void printKernel(void)
{
    printf("Kernel: Mach\n");
}

/* ---- host: best-effort, exact NeXT hardware model (Cube/Slab/Turbo/   */
/* Color) is NOT decoded here - it depends on cpu_subtype values we      */
/* can't verify without a real system. The raw subtype is printed so it  */
/* can be mapped by hand later. ------------------------------------------ */

static void printHost(void)
{
    kern_return_t kr;
    struct host_basic_info info;
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
    const char *cpuTypeName = "Unknown";

    kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t) &info, &count);
    if (kr != KERN_SUCCESS) {
        printf("Host: NeXT-compatible system (host_info failed)\n");
        return;
    }

#ifdef CPU_TYPE_MC680X0
    if (info.cpu_type == CPU_TYPE_MC680X0) cpuTypeName = "Motorola 68k";
#endif
#ifdef CPU_TYPE_I386
    if (info.cpu_type == CPU_TYPE_I386) cpuTypeName = "Intel x86";
#endif

    printf("Host: NeXT-compatible system (%s, cpu_subtype=%ld)\n",
        cpuTypeName, (long) info.cpu_subtype);
}

/* ---- cpu ---------------------------------------------------------------- */

static void printCPU(void)
{
    kern_return_t kr;
    struct host_basic_info info;
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
    const char *name = "Unknown CPU";

    kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t) &info, &count);
    if (kr != KERN_SUCCESS) {
        printf("CPU: unknown\n");
        return;
    }

#ifdef CPU_TYPE_MC680X0
    if (info.cpu_type == CPU_TYPE_MC680X0) {
        name = "Motorola 68030/68040";
#ifdef CPU_SUBTYPE_MC68030_ONLY
        if (info.cpu_subtype == CPU_SUBTYPE_MC68030_ONLY) name = "Motorola 68030";
#endif
#ifdef CPU_SUBTYPE_MC68040
        if (info.cpu_subtype == CPU_SUBTYPE_MC68040) name = "Motorola 68040";
#endif
    }
#endif
#ifdef CPU_TYPE_I386
    if (info.cpu_type == CPU_TYPE_I386) name = "Intel x86 (i386-class)";
#endif

    printf("CPU: %s (%d)\n", name, (int) info.avail_cpus);
}

/* ---- memory: this Mach vintage predates Darwin's HOST_VM_INFO flavor of */
/* host_statistics() (confirmed against a period host_info.h: it only     */
/* defines HOST_BASIC_INFO/HOST_PROCESSOR_SLOTS/HOST_SCHED_INFO/           */
/* HOST_LOAD_INFO, no VM flavor at all). VM stats instead come from their  */
/* own dedicated RPC, vm_statistics(task, &stats), whose struct carries    */
/* pagesize directly - no separate host_page_size() call needed. */

static void printMemory(void)
{
    kern_return_t kr;
    struct host_basic_info info;
    mach_msg_type_number_t basicCount = HOST_BASIC_INFO_COUNT;
    vm_statistics_data_t vmstat;
    unsigned long totalBytes, freeBytes, usedBytes, pageSize;

    kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t) &info, &basicCount);
    if (kr != KERN_SUCCESS) {
        printf("Memory: unknown\n");
        return;
    }
    totalBytes = (unsigned long) info.memory_size;

    kr = vm_statistics(mach_task_self(), &vmstat);
    if (kr != KERN_SUCCESS) {
        printf("Memory: %luMiB total (used/free unknown)\n", totalBytes / 1024 / 1024);
        return;
    }

    pageSize = (unsigned long) vmstat.pagesize;
    if (pageSize == 0) pageSize = 4096; /* fallback: both m68k and i386 NeXT hardware use 4K pages */

    freeBytes = (unsigned long) vmstat.free_count * pageSize;
    usedBytes = totalBytes > freeBytes ? totalBytes - freeBytes : 0;

    printf("Memory: %luMiB / %luMiB\n", usedBytes / 1024 / 1024, totalBytes / 1024 / 1024);
}

/* ---- uptime: classic 4.3BSD technique - look up the kernel's boottime */
/* symbol with nlist() and read it out of /dev/kmem. This is what        */
/* uptime(1)/w(1) did on systems of this vintage; it needs read access   */
/* to /dev/kmem (commonly group "kmem"), and will just report "unknown"  */
/* if that's not available or if the leading-underscore symbol naming    */
/* assumption ("_boottime") doesn't match. --------------------------------- */

static int readBoottime(time_t *outSec)
{
    static const char *kernels[] = { "/mach", "/vmunix", "/unix", NULL };
    static const char *symbolNames[] = { "_boottime", "boottime", NULL };
    int k, s;

    for (k = 0; kernels[k] != NULL; ++k) {
        for (s = 0; symbolNames[s] != NULL; ++s) {
            struct nlist nl[2];
            int fd;

            memset(nl, 0, sizeof(nl));
            /* This system's struct nlist nests the name behind a union
             * (n_un.n_name) rather than exposing it directly. */
            nl[0].n_un.n_name = (char *) symbolNames[s];
            nl[1].n_un.n_name = "";

            if (nlist(kernels[k], nl) != 0) continue;
            if (nl[0].n_value == 0) continue;

            fd = open("/dev/kmem", O_RDONLY, 0);
            if (fd < 0) return 0;

            if (lseek(fd, (off_t) nl[0].n_value, SEEK_SET) == (off_t) nl[0].n_value) {
                struct timeval tv;
                if (read(fd, &tv, sizeof(tv)) == (int) sizeof(tv) && tv.tv_sec > 0) {
                    close(fd);
                    *outSec = tv.tv_sec;
                    return 1;
                }
            }
            close(fd);
        }
    }
    return 0;
}

static void printUptime(void)
{
    time_t boot;
    long diff, days, hours, mins;

    if (!readBoottime(&boot)) {
        printf("Uptime: unknown (nlist/kmem lookup failed - see NOTE in fastfetch.c)\n");
        return;
    }

    diff = (long) time(NULL) - (long) boot;
    if (diff < 0) diff = 0;

    days = diff / 86400;
    hours = (diff % 86400) / 3600;
    mins = (diff % 3600) / 60;

    printf("Uptime: ");
    if (days > 0) printf("%ld days, ", days);
    printf("%ld hours, %ld mins\n", hours, mins);
}

/* ---- shell / terminal: plain environment reads, fully portable -------- */

static void printShell(void)
{
    char *shell = getenv("SHELL");
    const char *base;

    if (shell == NULL) {
        printf("Shell: unknown\n");
        return;
    }

    base = strrchr(shell, '/');
    base = (base != NULL) ? base + 1 : shell;
    printf("Shell: %s\n", base);
}

static void printTerminal(void)
{
    char *term = getenv("TERM");
    printf("Terminal: %s\n", term != NULL ? term : "unknown");
}

/* ---- colors: cosmetic ANSI SGR swatches. NeXTSTEP's Terminal.app in   */
/* this era has limited/uncertain ANSI color support - if this prints    */
/* garbage escape codes instead of colored blocks, just drop the call in */
/* main(). --------------------------------------------------------------- */

static void printColors(void)
{
    int i;

    if (!ffIsTty()) return;

    for (i = 0; i < 8; ++i) printf("\033[4%dm   \033[0m", i);
    putchar('\n');
    for (i = 0; i < 8; ++i) printf("\033[10%dm   \033[0m", i);
    putchar('\n');
}

/* ---- logo: decorative only, no platform dependency --------------------- */

static const char *logo[] = {
    "     .----------.",
    "    /   NeXT   /|",
    "   /----------/ |",
    "  |          |  |",
    "  |          | /",
    "  '----------'/",
    "",
    NULL
};

/* ---- main ---------------------------------------------------------------- */

int main(void)
{
    int i;

    for (i = 0; logo[i] != NULL; ++i) {
        printf("%s\n", logo[i]);
    }

    printTitle();
    printOS();
    printHost();
    printKernel();
    printCPU();
    printMemory();
    printUptime();
    printShell();
    printTerminal();
    printColors();

    return 0;
}
