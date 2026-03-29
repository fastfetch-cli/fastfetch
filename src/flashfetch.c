#include "fastfetch.h"

#include "common/init.h"
#include "logo/logo.h"
#include "modules/modules.h"

#define MODULE_OPTION(name)                                                       \
    __attribute__((cleanup(ffDestroy##name##Options))) FF##name##Options options; \
    ffInit##name##Options(&options);

// A dirty replicate of neofetch; demonstration only.
int main(void) {
    ffInitInstance(); // Init everything

    // Modify global config here if needed
    instance.config.display.sizeMaxPrefix = 2; // MB
    instance.config.display.sizeNdigits = 0;
    instance.config.display.freqNdigits = 3;
    instance.config.display.freqSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;
    instance.config.display.sizeSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;

    // Some preparation stuff
    ffStart();

    // Print logo
    ffLogoPrint();

    // Print all modules
    {
        MODULE_OPTION(Title)
        ffPrintTitle(&options);
    }
    {
        MODULE_OPTION(Separator)
        ffPrintSeparator(&options);
    }
    {
        MODULE_OPTION(OS)
        ffPrintOS(&options);
    }
    {
        MODULE_OPTION(Host)
        ffPrintHost(&options);
    }
    {
        MODULE_OPTION(Kernel)
        ffStrbufSetStatic(&options.moduleArgs.outputFormat, "{release}");
        ffPrintKernel(&options);
    }
    {
        MODULE_OPTION(Uptime)
        ffPrintUptime(&options);
    }
    {
        MODULE_OPTION(Packages)
        options.combined = true;
        ffPrintPackages(&options);
    }
    {
        MODULE_OPTION(Shell)
        ffPrintShell(&options);
    }
    {
        MODULE_OPTION(Display)
        options.compactType = FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT;
        ffStrbufSetStatic(&options.moduleArgs.key, "Resolution");
        ffPrintDisplay(&options);
    }
    {
        MODULE_OPTION(DE)
        ffPrintDE(&options);
    }
    {
        instance.config.general.detectVersion = false;
        MODULE_OPTION(WM)
        options.detectPlugin = true;
        ffPrintWM(&options);
        instance.config.general.detectVersion = true;
    }
    {
        MODULE_OPTION(WMTheme)
        ffPrintWMTheme(&options);
    }
    {
        MODULE_OPTION(Theme)
        ffPrintTheme(&options);
    }
    {
        MODULE_OPTION(Icons)
        ffPrintIcons(&options);
    }
    {
        MODULE_OPTION(Terminal)
        ffPrintTerminal(&options);
    }
    {
        MODULE_OPTION(TerminalFont)
        ffStrbufSetStatic(&options.moduleArgs.outputFormat, "{/name}{-}{/}{name}{?size} {size}{?}");
        ffPrintTerminalFont(&options);
    }
    {
        MODULE_OPTION(CPU)
        ffPrintCPU(&options);
    }
    {
        MODULE_OPTION(GPU)
        ffStrbufSetStatic(&options.moduleArgs.key, "GPU");
        ffStrbufSetStatic(&options.moduleArgs.outputFormat, "{name}");
        ffPrintGPU(&options);
    }
    {
        MODULE_OPTION(Memory)
        ffStrbufSetStatic(&options.moduleArgs.outputFormat, "{} / {}");
        ffPrintMemory(&options);
    }
    {
        MODULE_OPTION(Break)
        ffPrintBreak(&options);
    }
    {
        MODULE_OPTION(Colors)
        ffPrintColors(&options);
    }

    ffLogoPrintRemaining();
    ffFinish();
    ffDestroyInstance();
    return 0;
}
