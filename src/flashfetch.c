#include "fastfetch.h"

int main(int argc, char** argv)
{
    //Disable compiler warnings
    FF_UNUSED(argc, argv);

    FFinstance instance;
    ffInitInstance(&instance); //This also applys default configuration to instance.config

    //Modify instance.config here

    //Does things like starting detection threads, disabling line wrap, etc
    ffStart(&instance);

    //Printing
    ffPrintTitle(&instance);
    ffPrintSeparator(&instance);
    ffPrintOS(&instance);
    ffPrintHost(&instance);
    ffPrintKernel(&instance);
    ffPrintUptime(&instance);
    //ffPrintProcesses(&instance);
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
    //ffPrintSwap(&instance);
    ffPrintDisk(&instance);
    ffPrintBattery(&instance);
    ffPrintPowerAdapter(&instance);
    //ffPrintPlayer(&instance);
    //ffPrintSong(&instance);
    //ffPrintLocalIp(&instance);
    //ffPrintPublicIp(&instance);
    ffPrintLocale(&instance);
    //ffPrintDateTime(&instance);
    //ffPrintDate(&instance);
    //ffPrintTime(&instance);
    //ffPrintVulkan(&instance);
    //ffPrintOpenGL(&instance);
    //ffPrintOpenCL(&instance);
    //ffPrintUsers(&instance);
    ffPrintBreak(&instance);
    ffPrintColors(&instance);

    ffFinish(&instance);
    ffDestroyInstance(&instance);
    return 0;
}
