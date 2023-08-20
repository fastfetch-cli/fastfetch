#include "fastfetch.h"

static FFModuleBaseInfo* A[] = {
    NULL,
};

static FFModuleBaseInfo* B[] = {
    (void*) &instance.config.battery,
    (void*) &instance.config.bios,
    (void*) &instance.config.bluetooth,
    (void*) &instance.config.board,
    (void*) &instance.config.break_,
    (void*) &instance.config.brightness,
    NULL,
};

static FFModuleBaseInfo* C[] = {
    (void*) &instance.config.chassis,
    (void*) &instance.config.command,
    (void*) &instance.config.colors,
    (void*) &instance.config.cpu,
    (void*) &instance.config.cpuUsage,
    (void*) &instance.config.cursor,
    (void*) &instance.config.custom,
    NULL,
};

static FFModuleBaseInfo* D[] = {
    (void*) &instance.config.dateTime,
    (void*) &instance.config.de,
    (void*) &instance.config.display,
    (void*) &instance.config.disk,
    NULL,
};

static FFModuleBaseInfo* E[] = {
    NULL,
};

static FFModuleBaseInfo* F[] = {
    (void*) &instance.config.font,
    NULL,
};

static FFModuleBaseInfo* G[] = {
    (void*) &instance.config.gamepad,
    (void*) &instance.config.gpu,
    NULL,
};

static FFModuleBaseInfo* H[] = {
    (void*) &instance.config.host,
    NULL,
};

static FFModuleBaseInfo* I[] = {
    (void*) &instance.config.icons,
    NULL,
};

static FFModuleBaseInfo* J[] = {
    NULL,
};

static FFModuleBaseInfo* K[] = {
    (void*) &instance.config.kernel,
    NULL,
};

static FFModuleBaseInfo* L[] = {
    (void*) &instance.config.lm,
    (void*) &instance.config.locale,
    (void*) &instance.config.localIP,
    NULL,
};

static FFModuleBaseInfo* M[] = {
    (void*) &instance.config.media,
    (void*) &instance.config.memory,
    (void*) &instance.config.monitor,
    NULL,
};

static FFModuleBaseInfo* N[] = {
    NULL,
};

static FFModuleBaseInfo* O[] = {
    (void*) &instance.config.openCL,
    (void*) &instance.config.openGL,
    (void*) &instance.config.os,
    NULL,
};

static FFModuleBaseInfo* P[] = {
    (void*) &instance.config.packages,
    (void*) &instance.config.player,
    (void*) &instance.config.powerAdapter,
    (void*) &instance.config.processes,
    (void*) &instance.config.publicIP,
    NULL,
};

static FFModuleBaseInfo* Q[] = {
    NULL,
};

static FFModuleBaseInfo* R[] = {
    NULL,
};

static FFModuleBaseInfo* S[] = {
    (void*) &instance.config.separator,
    (void*) &instance.config.shell,
    (void*) &instance.config.sound,
    (void*) &instance.config.swap,
    NULL,
};

static FFModuleBaseInfo* T[] = {
    (void*) &instance.config.terminal,
    (void*) &instance.config.terminalFont,
    (void*) &instance.config.terminalSize,
    (void*) &instance.config.title,
    (void*) &instance.config.theme,
    NULL,
};

static FFModuleBaseInfo* U[] = {
    (void*) &instance.config.uptime,
    (void*) &instance.config.users,
    NULL,
};

static FFModuleBaseInfo* V[] = {
    (void*) &instance.config.vulkan,
    NULL,
};

static FFModuleBaseInfo* W[] = {
    (void*) &instance.config.wallpaper,
    (void*) &instance.config.weather,
    (void*) &instance.config.wm,
    (void*) &instance.config.wifi,
    (void*) &instance.config.wmTheme,
    NULL,
};

static FFModuleBaseInfo* X[] = {
    NULL,
};

static FFModuleBaseInfo* Y[] = {
    NULL,
};

static FFModuleBaseInfo* Z[] = {
    NULL,
};

FFModuleBaseInfo** ffModuleInfos[] = {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
};
