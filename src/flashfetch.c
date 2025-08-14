#include "fastfetch.h"

#include "common/init.h"
#include "modules/modules.h"

// A dirty replicate of neofetch
int main(void)
{
    ffInitInstance(); // Init everything

    // Modify global config here if needed
    instance.config.display.sizeMaxPrefix = 2; // MB
    instance.config.display.sizeNdigits = 0;
    instance.config.display.freqNdigits = 3;
    instance.config.display.freqSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;
    instance.config.display.sizeSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;

    // Logo printing and other preparation stuff
    ffStart();

    // Print all modules
    {
        __attribute__((cleanup(ffDestroyTitleOptions))) FFTitleOptions options;
        ffInitTitleOptions(&options);
        ffPrintTitle(&options);
    }
    {
        __attribute__((cleanup(ffDestroySeparatorOptions))) FFSeparatorOptions options;
        ffInitSeparatorOptions(&options);
        ffPrintSeparator(&options);
    }
    {
        __attribute__((cleanup(ffDestroyOSOptions))) FFOSOptions options;
        ffInitOSOptions(&options);
        ffPrintOS(&options);
    }
    {
        __attribute__((cleanup(ffDestroyHostOptions))) FFHostOptions options;
        ffInitHostOptions(&options);
        ffPrintHost(&options);
    }
    {
        __attribute__((cleanup(ffDestroyKernelOptions))) FFKernelOptions options;
        ffInitKernelOptions(&options);
        ffStrbufSetStatic(&options.moduleArgs.outputFormat, "{release}");
        ffPrintKernel(&options);
    }
    {
        __attribute__((cleanup(ffDestroyUptimeOptions))) FFUptimeOptions options;
        ffInitUptimeOptions(&options);
        ffPrintUptime(&options);
    }
    {
        __attribute__((cleanup(ffDestroyPackagesOptions))) FFPackagesOptions options;
        ffInitPackagesOptions(&options);
        options.combined = true;
        ffPrintPackages(&options);
    }
    {
        __attribute__((cleanup(ffDestroyShellOptions))) FFShellOptions options;
        ffInitShellOptions(&options);
        ffPrintShell(&options);
    }
    {
        __attribute__((cleanup(ffDestroyDisplayOptions))) FFDisplayOptions options;
        ffInitDisplayOptions(&options);
        options.compactType = FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT;
        ffStrbufSetStatic(&options.moduleArgs.key, "Resolution");
        ffPrintDisplay(&options);
    }
    {
        __attribute__((cleanup(ffDestroyDEOptions))) FFDEOptions options;
        ffInitDEOptions(&options);
        ffPrintDE(&options);
    }
    {
        instance.config.general.detectVersion = false;
        __attribute__((cleanup(ffDestroyWMOptions))) FFWMOptions options;
        ffInitWMOptions(&options);
        options.detectPlugin = true;
        ffPrintWM(&options);
        instance.config.general.detectVersion = true;
    }
    {
        __attribute__((cleanup(ffDestroyWMThemeOptions))) FFWMThemeOptions options;
        ffInitWMThemeOptions(&options);
        ffPrintWMTheme(&options);
    }
    {
        __attribute__((cleanup(ffDestroyThemeOptions))) FFThemeOptions options;
        ffInitThemeOptions(&options);
        ffPrintTheme(&options);
    }
    {
        __attribute__((cleanup(ffDestroyIconsOptions))) FFIconsOptions options;
        ffInitIconsOptions(&options);
        ffPrintIcons(&options);
    }
    {
        __attribute__((cleanup(ffDestroyTerminalOptions))) FFTerminalOptions options;
        ffInitTerminalOptions(&options);
        ffPrintTerminal(&options);
    }
    {
        __attribute__((cleanup(ffDestroyTerminalFontOptions))) FFTerminalFontOptions options;
        ffInitTerminalFontOptions(&options);
        ffStrbufSetStatic(&options.moduleArgs.outputFormat, "{/name}{-}{/}{name}{?size} {size}{?}");
        ffPrintTerminalFont(&options);
    }
    {
        __attribute__((cleanup(ffDestroyCPUOptions))) FFCPUOptions options;
        ffInitCPUOptions(&options);
        ffPrintCPU(&options);
    }
    {
        __attribute__((cleanup(ffDestroyGPUOptions))) FFGPUOptions options;
        ffInitGPUOptions(&options);
        ffStrbufSetStatic(&options.moduleArgs.key, "GPU");
        ffStrbufSetStatic(&options.moduleArgs.outputFormat, "{name}");
        ffPrintGPU(&options);
    }
    {
        __attribute__((cleanup(ffDestroyMemoryOptions))) FFMemoryOptions options;
        ffInitMemoryOptions(&options);
        ffStrbufSetStatic(&options.moduleArgs.outputFormat, "{} / {}");
        ffPrintMemory(&options);
    }
    {
        __attribute__((cleanup(ffDestroyBreakOptions))) FFBreakOptions options;
        ffInitBreakOptions(&options);
        ffPrintBreak(&options);
    }
    {
        __attribute__((cleanup(ffDestroyColorsOptions))) FFColorsOptions options;
        ffInitColorsOptions(&options);
        ffPrintColors(&options);
    }

    ffFinish();
    ffDestroyInstance();
    return 0;
}
