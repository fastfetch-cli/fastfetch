#pragma once

#include "fastfetch.h"
#include "modules/security/option.h"

typedef struct FFSecurityResult
{
    FFstrbuf tpmStatus;
    FFstrbuf secureBootStatus;
} FFSecurityResult;

const char* ffDetectSecurity(FFSecurityResult* result);
