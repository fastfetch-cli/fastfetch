#pragma once

#include "common/FFstrbuf.h"

#include <stdint.h>

typedef struct FFVersion {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} FFVersion;

typedef struct FFColorRangeConfig {
    uint8_t green;
    uint8_t yellow;
} FFColorRangeConfig;

#define FF_VERSION_INIT ((FFVersion) { 0 })

// Every argument is dereferenced unconditionally
[[gnu::nonnull(1, 2, 3, 4)]] void ffParseSemver(FFstrbuf* buffer, const FFstrbuf* major, const FFstrbuf* minor, const FFstrbuf* patch);
[[gnu::nonnull(1, 2, 3, 4)]] void ffParseGTK(FFstrbuf* buffer, const FFstrbuf* gtk2, const FFstrbuf* gtk3, const FFstrbuf* gtk4);

[[gnu::nonnull(1, 2)]] void ffVersionToPretty(const FFVersion* version, FFstrbuf* pretty);
[[gnu::nonnull(1, 2), gnu::pure, nodiscard]] int8_t ffVersionCompare(const FFVersion* version1, const FFVersion* version2);
