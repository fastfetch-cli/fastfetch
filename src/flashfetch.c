#include "fastfetch.h"

#include "modules/modules.h"

int main(void)
{
    ffInitInstance(); //This also applies default configuration to instance.config

    //Modify instance.config here
    FFOptionsModules* const options = &instance.config.modules;

    // ffPrepareCPUUsage();
    // ffPreparePublicIp(&options->publicIP);
    // ffPrepareWeather(&options->weather);

    //Does things like starting detection threads, disabling line wrap, etc
    ffStart();

    //Printing
    void* const modules[] = {
        &options->title,
        &options->separator,
        &options->os,
        &options->host,
        &options->kernel,
        &options->uptime,
        &options->packages,
        &options->shell,
        &options->display,
        &options->de,
        &options->wm,
        &options->wmTheme,
        &options->theme,
        &options->icons,
        &options->font,
        &options->cursor,
        &options->terminal,
        &options->terminalFont,
        &options->cpu,
        &options->gpu,
        &options->memory,
        &options->swap,
        &options->disk,
        &options->localIP,
        &options->battery,
        &options->powerAdapter,
        &options->locale,
        &options->break_,
        &options->colors,
    };

    for (size_t i = 0; i < ARRAY_SIZE(modules); i++)
        ((const FFModuleBaseInfo*) modules[i])->printModule(modules[i]);

    ffFinish();
    ffDestroyInstance();
    return 0;
}
