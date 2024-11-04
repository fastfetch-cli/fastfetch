#include "fastfetch.h"

static FFModuleBaseInfo* A[] = {
    NULL,
};

static FFModuleBaseInfo* B[] = {
    (FFModuleBaseInfo*) &instance.config.modules.battery,
    (FFModuleBaseInfo*) &instance.config.modules.bios,
    (FFModuleBaseInfo*) &instance.config.modules.bluetooth,
    (FFModuleBaseInfo*) &instance.config.modules.bluetoothRadio,
    (FFModuleBaseInfo*) &instance.config.modules.board,
    (FFModuleBaseInfo*) &instance.config.modules.bootmgr,
    (FFModuleBaseInfo*) &instance.config.modules.break_,
    (FFModuleBaseInfo*) &instance.config.modules.brightness,
    (FFModuleBaseInfo*) &instance.config.modules.btrfs,
    NULL,
};

static FFModuleBaseInfo* C[] = {
    (FFModuleBaseInfo*) &instance.config.modules.camera,
    (FFModuleBaseInfo*) &instance.config.modules.chassis,
    (FFModuleBaseInfo*) &instance.config.modules.command,
    (FFModuleBaseInfo*) &instance.config.modules.colors,
    (FFModuleBaseInfo*) &instance.config.modules.cpu,
    (FFModuleBaseInfo*) &instance.config.modules.cpuCache,
    (FFModuleBaseInfo*) &instance.config.modules.cpuUsage,
    (FFModuleBaseInfo*) &instance.config.modules.cursor,
    (FFModuleBaseInfo*) &instance.config.modules.custom,
    NULL,
};

static FFModuleBaseInfo* D[] = {
    (FFModuleBaseInfo*) &instance.config.modules.dateTime,
    (FFModuleBaseInfo*) &instance.config.modules.de,
    (FFModuleBaseInfo*) &instance.config.modules.display,
    (FFModuleBaseInfo*) &instance.config.modules.disk,
    (FFModuleBaseInfo*) &instance.config.modules.diskIo,
    (FFModuleBaseInfo*) &instance.config.modules.dns,
    NULL,
};

static FFModuleBaseInfo* E[] = {
    (FFModuleBaseInfo*) &instance.config.modules.editor,
    NULL,
};

static FFModuleBaseInfo* F[] = {
    (FFModuleBaseInfo*) &instance.config.modules.font,
    NULL,
};

static FFModuleBaseInfo* G[] = {
    (FFModuleBaseInfo*) &instance.config.modules.gamepad,
    (FFModuleBaseInfo*) &instance.config.modules.gpu,
    NULL,
};

static FFModuleBaseInfo* H[] = {
    (FFModuleBaseInfo*) &instance.config.modules.host,
    NULL,
};

static FFModuleBaseInfo* I[] = {
    (FFModuleBaseInfo*) &instance.config.modules.icons,
    (FFModuleBaseInfo*) &instance.config.modules.initSystem,
    NULL,
};

static FFModuleBaseInfo* J[] = {
    NULL,
};

static FFModuleBaseInfo* K[] = {
    (FFModuleBaseInfo*) &instance.config.modules.kernel,
    (FFModuleBaseInfo*) &instance.config.modules.keyboard,
    NULL,
};

static FFModuleBaseInfo* L[] = {
    (FFModuleBaseInfo*) &instance.config.modules.lm,
    (FFModuleBaseInfo*) &instance.config.modules.loadavg,
    (FFModuleBaseInfo*) &instance.config.modules.locale,
    (FFModuleBaseInfo*) &instance.config.modules.localIP,
    NULL,
};

static FFModuleBaseInfo* M[] = {
    (FFModuleBaseInfo*) &instance.config.modules.media,
    (FFModuleBaseInfo*) &instance.config.modules.memory,
    (FFModuleBaseInfo*) &instance.config.modules.monitor,
    (FFModuleBaseInfo*) &instance.config.modules.mouse,
    NULL,
};

static FFModuleBaseInfo* N[] = {
    (FFModuleBaseInfo*) &instance.config.modules.netIo,
    NULL,
};

static FFModuleBaseInfo* O[] = {
    (FFModuleBaseInfo*) &instance.config.modules.openCL,
    (FFModuleBaseInfo*) &instance.config.modules.openGL,
    (FFModuleBaseInfo*) &instance.config.modules.os,
    NULL,
};

static FFModuleBaseInfo* P[] = {
    (FFModuleBaseInfo*) &instance.config.modules.packages,
    (FFModuleBaseInfo*) &instance.config.modules.physicalDisk,
    (FFModuleBaseInfo*) &instance.config.modules.physicalMemory,
    (FFModuleBaseInfo*) &instance.config.modules.player,
    (FFModuleBaseInfo*) &instance.config.modules.powerAdapter,
    (FFModuleBaseInfo*) &instance.config.modules.processes,
    (FFModuleBaseInfo*) &instance.config.modules.publicIP,
    NULL,
};

static FFModuleBaseInfo* Q[] = {
    NULL,
};

static FFModuleBaseInfo* R[] = {
    NULL,
};

static FFModuleBaseInfo* S[] = {
    (FFModuleBaseInfo*) &instance.config.modules.separator,
    (FFModuleBaseInfo*) &instance.config.modules.shell,
    (FFModuleBaseInfo*) &instance.config.modules.sound,
    (FFModuleBaseInfo*) &instance.config.modules.swap,
    NULL,
};

static FFModuleBaseInfo* T[] = {
    (FFModuleBaseInfo*) &instance.config.modules.terminal,
    (FFModuleBaseInfo*) &instance.config.modules.terminalFont,
    (FFModuleBaseInfo*) &instance.config.modules.terminalSize,
    (FFModuleBaseInfo*) &instance.config.modules.terminalTheme,
    (FFModuleBaseInfo*) &instance.config.modules.title,
    (FFModuleBaseInfo*) &instance.config.modules.theme,
    (FFModuleBaseInfo*) &instance.config.modules.tpm,
    NULL,
};

static FFModuleBaseInfo* U[] = {
    (FFModuleBaseInfo*) &instance.config.modules.uptime,
    (FFModuleBaseInfo*) &instance.config.modules.users,
    NULL,
};

static FFModuleBaseInfo* V[] = {
    (FFModuleBaseInfo*) &instance.config.modules.version,
    (FFModuleBaseInfo*) &instance.config.modules.vulkan,
    NULL,
};

static FFModuleBaseInfo* W[] = {
    (FFModuleBaseInfo*) &instance.config.modules.wallpaper,
    (FFModuleBaseInfo*) &instance.config.modules.weather,
    (FFModuleBaseInfo*) &instance.config.modules.wm,
    (FFModuleBaseInfo*) &instance.config.modules.wifi,
    (FFModuleBaseInfo*) &instance.config.modules.wmTheme,
    NULL,
};

static FFModuleBaseInfo* X[] = {
    NULL,
};

static FFModuleBaseInfo* Y[] = {
    NULL,
};

static FFModuleBaseInfo* Z[] = {
    (FFModuleBaseInfo*) &instance.config.modules.zpool,
    NULL,
};

FFModuleBaseInfo** ffModuleInfos[] = {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
};
