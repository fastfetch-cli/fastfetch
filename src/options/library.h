#pragma once

#include "util/FFstrbuf.h"

typedef struct FFOptionsLibrary
{
    FFstrbuf libVulkan;
    FFstrbuf libOpenCL;
    FFstrbuf libSQLite3;
    FFstrbuf libImageMagick;
    FFstrbuf libChafa;
    FFstrbuf libZ;

#if defined(__linux__) || defined(__FreeBSD__)
    FFstrbuf libWayland;
    FFstrbuf libXcbRandr;
    FFstrbuf libXcb;
    FFstrbuf libXrandr;
    FFstrbuf libX11;
    FFstrbuf libGIO;
    FFstrbuf libDConf;
    FFstrbuf libDBus;
    FFstrbuf libXFConf;
    FFstrbuf librpm;
    FFstrbuf libEGL;
    FFstrbuf libGLX;
    FFstrbuf libOSMesa;
    FFstrbuf libfreetype;
    FFstrbuf libPulse;
    FFstrbuf libnm;
    FFstrbuf libDdcutil;
    FFstrbuf libdrm;
#endif
} FFOptionsLibrary;

const char* ffOptionsParseLibraryJsonConfig(FFOptionsLibrary* options, yyjson_val* root);
bool ffOptionsParseLibraryCommandLine(FFOptionsLibrary* options, const char* key, const char* value);
void ffOptionsInitLibrary(FFOptionsLibrary* options);
void ffOptionsDestroyLibrary(FFOptionsLibrary* options);
void ffOptionsGenerateLibraryJsonConfig(FFOptionsLibrary* options, yyjson_mut_doc* doc);
