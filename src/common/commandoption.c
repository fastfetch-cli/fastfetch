#include "commandoption.h"
#include "util/stringUtils.h"

#include <ctype.h>

static inline bool tryModule(const char* type, void* options)
{
    FFModuleBaseInfo* baseInfo = (FFModuleBaseInfo*) options;
    if (ffStrEqualsIgnCase(type, baseInfo->name))
    {
        baseInfo->printModule(options);
        return true;
    }
    return false;
}

bool ffParseModuleCommand(const char* type)
{
    FFconfig* cfg = &instance.config;
    switch (toupper(type[0]))
    {
        case 'B': {
            return
                tryModule(type, &cfg->battery) ||
                tryModule(type, &cfg->bios) ||
                tryModule(type, &cfg->bluetooth) ||
                tryModule(type, &cfg->board) ||
                tryModule(type, &cfg->break_) ||
                tryModule(type, &cfg->brightness) ||
                false;
        }

        case 'C': {
            return
                tryModule(type, &cfg->chassis) ||
                tryModule(type, &cfg->command) ||
                tryModule(type, &cfg->colors) ||
                tryModule(type, &cfg->cpu) ||
                tryModule(type, &cfg->cpuUsage) ||
                tryModule(type, &cfg->cursor) ||
                tryModule(type, &cfg->custom) ||
                false;
        }

        case 'D': {
            return
                tryModule(type, &cfg->dateTime) ||
                tryModule(type, &cfg->de) ||
                tryModule(type, &cfg->display) ||
                tryModule(type, &cfg->disk) ||
                false;
        }

        case 'F': {
            return
                tryModule(type, &cfg->font) ||
                false;
        }

        case 'G': {
            return
                tryModule(type, &cfg->gamepad) ||
                tryModule(type, &cfg->gpu) ||
                false;
        }

        case 'H': {
            return
                tryModule(type, &cfg->host) ||
                false;
        }

        case 'I': {
            return
                tryModule(type, &cfg->icons) ||
                false;
        }

        case 'K': {
            return
                tryModule(type, &cfg->kernel) ||
                false;
        }

        case 'L': {
            return
                tryModule(type, &cfg->lm) ||
                tryModule(type, &cfg->locale) ||
                tryModule(type, &cfg->localIP) ||
                false;
        }

        case 'M': {
            return
                tryModule(type, &cfg->media) ||
                tryModule(type, &cfg->memory) ||
                tryModule(type, &cfg->monitor) ||
                false;
        }

        case 'O': {
            return
                tryModule(type, &cfg->openCL) ||
                tryModule(type, &cfg->openGL) ||
                tryModule(type, &cfg->os) ||
                false;
        }

        case 'P': {
            return
                tryModule(type, &cfg->packages) ||
                tryModule(type, &cfg->player) ||
                tryModule(type, &cfg->powerAdapter) ||
                tryModule(type, &cfg->processes) ||
                tryModule(type, &cfg->publicIP) ||
                false;
        }

        case 'S': {
            return
                tryModule(type, &cfg->separator) ||
                tryModule(type, &cfg->shell) ||
                tryModule(type, &cfg->sound) ||
                tryModule(type, &cfg->swap) ||
                false;
        }

        case 'T': {
            return
                tryModule(type, &cfg->terminal) ||
                tryModule(type, &cfg->terminalFont) ||
                tryModule(type, &cfg->terminalSize) ||
                tryModule(type, &cfg->title) ||
                tryModule(type, &cfg->theme) ||
                false;
        }

        case 'U': {
            return
                tryModule(type, &cfg->uptime) ||
                tryModule(type, &cfg->users) ||
                false;
        }

        case 'V': {
            return
                tryModule(type, &cfg->vulkan) ||
                false;
        }

        case 'W': {
            return
                tryModule(type, &cfg->wallpaper) ||
                tryModule(type, &cfg->weather) ||
                tryModule(type, &cfg->wm) ||
                tryModule(type, &cfg->wifi) ||
                tryModule(type, &cfg->wmTheme) ||
                false;
        }

        default:
            return false;
    }
}

static inline bool tryModuleCommandOptions(const char* key, const char* value, void* options)
{
    FFModuleBaseInfo* baseInfo = (FFModuleBaseInfo*) options;
    return baseInfo->parseCommandOptions(options, key, value);
}

