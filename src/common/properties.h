#pragma once

#include "fastfetch.h"

typedef struct FFpropquery {
    const char* start;
    FFstrbuf* buffer;
} FFpropquery;

// These report "was the file/property found", which callers routinely discard, so they are not
// `nodiscard`.
//
// `queries` is only walked when `numQueries > 0`, so it is intentionally not `nonnull(3)`.
[[gnu::nonnull(1, 2, 3)]] bool ffParsePropLines(const char* lines, const char* start, FFstrbuf* buffer);
[[gnu::nonnull(1)]] bool ffParsePropFileValues(const char* filename, uint32_t numQueries, FFpropquery* queries);
[[gnu::nonnull(1)]] bool ffParsePropFileHomeValues(const char* relativeFile, uint32_t numQueries, FFpropquery* queries);
[[gnu::nonnull(1, 2)]] bool ffParsePropFileListValues(const FFlist* list, const char* relativeFile, uint32_t numQueries, FFpropquery* queries);

[[gnu::nonnull(1, 2, 3)]] bool ffParsePropLinePointer(const char** line, const char* start, FFstrbuf* buffer);

[[gnu::nonnull(1, 2, 3)]] static inline bool ffParsePropLine(const char* line, const char* start, FFstrbuf* buffer) {
    return ffParsePropLinePointer(&line, start, buffer);
}

[[gnu::nonnull(1, 2, 3)]] static inline bool ffParsePropFile(const char* filename, const char* start, FFstrbuf* buffer) {
    return ffParsePropFileValues(filename, 1, (FFpropquery[]) { { start, buffer } });
}

[[gnu::nonnull(1, 2, 3)]] static inline bool ffParsePropFileHome(const char* relativeFile, const char* start, FFstrbuf* buffer) {
    return ffParsePropFileHomeValues(relativeFile, 1, (FFpropquery[]) { { start, buffer } });
}

[[gnu::nonnull(1, 2, 3, 4)]] static inline bool ffParsePropFileList(const FFlist* list, const char* relativeFile, const char* start, FFstrbuf* buffer) {
    return ffParsePropFileListValues(list, relativeFile, 1, (FFpropquery[]) { { start, buffer } });
}

[[gnu::nonnull(1)]] static inline bool ffParsePropFileConfigValues(const char* relativeFile, uint32_t numQueries, FFpropquery* queries) {
    return ffParsePropFileListValues(&instance.state.platform.configDirs, relativeFile, numQueries, queries);
}

[[gnu::nonnull(1, 2, 3)]] static inline bool ffParsePropFileConfig(const char* relativeFile, const char* start, FFstrbuf* buffer) {
    return ffParsePropFileConfigValues(relativeFile, 1, (FFpropquery[]) { { start, buffer } });
}

[[gnu::nonnull(1)]] static inline bool ffParsePropFileDataValues(const char* relativeFile, uint32_t numQueries, FFpropquery* queries) {
    return ffParsePropFileListValues(&instance.state.platform.dataDirs, relativeFile, numQueries, queries);
}

[[gnu::nonnull(1, 2, 3)]] static inline bool ffParsePropFileData(const char* relativeFile, const char* start, FFstrbuf* buffer) {
    return ffParsePropFileDataValues(relativeFile, 1, (FFpropquery[]) { { start, buffer } });
}
