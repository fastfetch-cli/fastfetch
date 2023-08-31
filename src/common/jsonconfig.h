#pragma once

#include "fastfetch.h"

bool ffJsonConfigParseModuleArgs(const char* key, yyjson_val* val, FFModuleArgs* moduleArgs);
const char* ffJsonConfigParseEnum(yyjson_val* val, int* result, FFKeyValuePair pairs[]);
void ffPrintJsonConfig(bool prepare);
const char* ffParseGeneralJsonConfig(void);
const char* ffParseDisplayJsonConfig(void);
const char* ffParseLibraryJsonConfig(void);

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

yyjson_api_inline yyjson_mut_val* yyjson_mut_obj_add_obj(yyjson_mut_doc *doc,
                                                          yyjson_mut_val *obj,
                                                          const char *_key) {
    yyjson_mut_val* newObj = yyjson_mut_obj(doc);
    yyjson_mut_obj_add_val(doc, obj, _key, newObj);
    return newObj;
}

yyjson_api_inline yyjson_mut_val* yyjson_mut_obj_add_arr(yyjson_mut_doc *doc,
                                                          yyjson_mut_val *obj,
                                                          const char *_key) {
    yyjson_mut_val* newArr = yyjson_mut_arr(doc);
    yyjson_mut_obj_add_val(doc, obj, _key, newArr);
    return newArr;
}