bool ffParseModuleOptions(const char* key, const char* value)
{
    if (!ffStrStartsWith(key, "--")) return false;
    FFconfig* cfg = &instance.config;

    switch (toupper(key[2]))
    {
        case 'B': {
            return
                tryModuleCommandOptions(key, value, &cfg->battery) ||
                tryModuleCommandOptions(key, value, &cfg->bios) ||
                tryModuleCommandOptions(key, value, &cfg->bluetooth) ||
                tryModuleCommandOptions(key, value, &cfg->board) ||
                tryModuleCommandOptions(key, value, &cfg->break_) ||
                tryModuleCommandOptions(key, value, &cfg->brightness) ||
                false;
        }

        case 'C': {
            return
                tryModuleCommandOptions(key, value, &cfg->chassis) ||
                tryModuleCommandOptions(key, value, &cfg->command) ||
                tryModuleCommandOptions(key, value, &cfg->colors) ||
                tryModuleCommandOptions(key, value, &cfg->cpu) ||
                tryModuleCommandOptions(key, value, &cfg->cpuUsage) ||
                tryModuleCommandOptions(key, value, &cfg->cursor) ||
                tryModuleCommandOptions(key, value, &cfg->custom) ||
                false;
        }

        case 'D': {
            return
                tryModuleCommandOptions(key, value, &cfg->dateTime) ||
                tryModuleCommandOptions(key, value, &cfg->de) ||
                tryModuleCommandOptions(key, value, &cfg->display) ||
                tryModuleCommandOptions(key, value, &cfg->disk) ||
                false;
        }

        case 'F': {
            return
                tryModuleCommandOptions(key, value, &cfg->font) ||
                false;
        }

        case 'G': {
            return
                tryModuleCommandOptions(key, value, &cfg->gamepad) ||
                tryModuleCommandOptions(key, value, &cfg->gpu) ||
                false;
        }

        case 'H': {
            return
                tryModuleCommandOptions(key, value, &cfg->host) ||
                false;
        }

        case 'I': {
            return
                tryModuleCommandOptions(key, value, &cfg->icons) ||
                false;
        }

        case 'K': {
            return
                tryModuleCommandOptions(key, value, &cfg->kernel) ||
                false;
        }

        case 'L': {
            return
                tryModuleCommandOptions(key, value, &cfg->lm) ||
                tryModuleCommandOptions(key, value, &cfg->locale) ||
                tryModuleCommandOptions(key, value, &cfg->localIP) ||
                false;
        }

        case 'M': {
            return
                tryModuleCommandOptions(key, value, &cfg->media) ||
                tryModuleCommandOptions(key, value, &cfg->memory) ||
                tryModuleCommandOptions(key, value, &cfg->monitor) ||
                false;
        }

        case 'O': {
            return
                tryModuleCommandOptions(key, value, &cfg->openCL) ||
                tryModuleCommandOptions(key, value, &cfg->openGL) ||
                tryModuleCommandOptions(key, value, &cfg->os) ||
                false;
        }

        case 'P': {
            return
                tryModuleCommandOptions(key, value, &cfg->packages) ||
                tryModuleCommandOptions(key, value, &cfg->player) ||
                tryModuleCommandOptions(key, value, &cfg->powerAdapter) ||
                tryModuleCommandOptions(key, value, &cfg->processes) ||
                tryModuleCommandOptions(key, value, &cfg->publicIP) ||
                false;
        }

        case 'S': {
            return
                tryModuleCommandOptions(key, value, &cfg->separator) ||
                tryModuleCommandOptions(key, value, &cfg->shell) ||
                tryModuleCommandOptions(key, value, &cfg->sound) ||
                tryModuleCommandOptions(key, value, &cfg->swap) ||
                false;
        }

        case 'T': {
            return
                tryModuleCommandOptions(key, value, &cfg->terminal) ||
                tryModuleCommandOptions(key, value, &cfg->terminalFont) ||
                tryModuleCommandOptions(key, value, &cfg->terminalSize) ||
                tryModuleCommandOptions(key, value, &cfg->title) ||
                tryModuleCommandOptions(key, value, &cfg->theme) ||
                false;
        }

        case 'U': {
            return
                tryModuleCommandOptions(key, value, &cfg->uptime) ||
                tryModuleCommandOptions(key, value, &cfg->users) ||
                false;
        }

        case 'V': {
            return
                tryModuleCommandOptions(key, value, &cfg->vulkan) ||
                false;
        }

        case 'W': {
            return
                tryModuleCommandOptions(key, value, &cfg->wallpaper) ||
                tryModuleCommandOptions(key, value, &cfg->weather) ||
                tryModuleCommandOptions(key, value, &cfg->wm) ||
                tryModuleCommandOptions(key, value, &cfg->wifi) ||
                tryModuleCommandOptions(key, value, &cfg->wmTheme) ||
                false;
        }

        default:
            return false;
    }
}
