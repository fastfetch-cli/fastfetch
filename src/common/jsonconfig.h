#pragma once

#include "common/ffdata.h"
#include "common/option.h"

// `key` / `val` may be null when the JSON object has no such member; `moduleArgs` is always real
[[gnu::nonnull(3)]] bool ffJsonConfigParseModuleArgs(yyjson_val* key, yyjson_val* val, FFModuleArgs* moduleArgs);
// `val` may be null (yyjson predicates tolerate it); `result` and `pairs` may not.
// Returns an error string, null on success, so it is `nodiscard`.
[[gnu::nonnull(2, 3), nodiscard]] const char* ffJsonConfigParseEnum(yyjson_val* val, int* result, FFKeyValuePair pairs[]);

// The three helpers below are routinely called as statements, to add a key to a document, so their
// result is not `nodiscard`.
[[gnu::nonnull(1, 2)]] yyjson_api_inline yyjson_mut_val* yyjson_mut_strbuf(yyjson_mut_doc* doc, const FFstrbuf* buf) {
    return yyjson_mut_strncpy(doc, buf->chars, buf->length);
}

[[gnu::nonnull(1, 2, 3, 4)]] yyjson_api_inline bool yyjson_mut_obj_add_strbuf(yyjson_mut_doc* doc,
    yyjson_mut_val* obj,
    const char* _key,
    const FFstrbuf* buf) {
    return yyjson_mut_obj_add_strncpy(doc, obj, _key, buf->chars, buf->length);
}

[[gnu::nonnull(1, 2, 3)]] yyjson_api_inline bool yyjson_mut_arr_add_strbuf(yyjson_mut_doc* doc,
    yyjson_mut_val* obj,
    const FFstrbuf* buf) {
    return yyjson_mut_arr_add_strncpy(doc, obj, buf->chars, buf->length);
}

[[gnu::nonnull(1)]] void ffPrintJsonConfig(FFdata* data, bool prepare);
[[gnu::nonnull(1, 2, 3)]] void ffJsonConfigGenerateModuleArgsConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FFModuleArgs* moduleArgs);
