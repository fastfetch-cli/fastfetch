#pragma once

#ifndef FASTFETCH_INCLUDED_FFSTRBUF
#define FASTFETCH_INCLUDED_FFSTRBUF

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define FASTFETCH_STRBUF_DEFAULT_ALLOC 64

typedef struct FFstrbuf
{
    uint32_t allocated;
    uint32_t length;
    char* chars;
} FFstrbuf;

#define FF_STRBUF_CREATE(name) FFstrbuf name; ffStrbufInit(&name);

void ffStrbufInit(FFstrbuf* strbuf);
void ffStrbufInitCopy(FFstrbuf* strbuf, const FFstrbuf* src);
void ffStrbufInitS(FFstrbuf* strbuf, const char* value);
void ffStrbufInitNS(FFstrbuf* strbuf, uint32_t length, const char* value);
void ffStrbufInitC(FFstrbuf* strbuf, const char c);
void ffStrbufInitA(FFstrbuf* strbuf, uint32_t allocate);
void ffStrbufInitAS(FFstrbuf* strbuf, uint32_t allocate, const char* value);
void ffStrbufInitANS(FFstrbuf* strbuf, uint32_t allocate, uint32_t length, const char* value);
void ffStrbufInitAC(FFstrbuf* strbuf, uint32_t allocate, const char c);
void ffStrbufInitF(FFstrbuf* strbuf, const char* format, ...);
void ffStrbufInitVF(FFstrbuf* strbuf, const char* format, va_list arguments);

void ffStrbufEnsureCapacity(FFstrbuf* strbuf, uint32_t allocate);
void ffStrbufEnsureFree(FFstrbuf* strbuf, uint32_t free);

void ffStrbufClear(FFstrbuf* strbuf);
bool ffStrbufIsEmpty(FFstrbuf* strbuf);

void ffStrbufSet(FFstrbuf* strbuf, const FFstrbuf* value);
void ffStrbufSetS(FFstrbuf* strbuf, const char* value);
void ffStrbufSetNS(FFstrbuf* strbuf, uint32_t length, const char* value);
void ffStrbufSetC(FFstrbuf* strbuf, const char c);
void ffStrbufSetF(FFstrbuf* strbuf, const char* format, ...);
void ffStrbufSetVF(FFstrbuf* strbuf, const char* format, va_list arguments);

void ffStrbufAppend(FFstrbuf* strbuf, const FFstrbuf* value);
void ffStrbufAppendS(FFstrbuf* strbuf, const char* value);
void ffStrbufAppendNS(FFstrbuf* strbuf, uint32_t length, const char* value);
void ffStrbufAppendC(FFstrbuf* strbuf, const char c);
void ffStrbufAppendF(FFstrbuf* strbuf, const char* format, ...);
void ffStrbufAppendVF(FFstrbuf* strbuf, const char* format, va_list arguments);

char ffStrbufGetC(FFstrbuf* strbuf, uint32_t index);

void ffStrbufWriteTo(FFstrbuf* strbuf, FILE* file);
void ffStrbufPutTo(FFstrbuf* strbuf, FILE* file);

int ffStrbufComp(FFstrbuf* strbuf, const FFstrbuf* comp);
int ffStrbufCompS(FFstrbuf* strbuf, const char* comp);
int ffStrbufCompNS(FFstrbuf* strbuf, uint32_t length, const char* comp);
int ffStrbufIgnCaseComp(FFstrbuf* strbuf, const FFstrbuf* comp);
int ffStrbufIgnCaseCompS(FFstrbuf* strbuf, const char* comp);
int ffStrbufIgnCaseCompNS(FFstrbuf* strbuf, uint32_t length, const char* comp);

void ffStrbufTrimLeft(FFstrbuf* strbuf, char c);
void ffStrbufTrimRight(FFstrbuf* strbuf, char c);
void ffStrbufTrim(FFstrbuf* strbuf, char c);

void ffStrbufDestroy(FFstrbuf* strbuf);

#endif
