#pragma once

#ifndef FASTFETCH_INCLUDED_FFSTRBUF
#define FASTFETCH_INCLUDED_FFSTRBUF

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define FASTFETCH_STRBUF_DEFAULT_ALLOC 32

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

uint32_t ffStrbufGetFree(const FFstrbuf* strbuf);

void ffStrbufClear(FFstrbuf* strbuf);

void ffStrbufSet(FFstrbuf* strbuf, const FFstrbuf* value);
void ffStrbufSetS(FFstrbuf* strbuf, const char* value);
void ffStrbufSetNS(FFstrbuf* strbuf, uint32_t length, const char* value);
void ffStrbufSetC(FFstrbuf* strbuf, const char c);
void ffStrbufSetF(FFstrbuf* strbuf, const char* format, ...);
void ffStrbufSetVF(FFstrbuf* strbuf, const char* format, va_list arguments);

void ffStrbufAppend(FFstrbuf* strbuf, const FFstrbuf* value);
void ffStrbufAppendTransformS(FFstrbuf* strbuf, const char* value, int(*transformFunc)(int));
void ffStrbufAppendSExcludingC(FFstrbuf* strbuf, const char* value, char exclude);
void ffStrbufAppendNSExludingC(FFstrbuf* strbuf, uint32_t length, const char* value, char exclude);
void ffStrbufAppendS(FFstrbuf* strbuf, const char* value);
void ffStrbufAppendNS(FFstrbuf* strbuf, uint32_t length, const char* value);
void ffStrbufAppendC(FFstrbuf* strbuf, const char c);
void ffStrbufAppendF(FFstrbuf* strbuf, const char* format, ...);
void ffStrbufAppendVF(FFstrbuf* strbuf, const char* format, va_list arguments);

char ffStrbufGetC(FFstrbuf* strbuf, uint32_t index);

void ffStrbufWriteTo(const FFstrbuf* strbuf, FILE* file);
void ffStrbufPutTo(const FFstrbuf* strbuf, FILE* file);

int ffStrbufComp(const FFstrbuf* strbuf, const FFstrbuf* comp);
int ffStrbufCompS(const FFstrbuf* strbuf, const char* comp);

int ffStrbufIgnCaseComp(const FFstrbuf* strbuf, const FFstrbuf* comp);
int ffStrbufIgnCaseCompS(const FFstrbuf* strbuf, const char* comp);

void ffStrbufTrimLeft(FFstrbuf* strbuf, char c);
void ffStrbufTrimRight(FFstrbuf* strbuf, char c);
void ffStrbufTrim(FFstrbuf* strbuf, char c);

void ffStrbufRemoveStringsA(FFstrbuf* strbuf, uint32_t numStrings, const char* strings[]);
void ffStrbufRemoveStringsV(FFstrbuf* strbuf, uint32_t numStrings, va_list arguments);
void ffStrbufRemoveStrings(FFstrbuf* strbuf, uint32_t numStrings, ...);

uint32_t ffStrbufNextIndexC(const FFstrbuf* strbuf, uint32_t start, char c);
uint32_t ffStrbufNextIndex(const FFstrbuf* strbuf, uint32_t start, const FFstrbuf* searched);
uint32_t ffStrbufNextIndexS(const FFstrbuf* strbuf, uint32_t start, const char* str);

uint32_t ffStrbufFirstIndexC(const FFstrbuf* strbuf, char c);
uint32_t ffStrbufFirstIndex(const FFstrbuf* strbuf, const FFstrbuf* searched);
uint32_t ffStrbufFirstIndexS(const FFstrbuf* strbuf, const char* str);

uint32_t ffStrbufPreviousIndexC(const FFstrbuf* strbuf, uint32_t start, char c);
uint32_t ffStrbufLastIndexC(const FFstrbuf* strbuf, char c);

void ffStrbufSubstrBefore(FFstrbuf* strbuf, uint32_t index);
void ffStrbufSubstrBeforeFirstC(FFstrbuf* strbuf, const char c);
void ffStrbufSubstrBeforeLastC(FFstrbuf* strbuf, const char c);
void ffStrbufSubstrAfter(FFstrbuf* strbuf, uint32_t index);
void ffStrbufSubstrAfterFirstC(FFstrbuf* strbuf, const char c);
void ffStrbufSubstrAfterLastC(FFstrbuf* strbuf, const char c);

bool ffStrbufStartsWith(const FFstrbuf* strbuf, const FFstrbuf* start);
bool ffStrbufStartsWithS(const FFstrbuf* strbuf, const char* start);
bool ffStrbufStartsWithNS(const FFstrbuf* strbuf, uint32_t length, const char* start);

bool ffStrbufStartsWithIgnCase(const FFstrbuf* strbuf, const FFstrbuf* start);
bool ffStrbufStartsWithIgnCaseS(const FFstrbuf* strbuf, const char* start);
bool ffStrbufStartsWithIgnCaseNS(const FFstrbuf* strbuf, uint32_t length, const char* start);

bool ffStrbufEndsWithC(const FFstrbuf* strbuf, char c);
bool ffStrbufEndsWithS(const FFstrbuf* strbuf, const char* end);

bool ffStrbufEndsWithIgnCaseS(const FFstrbuf* strbuf, const char* end);

bool ffStrbufRemoveIgnCaseEndS(FFstrbuf* strbuf, const char* end);

void ffStrbufRecalculateLength(FFstrbuf* strbuf);

void ffStrbufDestroy(FFstrbuf* strbuf);

#endif
