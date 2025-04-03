#pragma once

#include "fastfetch.h"

/**
 * Extracts string literals from a binary file
 *
 * @param file Path to the binary file to extract strings from
 * @param cb Callback function that will be called for each string found
 *           Return false from callback to stop extraction
 * @param userdata User-provided data passed to the callback function
 * @param minLength Minimum length of strings to extract
 *
 * @return NULL on success, error message on failure.
 * @note This function won't return an error if no strings are found.
 *       Always check if strings are correctly extracted after this function all.
 */
const char* ffBinaryExtractStrings(const char* file, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength);
