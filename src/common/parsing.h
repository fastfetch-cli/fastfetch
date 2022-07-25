#pragma once

#ifndef FF_INCLUDED_parsing_h
#define FF_INCLUDED_parsing_h

#include <stdint.h>

typedef struct FFVersion
{
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} FFVersion;

#define FF_VERSION_INIT ((FFVersion) {0})

bool ffStrSet(const char* str);
void ffParseSemver(FFstrbuf* buffer, const FFstrbuf* major, const FFstrbuf* minor, const FFstrbuf* patch);
void ffParseGTK(FFstrbuf* buffer, const FFstrbuf* gtk2, const FFstrbuf* gtk3, const FFstrbuf* gtk4);

void ffVersionToPretty(const FFVersion* version, FFstrbuf* pretty);
int8_t ffVersionCompare(const FFVersion* version1, const FFVersion* version2);

#endif
