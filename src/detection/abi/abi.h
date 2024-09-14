#pragma once

#include "fastfetch.h"

typedef struct FFABIFeature
{
    const char* name;
    bool supported;
} FFABIFeature;

typedef struct FFABICompat
{
    const char* name;
    const char* desc;
} FFABICompat;

typedef struct FFABIResult
{
    FFlist compats;  // List of FFABICompat
    FFlist features;  // List of FFABIFeature
} FFABIResult;

const char* ffDetectABI(const FFABIOptions* options, FFABIResult* result);

static inline void ffABIAddFeature(
    FFABIResult* result,
    const char* name,
    bool supported)
{
    FFABIFeature* item = (FFABIFeature*) ffListAdd(&result->features);
    *item = (FFABIFeature) {
        .name = name,
        .supported = supported,
    };
}
