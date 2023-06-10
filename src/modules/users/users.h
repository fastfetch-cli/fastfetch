#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_USERS_MODULE_NAME "Users"

void ffPrintUsers(FFinstance* instance, FFUsersOptions* options);
void ffInitUsersOptions(FFUsersOptions* options);
bool ffParseUsersCommandOptions(FFUsersOptions* options, const char* key, const char* value);
void ffDestroyUsersOptions(FFUsersOptions* options);
void ffParseUsersJsonObject(FFinstance* instance, yyjson_val* module);
