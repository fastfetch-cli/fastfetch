#pragma once

#ifndef FASTFETCH_INCLUDED_LOGO_logo
#define FASTFETCH_INCLUDED_LOGO_logo

#include "fastfetch.h"

//logo.c
void ffLogoPrint(FFinstance* instance, const char* data, bool doColorReplacement);

//builtin.c
void ffLogoSetMainColor(FFinstance* instance);

void ffLogoPrintUnknown(FFinstance* instance);
void ffLogoPrintBuiltin(FFinstance* instance);
bool ffLogoPrintBuiltinIfExists(FFinstance* instance);
void ffLogoPrintBuiltinDetected(FFinstance* instance);

//sixel/image.c
bool ffLogoPrintImageIfExists(FFinstance* instance, FFLogoType type);

#endif
