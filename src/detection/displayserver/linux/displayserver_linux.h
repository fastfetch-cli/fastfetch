#pragma once

#ifndef FASTFETCH_INCLUDED_DISPLAYSERVER
#define FASTFETCH_INCLUDED_DISPLAYSERVER

#include "detection/displayserver/displayserver.h"

bool ffdsMatchDrmConnector(const char* connName, FFstrbuf* edidName);

const char* ffdsConnectWayland(FFDisplayServerResult* result);

const char* ffdsConnectXcbRandr(FFDisplayServerResult* result);
const char* ffdsConnectXcb(FFDisplayServerResult* result);

const char* ffdsConnectXrandr(FFDisplayServerResult* result);
const char* ffdsConnectXlib(FFDisplayServerResult* result);

const char* ffdsConnectDrm(FFDisplayServerResult* result);

void ffdsDetectWMDE(FFDisplayServerResult* result);

FFDisplayType ffdsGetDisplayType(const char* drmConnectorName);

#endif
