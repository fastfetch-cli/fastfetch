#include "fastfetch.h"

#include "string.h"

int main(int argc, char** argv)
{
    FFinstance instance;
    ffInitState(&instance.state);

    //Configuration

    instance.config.logo_seperator = 4;
    instance.config.offsetx = 0;
    instance.config.titleLength = 20; // This is overwritten by ffPrintTitle
    instance.config.colorLogo = true;
    instance.config.showErrors = false;
    instance.config.recache = false;
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
    ffPrintTheme(&instance);
    ffPrintIcons(&instance);
    ffPrintFont(&instance);
    ffPrintTerminal(&instance);
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