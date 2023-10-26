#pragma once

#include "util/FFstrbuf.h"

typedef struct FFOptionsLibrary
{
    FFstrbuf libPCI;
    FFstrbuf libVulkan;
    FFstrbuf libWayland;
    FFstrbuf libXcbRandr;
    FFstrbuf libXcb;
    FFstrbuf libXrandr;
    FFstrbuf libX11;
    FFstrbuf libGIO;
    FFstrbuf libDConf;
    FFstrbuf libDBus;
    FFstrbuf libXFConf;
    FFstrbuf libSQLite3;
    FFstrbuf librpm;
    FFstrbuf libImageMagick;
    FFstrbuf libZ;
    FFstrbuf libChafa;
    FFstrbuf libEGL;
    FFstrbuf libGLX;
    FFstrbuf libOSMesa;
    FFstrbuf libOpenCL;
    FFstrbuf libfreetype;
    FFstrbuf libPulse;
    FFstrbuf libnm;
    FFstrbuf libDdcutil;
} FFOptionsLibrary;

const char* ffOptionsParseLibraryJsonConfig(FFOptionsLibrary* options, yyjson_val* root);
bool ffOptionsParseLibraryCommandLine(FFOptionsLibrary* options, const char* key, const char* value);
void ffOptionsInitLibrary(FFOptionsLibrary* options);
void ffOptionsDestroyLibrary(FFOptionsLibrary* options);
void ffOptionsGenerateLibraryJsonConfig(FFOptionsLibrary* options, yyjson_mut_doc* doc);
