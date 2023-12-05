#pragma once

#ifndef FASTFETCH_INCLUDED_DISPLAYSERVER
#define FASTFETCH_INCLUDED_DISPLAYSERVER

#include "detection/displayserver/displayserver.h"

void ffdsConnectWayland(FFDisplayServerResult* result);

void ffdsConnectXcbRandr(FFDisplayServerResult* result);
void ffdsConnectXcb(FFDisplayServerResult* result);

void ffdsConnectXrandr(FFDisplayServerResult* result);
void ffdsConnectXlib(FFDisplayServerResult* result);

void ffdsConnectDrm(FFDisplayServerResult* result);

void ffdsDetectWMDE(FFDisplayServerResult* result);

#endif
