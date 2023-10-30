#include "fastfetch.h"

#include "modules/modules.h"

int main(void)
{
    ffInitInstance(); //This also applys default configuration to instance.config

    //Modify instance.config here
    FFOptionsModules* const options = &instance.config.modules;

    // ffPrepareCPUUsage();
    // ffPreparePublicIp(&options->publicIP);
    // ffPrepareWeather(&options->weather);

    //Does things like starting detection threads, disabling line wrap, etc
    ffStart();

    //Printing
    ffPrintTitle(&options->title);
    ffPrintSeparator(&options->separator);
    ffPrintOS(&options->os);
    ffPrintHost(&options->host);
    //ffPrintBios(&options->bios);
    //ffPrintBoard(&options->board);
    //ffPrintChassis(&options->chassis);
    ffPrintKernel(&options->kernel);
    //ffPrintProcesses(&options->processes);
    ffPrintUptime(&options->uptime);
    ffPrintPackages(&options->packages);
    ffPrintShell(&options->shell);
    ffPrintDisplay(&options->display);
    // ffPrintBrightness(&options->brightness);
    ffPrintDE(&options->de);
    ffPrintWM(&options->wm);
    ffPrintWMTheme(&options->wmTheme);
    ffPrintTheme(&options->theme);
    ffPrintIcons(&options->icons);
    ffPrintFont(&options->font);
    ffPrintCursor(&options->cursor);
    ffPrintTerminal(&options->terminal);
    ffPrintTerminalFont(&options->terminalFont);
    ffPrintCPU(&options->cpu);
    ffPrintGPU(&options->gpu);
    ffPrintMemory(&options->memory);
    ffPrintDisk(&options->disk);
    ffPrintBattery(&options->battery);
    ffPrintPowerAdapter(&options->powerAdapter);
    //ffPrintPlayer(&options->player);
    //ffPrintMedia(&options->media);
    //ffPrintLocalIp(&options->localIp);
    //ffPrintPublicIp(&options->publicIp);
    //ffPrintWifi(&options->wifi);
    //ffPrintCPUUsage(&options->cpuUsage);
    ffPrintLocale(&options->locale);
    //ffPrintDateTime(&options->dateTime);
    //ffPrintVulkan(&options->vulkan);
    //ffPrintOpenGL(&options->openGL);
    //ffPrintOpenCL(&options->openCL);
    //ffPrintUsers(&options->users);
    //ffPrintWeather(&options->weather);
    //ffPrintBluetooth(&options->bluetooth);
    //ffPrintSound(&options->sound);
    //ffPrintGamepad(&options->gamepad);
    ffPrintBreak(&options->break_);
    ffPrintColors(&options->colors);

    ffFinish();
    ffDestroyInstance();
    return 0;
}
