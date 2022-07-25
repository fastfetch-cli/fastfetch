#pragma once

#ifndef FF_INCLUDED_common_properties
#define FF_INCLUDED_common_properties

typedef struct FFpropquery
{
    const char* start;
    FFstrbuf* buffer;
} FFpropquery;

bool ffParsePropLine(const char* line, const char* start, FFstrbuf* buffer);
bool ffParsePropLines(const char* lines, const char* start, FFstrbuf* buffer);
bool ffParsePropFileValues(const char* filename, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFile(const char* filename, const char* start, FFstrbuf* buffer);
bool ffParsePropFileHomeValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileHome(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer);
bool ffParsePropFileConfigValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileConfig(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer);

#endif
