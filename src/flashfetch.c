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
    // ffPreparePublicIp(&instance.config.publicIP);
    // ffPrepareWeather(&instance);

    //Does things like starting detection threads, disabling line wrap, etc
    ffStart(&instance);

    //Printing
    ffPrintTitle(&instance, &instance.config.title);
    ffPrintSeparator(&instance, &instance.config.separator);
    ffPrintOS(&instance, &instance.config.os);
    ffPrintHost(&instance, &instance.config.host);
    //ffPrintBios(&instance, &instance.config.bios);
    //ffPrintBoard(&instance, &instance.config.board);
    //ffPrintChassis(&instance);
    ffPrintKernel(&instance, &instance.config.kernel);
    //ffPrintProcesses(&instance);
    ffPrintUptime(&instance, &instance.config.uptime);
    ffPrintPackages(&instance, &instance.config.packages);
    ffPrintShell(&instance, &instance.config.shell);
    ffPrintDisplay(&instance, &instance.config.display);
    // ffPrintBrightness(&instance);
    ffPrintDE(&instance, &instance.config.de);
    ffPrintWM(&instance, &instance.config.wm);
    ffPrintWMTheme(&instance, &instance.config.wmTheme);
    ffPrintTheme(&instance);
    ffPrintIcons(&instance, &instance.config.icons);
    ffPrintFont(&instance, &instance.config.font);
    ffPrintCursor(&instance, &instance.config.cursor);
    ffPrintTerminal(&instance, &instance.config.terminal);
    ffPrintTerminalFont(&instance, &instance.config.terminalFont);
    ffPrintCPU(&instance, &instance.config.cpu);
    ffPrintGPU(&instance, &instance.config.gpu);
    ffPrintMemory(&instance, &instance.config.memory);
    ffPrintSwap(&instance, &instance.config.swap);
    ffPrintDisk(&instance, &instance.config.disk);
    ffPrintBattery(&instance, &instance.config.battery);
    ffPrintPowerAdapter(&instance, &instance.config.powerAdapter);
    //ffPrintPlayer(&instance, &instance.config.player);
    //ffPrintMedia(&instance, &instance.config.media);
    //ffPrintLocalIp(&instance);
    //ffPrintPublicIp(&instance, &instance.config.publicIp);
    //ffPrintWifi(&instance);
    //ffPrintCPUUsage(&instance, &instance.config.cpuUsage);
    ffPrintLocale(&instance, &instance.config.locale);
    //ffPrintDateTime(&instance);
    //ffPrintDate(&instance);
    //ffPrintTime(&instance);
    //ffPrintVulkan(&instance, &instance.config.vulkan);
    //ffPrintOpenGL(&instance, &instance.config.openGL);
    //ffPrintOpenCL(&instance, &instance.config.openCL);
    //ffPrintUsers(&instance, &instance.config.users);
    //ffPrintWeather(&instance);
    //ffPrintBluetooth(&instance);
    //ffPrintSound(&instance, &instance.config.sound);
    //ffPrintGamepad(&instance);
    ffPrintBreak(&instance);
    ffPrintColors(&instance);

    ffFinish(&instance);
    ffDestroyInstance(&instance);
    return 0;
}
