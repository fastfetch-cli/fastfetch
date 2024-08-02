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
    FFstrbuf libEGL;

#ifdef __ANDROID__
    FFstrbuf libfreetype;
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__sun)
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
    FFstrbuf libGLX;
    FFstrbuf libOSMesa;
    FFstrbuf libPulse;
    FFstrbuf libDdcutil;
    FFstrbuf libdrm;
#endif
} FFOptionsLibrary;

const char* ffOptionsParseLibraryJsonConfig(FFOptionsLibrary* options, yyjson_val* root);
bool ffOptionsParseLibraryCommandLine(FFOptionsLibrary* options, const char* key, const char* value);
void ffOptionsInitLibrary(FFOptionsLibrary* options);
void ffOptionsDestroyLibrary(FFOptionsLibrary* options);
void ffOptionsGenerateLibraryJsonConfig(FFOptionsLibrary* options, yyjson_mut_doc* doc);
