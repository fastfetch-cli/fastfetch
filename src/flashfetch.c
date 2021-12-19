#include "fastfetch.h"

int main(int argc, char** argv)
{
    //Disable compiler warnings
    FF_UNUSED(argc, argv);

    FFinstance instance;
    ffInitInstance(&instance); //This also applys default configuration to instance.config

    //ffLoadLogoSet(&instance, "my custom logo");
    ffStrbufSet(&instance.config.color, &instance.config.logoColors[0]); //Use the primary color of the logo as key color

    //Multithreading --> better performance
    ffStartDetectionThreads(&instance);

    //Does things like disabling line wrap
    ffStart(&instance);

    //Printing
    ffPrintTitle(&instance);
    ffPrintSeparator(&instance);
    ffPrintOS(&instance);
    ffPrintHost(&instance);
    ffPrintKernel(&instance);
    ffPrintUptime(&instance);
    ffPrintProcesses(&instance);
    ffPrintPackages(&instance);
    ffPrintShell(&instance);
    ffPrintResolution(&instance);
    ffPrintDesktopEnvironment(&instance);
    ffPrintWM(&instance);
    ffPrintWMTheme(&instance);
    ffPrintTheme(&instance);
    ffPrintIcons(&instance);
    ffPrintFont(&instance);
    ffPrintCursor(&instance);
    ffPrintTerminal(&instance);
    ffPrintTerminalFont(&instance);
    ffPrintCPU(&instance);
    //ffPrintCPUUsage(&instance);
    ffPrintGPU(&instance);
    ffPrintMemory(&instance);
    ffPrintDisk(&instance);
    ffPrintBattery(&instance);
    //ffPrintLocalIp(&instance);
    ffPrintLocale(&instance);
    ffPrintBreak(&instance);
    ffPrintColors(&instance);

    ffFinish(&instance);
    return 0;
}
