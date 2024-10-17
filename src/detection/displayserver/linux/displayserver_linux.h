#pragma once

#include "detection/displayserver/displayserver.h"

const char* ffdsConnectWayland(FFDisplayServerResult* result);

const char* ffdsConnectXcbRandr(FFDisplayServerResult* result);
const char* ffdsConnectXcb(FFDisplayServerResult* result);

const char* ffdsConnectXrandr(FFDisplayServerResult* result);
const char* ffdsConnectXlib(FFDisplayServerResult* result);

const char* ffdsConnectDrm(FFDisplayServerResult* result);

void ffdsDetectWMDE(FFDisplayServerResult* result);

FFDisplayType ffdsGetDisplayType(const char* drmConnectorName);
