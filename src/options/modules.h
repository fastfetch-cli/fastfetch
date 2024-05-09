#pragma once

#include "modules/options.h"

typedef struct FFOptionsModules
{
    FFBatteryOptions battery;
    FFBiosOptions bios;
    FFBluetoothOptions bluetooth;
    FFBoardOptions board;
    FFBreakOptions break_;
    FFBrightnessOptions brightness;
    FFCPUOptions cpu;
    FFCPUUsageOptions cpuUsage;
    FFCameraOptions camera;
    FFChassisOptions chassis;
    FFColorsOptions colors;
    FFCommandOptions command;
    FFCursorOptions cursor;
    FFCustomOptions custom;
    FFDEOptions de;
    FFDateTimeOptions dateTime;
    FFDiskOptions disk;
    FFDiskIOOptions diskIo;
    FFDisplayOptions display;
    FFFontOptions font;
    FFGPUOptions gpu;
    FFGamepadOptions gamepad;
    FFHostOptions host;
    FFIconsOptions icons;
    FFKernelOptions kernel;
    FFLMOptions lm;
    FFLoadavgOptions loadavg;
    FFLocalIpOptions localIP;
    FFLocaleOptions locale;
    FFMediaOptions media;
    FFMemoryOptions memory;
    FFMonitorOptions monitor;
    FFNetIOOptions netIo;
    FFOSOptions os;
    FFOpenCLOptions openCL;
    FFOpenGLOptions openGL;
    FFPackagesOptions packages;
    FFPhysicalDiskOptions physicalDisk;
    FFPlayerOptions player;
    FFPowerAdapterOptions powerAdapter;
    FFProcessesOptions processes;
    FFPublicIpOptions publicIP;
    FFSeparatorOptions separator;
    FFShellOptions shell;
    FFSoundOptions sound;
    FFSwapOptions swap;
    FFTerminalOptions terminal;
    FFTerminalFontOptions terminalFont;
    FFTerminalSizeOptions terminalSize;
    FFTerminalThemeOptions terminalTheme;
    FFThemeOptions theme;
    FFTitleOptions title;
    FFUptimeOptions uptime;
    FFUsersOptions users;
    FFVersionOptions version;
    FFVulkanOptions vulkan;
    FFWMOptions wm;
    FFWMThemeOptions wmTheme;
    FFWallpaperOptions wallpaper;
    FFWeatherOptions weather;
    FFWifiOptions wifi;
} FFOptionsModules;

void ffOptionsInitModules(FFOptionsModules* options);
void ffOptionsDestroyModules(FFOptionsModules* options);
