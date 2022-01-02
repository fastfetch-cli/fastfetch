#pragma once

#ifndef FASTFETCH_INCLUDED_DISPLAYSERVER
#define FASTFETCH_INCLUDED_DISPLAYSERVER

#include "fastfetch.h"

#define FF_DISPLAYSERVER_PROTOCOL_WAYLAND "Wayland"
#define FF_DISPLAYSERVER_PROTOCOL_X11 "X11"
#define FF_DISPLAYSERVER_PROTOCOL_TTY "TTY"

uint32_t ffdsParseRefreshRate(int32_t refreshRate);

void ffdsConnectWayland(const FFinstance* instance, FFDisplayServerResult* result);
void ffdsConnectXrandr(const FFinstance* instance, FFDisplayServerResult* result);
void ffdsConnectXlib(const FFinstance* instance, FFDisplayServerResult* result);

void ffdsDetectWMDE(const FFinstance* instance, FFDisplayServerResult* result);

#endif
