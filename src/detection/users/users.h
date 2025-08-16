#pragma once

#include "fastfetch.h"
#include "modules/users/option.h"

typedef struct FFUserResult
{
    FFstrbuf name;
    FFstrbuf hostName;
    FFstrbuf clientIp;
    FFstrbuf sessionName;
    uint64_t loginTime; // ms
} FFUserResult;

const char* ffDetectUsers(FFUsersOptions* options, FFlist* users);
