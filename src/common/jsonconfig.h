#pragma once

#include "fastfetch.h"

bool ffJsonConfigParseModuleArgs(yyjson_val* key, yyjson_val* val, FFModuleArgs* moduleArgs);
const char* ffJsonConfigParseEnum(yyjson_val* val, int* result, FFKeyValuePair pairs[]);
void ffPrintJsonConfig(bool prepare, yyjson_mut_doc* jsonDoc);
void ffJsonConfigGenerateModuleArgsConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FFModuleArgs* defaultModuleArgs, FFModuleArgs* moduleArgs);

yyjson_api_inline yyjson_mut_val* yyjson_mut_strbuf(yyjson_mut_doc *doc, const FFstrbuf* buf) {
    return yyjson_mut_strncpy(doc, buf->chars, buf->length);
}

yyjson_api_inline bool yyjson_mut_obj_add_strbuf(yyjson_mut_doc *doc,
                                                  yyjson_mut_val *obj,
                                                  const char *_key,
                                                  const FFstrbuf* buf) {
    return yyjson_mut_obj_add_strncpy(doc, obj, _key, buf->chars, buf->length);
}

yyjson_api_inline bool yyjson_mut_arr_add_strbuf(yyjson_mut_doc *doc,
                                                  yyjson_mut_val *obj,
                                                  const FFstrbuf* buf) {
    return yyjson_mut_arr_add_strncpy(doc, obj, buf->chars, buf->length);
}
