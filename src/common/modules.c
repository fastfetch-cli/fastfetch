#include "modules/modules.h"

static FFModuleBaseInfo* A[] = {
    NULL,
};

static FFModuleBaseInfo* B[] = {
    &ffBatteryModuleInfo,
    &ffBiosModuleInfo,
    &ffBluetoothModuleInfo,
    &ffBluetoothRadioModuleInfo,
    &ffBoardModuleInfo,
    &ffBootmgrModuleInfo,
    &ffBreakModuleInfo,
    &ffBrightnessModuleInfo,
    &ffBtrfsModuleInfo,
    NULL,
};

static FFModuleBaseInfo* C[] = {
    &ffCameraModuleInfo,
    &ffChassisModuleInfo,
    &ffCommandModuleInfo,
    &ffColorsModuleInfo,
    &ffCPUModuleInfo,
    &ffCPUCacheModuleInfo,
    &ffCPUUsageModuleInfo,
    &ffCursorModuleInfo,
    &ffCustomModuleInfo,
    NULL,
};

static FFModuleBaseInfo* D[] = {
    &ffDateTimeModuleInfo,
    &ffDEModuleInfo,
    &ffDisplayModuleInfo,
    &ffDiskModuleInfo,
    &ffDiskIOModuleInfo,
    &ffDNSModuleInfo,
    NULL,
};

static FFModuleBaseInfo* E[] = {
    &ffEditorModuleInfo,
    NULL,
};

static FFModuleBaseInfo* F[] = {
    &ffFontModuleInfo,
    NULL,
};

static FFModuleBaseInfo* G[] = {
    &ffGamepadModuleInfo,
    &ffGPUModuleInfo,
    NULL,
};

static FFModuleBaseInfo* H[] = {
    &ffHostModuleInfo,
    NULL,
};

static FFModuleBaseInfo* I[] = {
    &ffIconsModuleInfo,
    &ffInitSystemModuleInfo,
    NULL,
};

static FFModuleBaseInfo* J[] = {
    NULL,
};

static FFModuleBaseInfo* K[] = {
    &ffKernelModuleInfo,
    &ffKeyboardModuleInfo,
    NULL,
};

static FFModuleBaseInfo* L[] = {
    &ffLMModuleInfo,
    &ffLoadavgModuleInfo,
    &ffLocaleModuleInfo,
    &ffLocalIPModuleInfo,
    NULL,
};

static FFModuleBaseInfo* M[] = {
    &ffMediaModuleInfo,
    &ffMemoryModuleInfo,
    &ffMonitorModuleInfo,
    &ffMouseModuleInfo,
    NULL,
};

static FFModuleBaseInfo* N[] = {
    &ffNetIOModuleInfo,
    NULL,
};

static FFModuleBaseInfo* O[] = {
    &ffOpenCLModuleInfo,
    &ffOpenGLModuleInfo,
    &ffOSModuleInfo,
    NULL,
};

static FFModuleBaseInfo* P[] = {
    &ffPackagesModuleInfo,
    &ffPhysicalDiskModuleInfo,
    &ffPhysicalMemoryModuleInfo,
    &ffPlayerModuleInfo,
    &ffPowerAdapterModuleInfo,
    &ffProcessesModuleInfo,
    &ffPublicIPModuleInfo,
    NULL,
};

static FFModuleBaseInfo* Q[] = {
    NULL,
};

static FFModuleBaseInfo* R[] = {
    NULL,
};

static FFModuleBaseInfo* S[] = {
    &ffSeparatorModuleInfo,
    &ffShellModuleInfo,
    &ffSoundModuleInfo,
    &ffSwapModuleInfo,
    NULL,
};

static FFModuleBaseInfo* T[] = {
    &ffTerminalModuleInfo,
    &ffTerminalFontModuleInfo,
    &ffTerminalSizeModuleInfo,
    &ffTerminalThemeModuleInfo,
    &ffTitleModuleInfo,
    &ffThemeModuleInfo,
    &ffTPMModuleInfo,
    NULL,
};

static FFModuleBaseInfo* U[] = {
    &ffUptimeModuleInfo,
    &ffUsersModuleInfo,
    NULL,
};

static FFModuleBaseInfo* V[] = {
    &ffVersionModuleInfo,
    &ffVulkanModuleInfo,
    NULL,
};

static FFModuleBaseInfo* W[] = {
    &ffWallpaperModuleInfo,
    &ffWeatherModuleInfo,
    &ffWMModuleInfo,
    &ffWifiModuleInfo,
    &ffWMThemeModuleInfo,
    NULL,
};

static FFModuleBaseInfo* X[] = {
    NULL,
};

static FFModuleBaseInfo* Y[] = {
    NULL,
};

static FFModuleBaseInfo* Z[] = {
    &ffZpoolModuleInfo,
    NULL,
};

FFModuleBaseInfo** ffModuleInfos[] = {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
};
