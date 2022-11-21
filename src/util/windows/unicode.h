#pragma once

#ifndef FASTFETCH_INCLUDED_UNICODE_H
#define FASTFETCH_INCLUDED_UNICODE_H

#include "fastfetch.h"

void ffWcharToUtf8(const wchar_t* input, FFstrbuf* result);

#endif
