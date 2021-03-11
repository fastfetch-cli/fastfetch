#include "fastfetch.h"

int main(int argc, char** argv)
{
    FFinstance instance;
    ffInitState(&instance.state);
    ffDefaultConfig(&instance.config);

    //Configuration
    
    ffLoadLogoSet(&instance.config, "arch");
    strcpy(instance.config.color, instance.config.logo.color); //Use the primary color of the logo as key color

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

    return 0;
}