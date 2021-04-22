#include "fastfetch.h"

int main(int argc, char** argv)
{
    //Disable compiler warnings
    UNUSED(argc);
    UNUSED(argv);

    FFinstance instance;
    ffInitInstance(&instance); //This also applys default configuration to instance.config

    //Configuration
    ffLoadLogoSet(&instance.config, "arch");
    ffStrbufSetS(&instance.config.color, instance.config.logo.color); //Use the primary color of the logo as key color

    //Multithreading --> better performance
    ffStartCalculationThreads(&instance);

    //Printing
    ffPrintTitle(&instance);
    ffPrintSeperator(&instance);
    ffPrintOS(&instance);
    ffPrintHost(&instance);
    ffPrintKernel(&instance);
    ffPrintUptime(&instance);
    ffPrintPackages(&instance);
    ffPrintShell(&instance);
    ffPrintResolution(&instance);
    ffPrintDesktopEnvironment(&instance);
    ffPrintWM(&instance);
    ffPrintWMTheme(&instance);
    ffPrintTheme(&instance);
    ffPrintIcons(&instance);
    ffPrintFont(&instance);
    ffPrintTerminal(&instance);
    ffPrintTerminalFont(&instance);
    ffPrintCPU(&instance);
    ffPrintGPU(&instance);
    ffPrintMemory(&instance);
    ffPrintDisk(&instance);
    ffPrintBattery(&instance);
    ffPrintLocale(&instance);
    ffPrintBreak(&instance);
    ffPrintColors(&instance);

    ffFinish(&instance);
    return 0;
}
