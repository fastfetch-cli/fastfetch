#pragma once

#include "fastfetch.h"

#define FF_USERS_MODULE_NAME "Users"

void ffPrintUsers(FFinstance* instance, FFUsersOptions* options);
void ffInitUsersOptions(FFUsersOptions* options);
bool ffParseUsersCommandOptions(FFUsersOptions* options, const char* key, const char* value);
void ffDestroyUsersOptions(FFUsersOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseUsersJsonObject(FFinstance* instance, json_object* module);
#endif
