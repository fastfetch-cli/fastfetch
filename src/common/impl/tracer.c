// DON'T CALL ANY EXTERNAL FUNCTION IN THIS FILE

#include <stdio.h>
#include <time.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#if !_WIN32
    #include <dlfcn.h>
    #include <pthread.h>
    #include <unistd.h>
#else
    #include <windows.h>
    #include <dbghelp.h>
#endif

#if _WIN32
static LARGE_INTEGER freq;
    #define NtCurrentProcess() ((HANDLE) (-1))
#endif

static struct trace_event {
    uint64_t ts;
    uint64_t tid;
    void* func;
} events[4 * 1024 * 1024]; // 4M events, 96 MiB memory usage
static _Atomic uint32_t event_count;

__attribute__((no_instrument_function, always_inline)) static inline uint64_t get_time_us() {
#if !_WIN32
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
#else
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t) (now.QuadPart * 1000000 / freq.QuadPart);
#endif
}

#if _WIN32
__attribute__((constructor, no_instrument_function)) void trace_init() {
    QueryPerformanceFrequency(&freq);
}
#endif

__attribute__((destructor, no_instrument_function)) void trace_fini() {
#if _WIN32
    uint32_t pid = (uint32_t) GetCurrentProcessId();
#else
    uint32_t pid = (uint32_t) getpid();
#endif

    char fileName[64];
    snprintf(fileName, sizeof(fileName), "trace_%d.json", pid);
    FILE* trace_file = fopen(fileName, "wb");
    if (!trace_file) {
        fprintf(stderr, "Failed to open trace file %s for writing\n", fileName);
        abort();
    }

    fputs("[\n", trace_file);

#if _WIN32
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);
    SymInitialize(NtCurrentProcess(), NULL, TRUE);
#endif

    const char* fnName = NULL;

    uint32_t count = atomic_load_explicit(&event_count, memory_order_acquire);
    for (uint32_t i = 0; i < count; ++i) {
        void* fn = events[i].func;
        uint64_t ts = events[i].ts;
        bool is_exit = (ts >> 63) != 0;
        ts &= ~(1ULL << 63); // clear the highest bit
        uint64_t tid = events[i].tid;

#if _WIN32
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO) buffer;
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (SymFromAddr(NtCurrentProcess(), (DWORD64) fn, &displacement, pSymbol)) {
            fnName = pSymbol->Name;
        }
#else
        char buffer[32];
        Dl_info info;
        if (dladdr(fn, &info) && info.dli_sname) {
            fnName = info.dli_sname;
        }
#endif

        else {
            snprintf(buffer, sizeof(buffer), "%p", fn);
            fnName = buffer;
        }

        fprintf(trace_file, "{\"name\":\"%s\",\"ph\":\"%c\",\"pid\":%" PRIu32 ",\"tid\":%" PRIu64 ",\"ts\":%" PRIu64 "}%c\n", fnName, is_exit ? 'E' : 'B', pid, tid, ts, i < count - 1 ? ',' : ' ');
    }

    fputs("\n]\n", trace_file);
    fclose(trace_file);
    fprintf(stderr, "Trace written to trace_%d.json with %u events. Use https://ui.perfetto.dev/ to view it.\n", pid, count);

#if _WIN32
    SymCleanup(NtCurrentProcess());
#endif
}

__attribute__((no_instrument_function)) static void write_event(void* this_fn, bool is_exit) {
    uint32_t idx = atomic_fetch_add_explicit(&event_count, 1, memory_order_relaxed);
    if (__builtin_expect(idx >= sizeof(events) / sizeof(events[0]), false)) {
        abort();
    }

    events[idx].ts = get_time_us();
#if _WIN32
    events[idx].tid = (uint64_t) GetCurrentThreadId();
#else
    events[idx].tid = (uint64_t) (uintptr_t) pthread_self();
#endif
    events[idx].func = this_fn;
    if (is_exit) {
        events[idx].ts |= (1ULL << 63); // set the highest bit to indicate function exit
    }
}

__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void* this_fn, void* call_site) {
    (void) call_site;
    write_event(this_fn, false);
}

__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void* this_fn, void* call_site) {
    (void) call_site;
    write_event(this_fn, true);
}
