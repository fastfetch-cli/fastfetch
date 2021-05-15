#include "fastfetch.h"

#include <time.h>

#define FASTFETCH_TEST_PERFORMANCE(func) \
    { \
        struct timespec start, end; \
        clock_gettime(CLOCK_REALTIME, &start); \
        func; \
        clock_gettime(CLOCK_REALTIME, &end); \
        long nanos = end.tv_nsec - start.tv_nsec; \
        printf("\033[F"); \
        for(uint8_t i = 0; i < 110; i++) printf("\033[C"); \
        printf("%08ldns\n", nanos); \
    } \

int main(int argc, char** argv)
{
    UNUSED(argc);
    UNUSED(argv);

    FFinstance instance;

    FASTFETCH_TEST_PERFORMANCE(
        puts("Initialization");
        ffInitInstance(&instance);
    )

    FASTFETCH_TEST_PERFORMANCE(
        puts("Configuration");
        ffLoadLogoSet(&instance.config, "arch");
        ffStrbufSetS(&instance.config.color, instance.config.logo.color);
        instance.config.showErrors = true;
        instance.config.recache = argc == 1;
        instance.config.cacheSave = false;
    )

    FASTFETCH_TEST_PERFORMANCE(
        puts("Thread starting");
        ffStartCalculationThreads(&instance);
    )

    FASTFETCH_TEST_PERFORMANCE(ffPrintTitle(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintSeparator(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintOS(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintHost(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintKernel(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintUptime(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintPackages(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintShell(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintResolution(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintDesktopEnvironment(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintWM(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintWMTheme(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintTheme(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintIcons(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintFont(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintTerminal(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintTerminalFont(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintCPU(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintGPU(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintMemory(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintDisk(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintBattery(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintLocale(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintBreak(&instance))
    FASTFETCH_TEST_PERFORMANCE(ffPrintColors(&instance))

    return 0;
}
