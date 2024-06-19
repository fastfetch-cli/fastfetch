#pragma once

#include "fastfetch.h"

typedef struct FFUserResult
{
    FFstrbuf name;
    FFstrbuf hostName;
    FFstrbuf clientIp;
    FFstrbuf sessionName;
    uint64_t loginTime; // ms
} FFUserResult;

const char* ffDetectUsers(FFUsersOptions* options, FFlist* users);
