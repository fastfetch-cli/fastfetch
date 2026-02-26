#pragma once

#include "common/FFstrbuf.h"

typedef enum __attribute__((__packed__)) FFDataResultDocType
{
    FF_RESULT_DOC_TYPE_DEFAULT = 0,
    FF_RESULT_DOC_TYPE_JSON,
    FF_RESULT_DOC_TYPE_CONFIG,
    FF_RESULT_DOC_TYPE_CONFIG_FULL,
} FFDataResultDocType;

// FFdata aggregates configuration, generation parameters, and output state used by fastfetch.
// It holds the parsed configuration document, a mutable JSON document for results, and related metadata.
typedef struct FFdata
{
    yyjson_doc* configDoc; // Parsed JSON configuration document
    yyjson_mut_doc* resultDoc; // Mutable JSON document for storing results
    FFstrbuf structure; // Custom output structure from command line
    FFstrbuf structureDisabled; // Disabled modules in the output structure from command line
    FFstrbuf genConfigPath; // Path to generate configuration file
    FFDataResultDocType docType; // Type of result document
    bool configLoaded;
} FFdata;
