#pragma once

#ifndef FASTFETCH_INCLUDED_LOGO_logo
#define FASTFETCH_INCLUDED_LOGO_logo

#include "fastfetch.h"

//logo.c
void ffLogoPrint(FFinstance* instance, const char* data, bool doColorReplacement);

//builtin.c
void ffLogoSetMainColor(FFinstance* instance);

void ffLogoPrintUnknown(FFinstance* instance);
bool ffLogoPrintBuiltinIfExists(FFinstance* instance);
void ffLogoPrintBuiltinDetected(FFinstance* instance);

typedef enum FFLogoImageResult
{
    FF_LOGO_IMAGE_RESULT_SUCCESS,    //Logo printed
    FF_LOGO_IMAGE_RESULT_INIT_ERROR, //Failed to load library, try again with next IM version
    FF_LOGO_IMAGE_RESULT_RUN_ERROR   //Failed to load / convert image, cancle whole sixel code
} FFLogoImageResult;

//image/image.c
FFLogoImageResult ffLogoPrintImageIfExists(FFinstance* instance, FFLogoType type);

#endif
