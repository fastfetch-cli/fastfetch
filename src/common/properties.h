#pragma once

#ifndef FF_INCLUDED_common_properties
#define FF_INCLUDED_common_properties

#include "fastfetch.h"

typedef struct FFpropquery
{
    const char* start;
    FFstrbuf* buffer;
} FFpropquery;

bool ffParsePropLine(const char* line, const char* start, FFstrbuf* buffer);
bool ffParsePropLines(const char* lines, const char* start, FFstrbuf* buffer);
bool ffParsePropFileValues(const char* filename, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileHomeValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileListValues(const FFlist* list, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);

static inline bool ffParsePropFile(const char* filename, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileValues(filename, 1, (FFpropquery[]){{start, buffer}});
}

static inline bool ffParsePropFileHome(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileHomeValues(instance, relativeFile, 1, (FFpropquery[]){{start, buffer}});
}

static inline bool ffParsePropFileList(const FFlist* list, const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileListValues(list, relativeFile, 1, (FFpropquery[]){{start, buffer}});
}

static inline bool ffParsePropFileConfigValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries)
{
    return ffParsePropFileListValues(&instance->state.configDirs, relativeFile, numQueries, queries);
}

static inline bool ffParsePropFileConfig(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileConfigValues(instance, relativeFile, 1, (FFpropquery[]){{start, buffer}});
}

static inline bool ffParsePropFileDataValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries)
{
    return ffParsePropFileListValues(&instance->state.dataDirs, relativeFile, numQueries, queries);
}

static inline bool ffParsePropFileData(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileDataValues(instance, relativeFile, 1, (FFpropquery[]){{start, buffer}});
}

#endif
