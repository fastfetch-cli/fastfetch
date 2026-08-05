#include "modules/modules.h"

// Note:
// FF_MODULE_DISABLE_<module> only controls if the module is registered,
// the module code itself is still compiled.
// We rely `LTO` to remove the unused code (only enabled in Release mode)

static FFModuleBaseInfo* A[] = {
    nullptr,
};

static FFModuleBaseInfo* B[] = {
#if !FF_MODULE_DISABLE_BATTERY
    &ffBatteryModuleInfo,
#endif
#if !FF_MODULE_DISABLE_BIOS
    &ffBiosModuleInfo,
#endif
#if !FF_MODULE_DISABLE_BLUETOOTH
    &ffBluetoothModuleInfo,
#endif
#if !FF_MODULE_DISABLE_BLUETOOTHRADIO
    &ffBluetoothRadioModuleInfo,
#endif
#if !FF_MODULE_DISABLE_BOARD
    &ffBoardModuleInfo,
#endif
#if !FF_MODULE_DISABLE_BOOTMGR
    &ffBootmgrModuleInfo,
#endif
#if !FF_MODULE_DISABLE_BREAK
    &ffBreakModuleInfo,
#endif
#if !FF_MODULE_DISABLE_BRIGHTNESS
    &ffBrightnessModuleInfo,
#endif
#if !FF_MODULE_DISABLE_BTRFS
    &ffBtrfsModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* C[] = {
#if !FF_MODULE_DISABLE_CAMERA
    &ffCameraModuleInfo,
#endif
#if !FF_MODULE_DISABLE_CHASSIS
    &ffChassisModuleInfo,
#endif
#if !FF_MODULE_DISABLE_CODEC
    &ffCodecModuleInfo,
#endif
#if !FF_MODULE_DISABLE_COMMAND
    &ffCommandModuleInfo,
#endif
#if !FF_MODULE_DISABLE_COLORS
    &ffColorsModuleInfo,
#endif
#if !FF_MODULE_DISABLE_CPU
    &ffCPUModuleInfo,
#endif
#if !FF_MODULE_DISABLE_CPUCACHE
    &ffCPUCacheModuleInfo,
#endif
#if !FF_MODULE_DISABLE_CPUUSAGE
    &ffCPUUsageModuleInfo,
#endif
#if !FF_MODULE_DISABLE_CURSOR
    &ffCursorModuleInfo,
#endif
#if !FF_MODULE_DISABLE_CUSTOM
    &ffCustomModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* D[] = {
#if !FF_MODULE_DISABLE_DATETIME
    &ffDateTimeModuleInfo,
#endif
#if !FF_MODULE_DISABLE_DE
    &ffDEModuleInfo,
#endif
#if !FF_MODULE_DISABLE_DISPLAY
    &ffDisplayModuleInfo,
#endif
#if !FF_MODULE_DISABLE_DISK
    &ffDiskModuleInfo,
#endif
#if !FF_MODULE_DISABLE_DISKIO
    &ffDiskIOModuleInfo,
#endif
#if !FF_MODULE_DISABLE_DNS
    &ffDNSModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* E[] = {
#if !FF_MODULE_DISABLE_EDITOR
    &ffEditorModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* F[] = {
#if !FF_MODULE_DISABLE_FONT
    &ffFontModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* G[] = {
#if !FF_MODULE_DISABLE_GAMEPAD
    &ffGamepadModuleInfo,
#endif
#if !FF_MODULE_DISABLE_GPU
    &ffGPUModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* H[] = {
#if !FF_MODULE_DISABLE_HOST
    &ffHostModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* I[] = {
#if !FF_MODULE_DISABLE_ICONS
    &ffIconsModuleInfo,
#endif
#if !FF_MODULE_DISABLE_INITSYSTEM
    &ffInitSystemModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* J[] = {
    nullptr,
};

static FFModuleBaseInfo* K[] = {
#if !FF_MODULE_DISABLE_KERNEL
    &ffKernelModuleInfo,
#endif
#if !FF_MODULE_DISABLE_KEYBOARD
    &ffKeyboardModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* L[] = {
#if !FF_MODULE_DISABLE_LM
    &ffLMModuleInfo,
#endif
#if !FF_MODULE_DISABLE_LOADAVG
    &ffLoadavgModuleInfo,
#endif
#if !FF_MODULE_DISABLE_LOCALE
    &ffLocaleModuleInfo,
#endif
#if !FF_MODULE_DISABLE_LOCALIP
    &ffLocalIPModuleInfo,
#endif
#if !FF_MODULE_DISABLE_LOGO
    &ffLogoModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* M[] = {
#if !FF_MODULE_DISABLE_MEDIA
    &ffMediaModuleInfo,
#endif
#if !FF_MODULE_DISABLE_MEMORY
    &ffMemoryModuleInfo,
#endif
#if !FF_MODULE_DISABLE_MONITOR
    &ffMonitorModuleInfo,
#endif
#if !FF_MODULE_DISABLE_MOUSE
    &ffMouseModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* N[] = {
#if !FF_MODULE_DISABLE_NETIO
    &ffNetIOModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* O[] = {
#if !FF_MODULE_DISABLE_OPENCL
    &ffOpenCLModuleInfo,
#endif
#if !FF_MODULE_DISABLE_OPENGL
    &ffOpenGLModuleInfo,
#endif
#if !FF_MODULE_DISABLE_OS
    &ffOSModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* P[] = {
#if !FF_MODULE_DISABLE_PACKAGES
    &ffPackagesModuleInfo,
#endif
#if !FF_MODULE_DISABLE_PHYSICALDISK
    &ffPhysicalDiskModuleInfo,
#endif
#if !FF_MODULE_DISABLE_PHYSICALMEMORY
    &ffPhysicalMemoryModuleInfo,
#endif
#if !FF_MODULE_DISABLE_PLAYER
    &ffPlayerModuleInfo,
#endif
#if !FF_MODULE_DISABLE_POWERADAPTER
    &ffPowerAdapterModuleInfo,
#endif
#if !FF_MODULE_DISABLE_PROCESSES
    &ffProcessesModuleInfo,
#endif
#if !FF_MODULE_DISABLE_PUBLICIP
    &ffPublicIPModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* Q[] = {
    nullptr,
};

static FFModuleBaseInfo* R[] = {
    nullptr,
};

static FFModuleBaseInfo* S[] = {
#if !FF_MODULE_DISABLE_SEPARATOR
    &ffSeparatorModuleInfo,
#endif
#if !FF_MODULE_DISABLE_SHELL
    &ffShellModuleInfo,
#endif
#if !FF_MODULE_DISABLE_SOUND
    &ffSoundModuleInfo,
#endif
#if !FF_MODULE_DISABLE_SWAP
    &ffSwapModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* T[] = {
#if !FF_MODULE_DISABLE_TERMINAL
    &ffTerminalModuleInfo,
#endif
#if !FF_MODULE_DISABLE_TERMINALFONT
    &ffTerminalFontModuleInfo,
#endif
#if !FF_MODULE_DISABLE_TERMINALSIZE
    &ffTerminalSizeModuleInfo,
#endif
#if !FF_MODULE_DISABLE_TERMINALTHEME
    &ffTerminalThemeModuleInfo,
#endif
#if !FF_MODULE_DISABLE_TITLE
    &ffTitleModuleInfo,
#endif
#if !FF_MODULE_DISABLE_THEME
    &ffThemeModuleInfo,
#endif
#if !FF_MODULE_DISABLE_TPM
    &ffTPMModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* U[] = {
#if !FF_MODULE_DISABLE_UPTIME
    &ffUptimeModuleInfo,
#endif
#if !FF_MODULE_DISABLE_USERS
    &ffUsersModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* V[] = {
#if !FF_MODULE_DISABLE_VERSION
    &ffVersionModuleInfo,
#endif
#if !FF_MODULE_DISABLE_VULKAN
    &ffVulkanModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* W[] = {
#if !FF_MODULE_DISABLE_WALLPAPER
    &ffWallpaperModuleInfo,
#endif
#if !FF_MODULE_DISABLE_WEATHER
    &ffWeatherModuleInfo,
#endif
#if !FF_MODULE_DISABLE_WM
    &ffWMModuleInfo,
#endif
#if !FF_MODULE_DISABLE_WIFI
    &ffWifiModuleInfo,
#endif
#if !FF_MODULE_DISABLE_WMTHEME
    &ffWMThemeModuleInfo,
#endif
    nullptr,
};

static FFModuleBaseInfo* X[] = {
    nullptr,
};

static FFModuleBaseInfo* Y[] = {
    nullptr,
};

static FFModuleBaseInfo* Z[] = {
#if !FF_MODULE_DISABLE_ZPOOL
    &ffZpoolModuleInfo,
#endif
    nullptr,
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
