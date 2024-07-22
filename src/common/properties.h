#pragma once

#include "fastfetch.h"

typedef struct FFpropquery
{
    const char* start;
    FFstrbuf* buffer;
} FFpropquery;

bool ffParsePropLines(const char* lines, const char* start, FFstrbuf* buffer);
bool ffParsePropFileValues(const char* filename, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileHomeValues(const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
bool ffParsePropFileListValues(const FFlist* list, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);

bool ffParsePropLinePointer(const char** line, const char* start, FFstrbuf* buffer);

static inline bool ffParsePropLine(const char* line, const char* start, FFstrbuf* buffer)
{
    return ffParsePropLinePointer(&line, start, buffer);
}

static inline bool ffParsePropFile(const char* filename, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileValues(filename, 1, (FFpropquery[]){{start, buffer}});
}

static inline bool ffParsePropFileHome(const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileHomeValues(relativeFile, 1, (FFpropquery[]){{start, buffer}});
}

static inline bool ffParsePropFileList(const FFlist* list, const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileListValues(list, relativeFile, 1, (FFpropquery[]){{start, buffer}});
}

static inline bool ffParsePropFileConfigValues(const char* relativeFile, uint32_t numQueries, FFpropquery* queries)
{
    return ffParsePropFileListValues(&instance.state.platform.configDirs, relativeFile, numQueries, queries);
}

static inline bool ffParsePropFileConfig(const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileConfigValues(relativeFile, 1, (FFpropquery[]){{start, buffer}});
}

static inline bool ffParsePropFileDataValues(const char* relativeFile, uint32_t numQueries, FFpropquery* queries)
{
    return ffParsePropFileListValues(&instance.state.platform.dataDirs, relativeFile, numQueries, queries);
}

static inline bool ffParsePropFileData(const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileDataValues(relativeFile, 1, (FFpropquery[]){{start, buffer}});
}
