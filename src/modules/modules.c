#include "modules/modules.h"

// Note:
// FF_DISABLE_MODULE_<module> only controls if the module is registered,
// the module code itself is still compiled.
// We rely `LTO` to remove the unused code (only enabled in Release mode)

static FFModuleBaseInfo* A[] = {
    NULL,
};

static FFModuleBaseInfo* B[] = {
#if !FF_DISABLE_MODULE_BATTERY
    &ffBatteryModuleInfo,
#endif
#if !FF_DISABLE_MODULE_BIOS
    &ffBiosModuleInfo,
#endif
#if !FF_DISABLE_MODULE_BLUETOOTH
    &ffBluetoothModuleInfo,
#endif
#if !FF_DISABLE_MODULE_BLUETOOTHRADIO
    &ffBluetoothRadioModuleInfo,
#endif
#if !FF_DISABLE_MODULE_BOARD
    &ffBoardModuleInfo,
#endif
#if !FF_DISABLE_MODULE_BOOTMGR
    &ffBootmgrModuleInfo,
#endif
#if !FF_DISABLE_MODULE_BREAK
    &ffBreakModuleInfo,
#endif
#if !FF_DISABLE_MODULE_BRIGHTNESS
    &ffBrightnessModuleInfo,
#endif
#if !FF_DISABLE_MODULE_BTRFS
    &ffBtrfsModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* C[] = {
#if !FF_DISABLE_MODULE_CAMERA
    &ffCameraModuleInfo,
#endif
#if !FF_DISABLE_MODULE_CHASSIS
    &ffChassisModuleInfo,
#endif
#if !FF_DISABLE_MODULE_COMMAND
    &ffCommandModuleInfo,
#endif
#if !FF_DISABLE_MODULE_COLORS
    &ffColorsModuleInfo,
#endif
#if !FF_DISABLE_MODULE_CPU
    &ffCPUModuleInfo,
#endif
#if !FF_DISABLE_MODULE_CPUCACHE
    &ffCPUCacheModuleInfo,
#endif
#if !FF_DISABLE_MODULE_CPUUSAGE
    &ffCPUUsageModuleInfo,
#endif
#if !FF_DISABLE_MODULE_CURSOR
    &ffCursorModuleInfo,
#endif
#if !FF_DISABLE_MODULE_CUSTOM
    &ffCustomModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* D[] = {
#if !FF_DISABLE_MODULE_DATETIME
    &ffDateTimeModuleInfo,
#endif
#if !FF_DISABLE_MODULE_DE
    &ffDEModuleInfo,
#endif
#if !FF_DISABLE_MODULE_DISPLAY
    &ffDisplayModuleInfo,
#endif
#if !FF_DISABLE_MODULE_DISK
    &ffDiskModuleInfo,
#endif
#if !FF_DISABLE_MODULE_DISKIO
    &ffDiskIOModuleInfo,
#endif
#if !FF_DISABLE_MODULE_DNS
    &ffDNSModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* E[] = {
#if !FF_DISABLE_MODULE_EDITOR
    &ffEditorModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* F[] = {
#if !FF_DISABLE_MODULE_FONT
    &ffFontModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* G[] = {
#if !FF_DISABLE_MODULE_GAMEPAD
    &ffGamepadModuleInfo,
#endif
#if !FF_DISABLE_MODULE_GPU
    &ffGPUModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* H[] = {
#if !FF_DISABLE_MODULE_HOST
    &ffHostModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* I[] = {
#if !FF_DISABLE_MODULE_ICONS
    &ffIconsModuleInfo,
#endif
#if !FF_DISABLE_MODULE_INITSYSTEM
    &ffInitSystemModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* J[] = {
    NULL,
};

static FFModuleBaseInfo* K[] = {
#if !FF_DISABLE_MODULE_KERNEL
    &ffKernelModuleInfo,
#endif
#if !FF_DISABLE_MODULE_KEYBOARD
    &ffKeyboardModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* L[] = {
#if !FF_DISABLE_MODULE_LM
    &ffLMModuleInfo,
#endif
#if !FF_DISABLE_MODULE_LOADAVG
    &ffLoadavgModuleInfo,
#endif
#if !FF_DISABLE_MODULE_LOCALE
    &ffLocaleModuleInfo,
#endif
#if !FF_DISABLE_MODULE_LOCALIP
    &ffLocalIPModuleInfo,
#endif
#if !FF_DISABLE_MODULE_LOGO
    &ffLogoModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* M[] = {
#if !FF_DISABLE_MODULE_MEDIA
    &ffMediaModuleInfo,
#endif
#if !FF_DISABLE_MODULE_MEMORY
    &ffMemoryModuleInfo,
#endif
#if !FF_DISABLE_MODULE_MONITOR
    &ffMonitorModuleInfo,
#endif
#if !FF_DISABLE_MODULE_MOUSE
    &ffMouseModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* N[] = {
#if !FF_DISABLE_MODULE_NETIO
    &ffNetIOModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* O[] = {
#if !FF_DISABLE_MODULE_OPENCL
    &ffOpenCLModuleInfo,
#endif
#if !FF_DISABLE_MODULE_OPENGL
    &ffOpenGLModuleInfo,
#endif
#if !FF_DISABLE_MODULE_OS
    &ffOSModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* P[] = {
#if !FF_DISABLE_MODULE_PACKAGES
    &ffPackagesModuleInfo,
#endif
#if !FF_DISABLE_MODULE_PHYSICALDISK
    &ffPhysicalDiskModuleInfo,
#endif
#if !FF_DISABLE_MODULE_PHYSICALMEMORY
    &ffPhysicalMemoryModuleInfo,
#endif
#if !FF_DISABLE_MODULE_PLAYER
    &ffPlayerModuleInfo,
#endif
#if !FF_DISABLE_MODULE_POWERADAPTER
    &ffPowerAdapterModuleInfo,
#endif
#if !FF_DISABLE_MODULE_PROCESSES
    &ffProcessesModuleInfo,
#endif
#if !FF_DISABLE_MODULE_PUBLICIP
    &ffPublicIPModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* Q[] = {
    NULL,
};

static FFModuleBaseInfo* R[] = {
    NULL,
};

static FFModuleBaseInfo* S[] = {
#if !FF_DISABLE_MODULE_SEPARATOR
    &ffSeparatorModuleInfo,
#endif
#if !FF_DISABLE_MODULE_SHELL
    &ffShellModuleInfo,
#endif
#if !FF_DISABLE_MODULE_SOUND
    &ffSoundModuleInfo,
#endif
#if !FF_DISABLE_MODULE_SWAP
    &ffSwapModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* T[] = {
#if !FF_DISABLE_MODULE_TERMINAL
    &ffTerminalModuleInfo,
#endif
#if !FF_DISABLE_MODULE_TERMINALFONT
    &ffTerminalFontModuleInfo,
#endif
#if !FF_DISABLE_MODULE_TERMINALSIZE
    &ffTerminalSizeModuleInfo,
#endif
#if !FF_DISABLE_MODULE_TERMINALTHEME
    &ffTerminalThemeModuleInfo,
#endif
#if !FF_DISABLE_MODULE_TITLE
    &ffTitleModuleInfo,
#endif
#if !FF_DISABLE_MODULE_THEME
    &ffThemeModuleInfo,
#endif
#if !FF_DISABLE_MODULE_TPM
    &ffTPMModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* U[] = {
#if !FF_DISABLE_MODULE_UPTIME
    &ffUptimeModuleInfo,
#endif
#if !FF_DISABLE_MODULE_USERS
    &ffUsersModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* V[] = {
#if !FF_DISABLE_MODULE_VERSION
    &ffVersionModuleInfo,
#endif
#if !FF_DISABLE_MODULE_VULKAN
    &ffVulkanModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* W[] = {
#if !FF_DISABLE_MODULE_WALLPAPER
    &ffWallpaperModuleInfo,
#endif
#if !FF_DISABLE_MODULE_WEATHER
    &ffWeatherModuleInfo,
#endif
#if !FF_DISABLE_MODULE_WM
    &ffWMModuleInfo,
#endif
#if !FF_DISABLE_MODULE_WIFI
    &ffWifiModuleInfo,
#endif
#if !FF_DISABLE_MODULE_WMTHEME
    &ffWMThemeModuleInfo,
#endif
    NULL,
};

static FFModuleBaseInfo* X[] = {
    NULL,
};

static FFModuleBaseInfo* Y[] = {
    NULL,
};

static FFModuleBaseInfo* Z[] = {
#if !FF_DISABLE_MODULE_ZPOOL
    &ffZpoolModuleInfo,
#endif
    NULL,
};

FFModuleBaseInfo** ffModuleInfos[] = {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
};
