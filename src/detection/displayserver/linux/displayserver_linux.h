#pragma once

#ifndef FASTFETCH_INCLUDED_DISPLAYSERVER
#define FASTFETCH_INCLUDED_DISPLAYSERVER

#include "detection/displayserver/displayserver.h"

void ffdsConnectWayland(const FFinstance* instance, FFDisplayServerResult* result);

void ffdsConnectXcbRandr(const FFinstance* instance, FFDisplayServerResult* result);
void ffdsConnectXcb(const FFinstance* instance, FFDisplayServerResult* result);

void ffdsConnectXrandr(const FFinstance* instance, FFDisplayServerResult* result);
void ffdsConnectXlib(const FFinstance* instance, FFDisplayServerResult* result);

void ffdsDetectWMDE(const FFinstance* instance, FFDisplayServerResult* result);

#endif
