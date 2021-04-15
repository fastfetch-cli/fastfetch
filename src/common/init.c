#include "fastfetch.h"

#include <unistd.h>

static void initState(FFstate* state)
{
    state->current_row = 0;
    state->passwd = getpwuid(getuid());
    uname(&state->utsname);
    sysinfo(&state->sysinfo);
}

static void defaultConfig(FFconfig* config)
{
    ffStrbufInit(&config->color);
    config->logo_spacing = 4;
    ffStrbufInitS(&config->seperator, ": ");
    config->offsetx = 0;
    config->titleLength = 20; // This is overwritten by ffPrintTitle
    config->colorLogo = true;
    config->showErrors = false;
    config->recache = false;
    config->cacheSave = true;
    config->printRemainingLogo = true;

    //Since most of these properties are unlikely to be used at once, give them minimal heap space (the \0 character)
    ffStrbufInitA(&config->osFormat, 1);
    ffStrbufInitA(&config->osKey, 1);
    ffStrbufInitA(&config->hostFormat, 1);
    ffStrbufInitA(&config->hostKey, 1);
    ffStrbufInitA(&config->kernelFormat, 1);
    ffStrbufInitA(&config->kernelKey, 1);
    ffStrbufInitA(&config->uptimeFormat, 1);
    ffStrbufInitA(&config->uptimeKey, 1);
    ffStrbufInitA(&config->packagesFormat, 1);
    ffStrbufInitA(&config->packagesKey, 1);
    ffStrbufInitA(&config->shellFormat, 1);
    ffStrbufInitA(&config->shellKey, 1);
    ffStrbufInitA(&config->resolutionFormat, 1);
    ffStrbufInitA(&config->resolutionKey, 1);
    ffStrbufInitA(&config->deFormat, 1);
    ffStrbufInitA(&config->deKey, 1);
    ffStrbufInitA(&config->wmFormat, 1);
    ffStrbufInitA(&config->wmKey, 1);
    ffStrbufInitA(&config->wmThemeFormat, 1);
    ffStrbufInitA(&config->wmThemeKey, 1);
    ffStrbufInitA(&config->themeFormat, 1);
    ffStrbufInitA(&config->themeKey, 1);
    ffStrbufInitA(&config->iconsFormat, 1);
    ffStrbufInitA(&config->iconsKey, 1);
    ffStrbufInitA(&config->fontFormat, 1);
    ffStrbufInitA(&config->fontKey, 1);
    ffStrbufInitA(&config->terminalFormat, 1);
    ffStrbufInitA(&config->terminalKey, 1);
    ffStrbufInitA(&config->termFontFormat, 1);
    ffStrbufInitA(&config->termFontKey, 1);
    ffStrbufInitA(&config->cpuFormat, 1);
    ffStrbufInitA(&config->cpuKey, 1);
    ffStrbufInitA(&config->gpuFormat, 1);
    ffStrbufInitA(&config->gpuKey, 1);
    ffStrbufInitA(&config->memoryFormat, 1);
    ffStrbufInitA(&config->memoryKey, 1);
    ffStrbufInitA(&config->diskFormat, 1);
    ffStrbufInitA(&config->diskKey, 1);
    ffStrbufInitA(&config->batteryFormat, 1);
    ffStrbufInitA(&config->batteryKey, 1);
    ffStrbufInitA(&config->localeFormat, 1);
    ffStrbufInitA(&config->localeKey, 1);

    ffStrbufInitA(&config->libPCI, 1);
    ffStrbufInitA(&config->libX11, 1);
    ffStrbufInitA(&config->libXrandr, 1);
    ffStrbufInitA(&config->libDConf, 1);

    ffStrbufInitA(&config->diskFolders, 1);
}

void ffInitInstance(FFinstance* instance)
{
    initState(&instance->state);
    defaultConfig(&instance->config);
    ffCacheValidate(instance);
}

static void ffCleanup(FFinstance* instance)
{
    UNUSED(instance);
    // Place for cleaning up
    // I dont destroy the global strbufs, because the OS is typically faster doing it after the program terminates
    // This eliminates one function call per strbuf + setting things like length to 0
}

void ffFinish(FFinstance* instance)
{
    if(instance->config.printRemainingLogo)
        ffPrintRemainingLogo(instance);

    ffCleanup(instance);
}
