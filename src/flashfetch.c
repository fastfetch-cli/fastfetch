#include "fastfetch.h"

#include "modules/modules.h"

int main(void)
{
    ffInitInstance(); //This also applys default configuration to instance.config

    //Modify instance.config here

    // ffPrepareCPUUsage();
    // ffPreparePublicIp(&instance.config.publicIP);
    // ffPrepareWeather(&instance.config.weather);

    //Does things like starting detection threads, disabling line wrap, etc
    ffStart();

    //Printing
    ffPrintTitle(&instance.config.title);
    ffPrintSeparator(&instance.config.separator);
    ffPrintOS(&instance.config.os);
    ffPrintHost(&instance.config.host);
    //ffPrintBios(&instance.config.bios);
    //ffPrintBoard(&instance.config.board);
    //ffPrintChassis(&instance.config.chassis);
    ffPrintKernel(&instance.config.kernel);
    //ffPrintProcesses(&instance.config.processes);
    ffPrintUptime(&instance.config.uptime);
    ffPrintPackages(&instance.config.packages);
    ffPrintShell(&instance.config.shell);
    ffPrintDisplay(&instance.config.display);
    // ffPrintBrightness(&instance.config.brightness);
    ffPrintDE(&instance.config.de);
    ffPrintWM(&instance.config.wm);
    ffPrintWMTheme(&instance.config.wmTheme);
    ffPrintTheme(&instance.config.theme);
    ffPrintIcons(&instance.config.icons);
    ffPrintFont(&instance.config.font);
    ffPrintCursor(&instance.config.cursor);
    ffPrintTerminal(&instance.config.terminal);
    ffPrintTerminalFont(&instance.config.terminalFont);
    ffPrintCPU(&instance.config.cpu);
    ffPrintGPU(&instance.config.gpu);
    ffPrintMemory(&instance.config.memory);
    ffPrintSwap(&instance.config.swap);
    ffPrintDisk(&instance.config.disk);
    ffPrintBattery(&instance.config.battery);
    ffPrintPowerAdapter(&instance.config.powerAdapter);
    //ffPrintPlayer(&instance.config.player);
    //ffPrintMedia(&instance.config.media);
    //ffPrintLocalIp(&instance.config.localIp);
    //ffPrintPublicIp(&instance.config.publicIp);
    //ffPrintWifi(&instance.config.wifi);
    //ffPrintCPUUsage(&instance.config.cpuUsage);
    ffPrintLocale(&instance.config.locale);
    //ffPrintDateTime(&instance.config.dateTime);
    //ffPrintVulkan(&instance.config.vulkan);
    //ffPrintOpenGL(&instance.config.openGL);
    //ffPrintOpenCL(&instance.config.openCL);
    //ffPrintUsers(&instance.config.users);
    //ffPrintWeather(&instance.config.weather);
    //ffPrintBluetooth(&instance.config.bluetooth);
    //ffPrintSound(&instance.config.sound);
    //ffPrintGamepad(&instance.config.gamepad);
    ffPrintBreak();
    ffPrintColors(&instance.config.colors);

    ffFinish();
    ffDestroyInstance();
    return 0;
}
