#include "fastfetch.h"

#include "modules/modules.h"

int main(int argc, char** argv)
{
    //Disable compiler warnings
    FF_UNUSED(argc, argv);

    FFinstance instance;
    ffInitInstance(&instance); //This also applys default configuration to instance.config

    //Modify instance.config here

    // ffPrepareCPUUsage();
    // ffPreparePublicIp(&instance);
    // ffPrepareWeather(&instance);

    //Does things like starting detection threads, disabling line wrap, etc
    ffStart(&instance);

    //Printing
    ffPrintTitle(&instance, &instance.config.title);
    ffPrintSeparator(&instance, &instance.config.separator);
    ffPrintOS(&instance, &instance.config.os);
    ffPrintHost(&instance, &instance.config.host);
    //ffPrintBios(&instance);
    //ffPrintBoard(&instance);
    //ffPrintChassis(&instance);
    ffPrintKernel(&instance, &instance.config.kernel);
    ffPrintUptime(&instance);
    //ffPrintProcesses(&instance);
    ffPrintPackages(&instance);
    ffPrintShell(&instance);
    ffPrintDisplay(&instance, &instance.config.display);
    // ffPrintBrightness(&instance);
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
    ffPrintGPU(&instance);
    ffPrintMemory(&instance);
    //ffPrintSwap(&instance);
    ffPrintDisk(&instance);
    ffPrintBattery(&instance, &instance.config.battery);
    ffPrintPowerAdapter(&instance);
    //ffPrintPlayer(&instance);
    //ffPrintMedia(&instance);
    //ffPrintLocalIp(&instance);
    //ffPrintPublicIp(&instance);
    //ffPrintWifi(&instance);
    //ffPrintCPUUsage(&instance);
    ffPrintLocale(&instance);
    //ffPrintDateTime(&instance);
    //ffPrintDate(&instance);
    //ffPrintTime(&instance);
    //ffPrintVulkan(&instance);
    //ffPrintOpenGL(&instance);
    //ffPrintOpenCL(&instance);
    //ffPrintUsers(&instance);
    //ffPrintWeather(&instance);
    //ffPrintBluetooth(&instance);
    //ffPrintSound(&instance);
    //ffPrintGamepad(&instance);
    ffPrintBreak(&instance);
    ffPrintColors(&instance);

    ffFinish(&instance);
    ffDestroyInstance(&instance);
    return 0;
}
