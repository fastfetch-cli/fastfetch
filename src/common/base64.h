#pragma once

#include "fastfetch.h"

void ffBase64EncodeRaw(uint32_t size, const char *str, uint32_t *out_size, char *output);
static inline FFstrbuf ffBase64EncodeStrbuf(const FFstrbuf* in)
{
    FFstrbuf out = ffStrbufCreateA(10 + in->length * 4 / 3);
    ffBase64EncodeRaw(in->length, in->chars, &out.length, out.chars);
    assert(out.length < out.allocated);

    return out;
}

bool ffBase64DecodeRaw(uint32_t size, const char *str, uint32_t *out_size, char *output);
static inline FFstrbuf ffBase64DecodeStrbuf(const FFstrbuf* in)
{
    FFstrbuf out = ffStrbufCreateA(10 + in->length * 3 / 4);
    ffBase64DecodeRaw(in->length, in->chars, &out.length, out.chars);
    assert(out.length < out.allocated);

    return out;
}
