#include "detection/cpu/cpu_riscv.h"

#include <stdlib.h>

static void verify(const char* input, uint16_t online, const char* soc, const char* expected, bool changed) {
    FF_STRBUF_AUTO_DESTROY cpuinfo = ffStrbufCreateS(input);
    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateS(soc);
    bool result = ffCPUDetectRiscvUarch(&cpuinfo, online, &name);
    if (result != changed || !ffStrbufEqualS(&name, expected) || !ffStrbufEqualS(&cpuinfo, input)) {
        fprintf(stderr, "Expected '%s' (%d), got '%s' (%d)\n", expected, changed, name.chars, result);
        exit(1);
    }
}

int main(void) {
    // Interleaved IDs must be grouped by uarch, not adjacency or frequency.
    const char* mixed =
        "processor\t: 0\nuarch\t: thead,c908\n\n"
        "processor\t: 4\nuarch\t: thead,c920\n\n"
        "processor\t: 1\nuarch\t: thead,c908\n\n"
        "processor\t: 5\nuarch\t: thead,c920\n\n"
        "processor\t: 2\nuarch\t: thead,c908\n\n"
        "processor\t: 6\nuarch\t: thead,c920\n\n"
        "processor\t: 3\nuarch\t: thead,c908\n\n"
        "processor\t: 7\nuarch\t: thead,c920\n";
    verify(mixed, 8, "a210", "a210 (4 x thead,c908 + 4 x thead,c920)", true);
    verify(mixed, 8, "", "4 x thead,c908 + 4 x thead,c920", true);
    verify(mixed, 7, "a210", "a210", false);
    verify("processor : 0\nuarch : vendor,small\n\nprocessor : 7\nuarch : vendor,large",
        2, "SoC", "SoC (1 x vendor,small + 1 x vendor,large)", true);
    verify("processor : 0\nuarch : thead,c908\n\nprocessor : 1\nuarch : thead,c908\n",
        2, "SoC", "SoC", false);
    verify("processor : 0\nisa : rv64imafdc\n", 1, "SoC", "SoC", false);
    verify("processor : 0\nuarch : vendor,a\n\nprocessor : 1\n\nprocessor : 2\nuarch : vendor,b\n",
        3, "SoC", "SoC", false);
    verify("processor : 0\nuarch : vendor,a\nuarch : vendor,b\n",
        1, "SoC", "SoC", false);
    verify("processor : 0\nuarch : \n", 1, "SoC", "SoC", false);
    verify("", 0, "SoC", "SoC", false);
    puts("RISC-V uarch tests passed");
    return 0;
}
