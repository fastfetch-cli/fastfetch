#pragma once

#ifndef FASTFETCH_INCLUDED_DISPLAYSERVER
#define FASTFETCH_INCLUDED_DISPLAYSERVER

#include "detection/displayserver/displayserver.h"

#define FF_DISPLAYSERVER_PROTOCOL_WAYLAND "Wayland"
#define FF_DISPLAYSERVER_PROTOCOL_X11 "X11"
#define FF_DISPLAYSERVER_PROTOCOL_TTY "TTY"

void ffdsConnectWayland(const FFinstance* instance, FFDisplayServerResult* result);

void ffdsConnectXcbRandr(const FFinstance* instance, FFDisplayServerResult* result);
void ffdsConnectXcb(const FFinstance* instance, FFDisplayServerResult* result);

void ffdsConnectXrandr(const FFinstance* instance, FFDisplayServerResult* result);
void ffdsConnectXlib(const FFinstance* instance, FFDisplayServerResult* result);

void ffdsDetectWMDE(const FFinstance* instance, FFDisplayServerResult* result);

#endif
