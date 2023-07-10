#pragma once

#ifndef FF_INCLUDED_common_processing
#define FF_INCLUDED_common_processing

#include "util/FFstrbuf.h"

const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[]);
const char* ffProcessAppendStdErr(FFstrbuf* buffer, char* const argv[]);

#endif
