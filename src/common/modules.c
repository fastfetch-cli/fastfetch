#include "fastfetch.h"

static FFModuleBaseInfo* A[] = {
    NULL,
};

static FFModuleBaseInfo* B[] = {
    (void*) &instance.config.modules.battery,
    (void*) &instance.config.modules.bios,
    (void*) &instance.config.modules.bluetooth,
    (void*) &instance.config.modules.bluetoothRadio,
    (void*) &instance.config.modules.board,
    (void*) &instance.config.modules.bootmgr,
    (void*) &instance.config.modules.break_,
    (void*) &instance.config.modules.brightness,
    (void*) &instance.config.modules.btrfs,
    NULL,
};

static FFModuleBaseInfo* C[] = {
    (void*) &instance.config.modules.camera,
    (void*) &instance.config.modules.chassis,
    (void*) &instance.config.modules.command,
    (void*) &instance.config.modules.colors,
    (void*) &instance.config.modules.cpu,
    (void*) &instance.config.modules.cpuCache,
    (void*) &instance.config.modules.cpuUsage,
    (void*) &instance.config.modules.cursor,
    (void*) &instance.config.modules.custom,
    NULL,
};

static FFModuleBaseInfo* D[] = {
    (void*) &instance.config.modules.dateTime,
    (void*) &instance.config.modules.de,
    (void*) &instance.config.modules.display,
    (void*) &instance.config.modules.disk,
    (void*) &instance.config.modules.diskIo,
    (void*) &instance.config.modules.dns,
    NULL,
};

static FFModuleBaseInfo* E[] = {
    (void*) &instance.config.modules.editor,
    NULL,
};

static FFModuleBaseInfo* F[] = {
    (void*) &instance.config.modules.font,
    NULL,
};

static FFModuleBaseInfo* G[] = {
    (void*) &instance.config.modules.gamepad,
    (void*) &instance.config.modules.gpu,
    NULL,
};

static FFModuleBaseInfo* H[] = {
    (void*) &instance.config.modules.host,
    NULL,
};

static FFModuleBaseInfo* I[] = {
    (void*) &instance.config.modules.icons,
    (void*) &instance.config.modules.initSystem,
    NULL,
};

static FFModuleBaseInfo* J[] = {
    NULL,
};

static FFModuleBaseInfo* K[] = {
    (void*) &instance.config.modules.kernel,
    (void*) &instance.config.modules.keyboard,
    NULL,
};

static FFModuleBaseInfo* L[] = {
    (void*) &instance.config.modules.lm,
    (void*) &instance.config.modules.loadavg,
    (void*) &instance.config.modules.locale,
    (void*) &instance.config.modules.localIP,
    NULL,
};

static FFModuleBaseInfo* M[] = {
    (void*) &instance.config.modules.media,
    (void*) &instance.config.modules.memory,
    (void*) &instance.config.modules.monitor,
    (void*) &instance.config.modules.mouse,
    NULL,
};

static FFModuleBaseInfo* N[] = {
    (void*) &instance.config.modules.netIo,
    NULL,
};

static FFModuleBaseInfo* O[] = {
    (void*) &instance.config.modules.openCL,
    (void*) &instance.config.modules.openGL,
    (void*) &instance.config.modules.os,
    NULL,
};

static FFModuleBaseInfo* P[] = {
    (void*) &instance.config.modules.packages,
    (void*) &instance.config.modules.physicalDisk,
    (void*) &instance.config.modules.physicalMemory,
    (void*) &instance.config.modules.player,
    (void*) &instance.config.modules.powerAdapter,
    (void*) &instance.config.modules.processes,
    (void*) &instance.config.modules.publicIP,
    NULL,
};

static FFModuleBaseInfo* Q[] = {
    NULL,
};

static FFModuleBaseInfo* R[] = {
    NULL,
};

static FFModuleBaseInfo* S[] = {
    (void*) &instance.config.modules.separator,
    (void*) &instance.config.modules.shell,
    (void*) &instance.config.modules.sound,
    (void*) &instance.config.modules.swap,
    NULL,
};

static FFModuleBaseInfo* T[] = {
    (void*) &instance.config.modules.terminal,
    (void*) &instance.config.modules.terminalFont,
    (void*) &instance.config.modules.terminalSize,
    (void*) &instance.config.modules.terminalTheme,
    (void*) &instance.config.modules.title,
    (void*) &instance.config.modules.theme,
    (void*) &instance.config.modules.tpm,
    NULL,
};

static FFModuleBaseInfo* U[] = {
    (void*) &instance.config.modules.uptime,
    (void*) &instance.config.modules.users,
    NULL,
};

static FFModuleBaseInfo* V[] = {
    (void*) &instance.config.modules.version,
    (void*) &instance.config.modules.vulkan,
    NULL,
};

static FFModuleBaseInfo* W[] = {
    (void*) &instance.config.modules.wallpaper,
    (void*) &instance.config.modules.weather,
    (void*) &instance.config.modules.wm,
    (void*) &instance.config.modules.wifi,
    (void*) &instance.config.modules.wmTheme,
    NULL,
};

static FFModuleBaseInfo* X[] = {
    NULL,
};

static FFModuleBaseInfo* Y[] = {
    NULL,
};

static FFModuleBaseInfo* Z[] = {
    (void*) &instance.config.modules.zpool,
    NULL,
};

FFModuleBaseInfo** ffModuleInfos[] = {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
};
