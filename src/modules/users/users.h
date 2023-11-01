#pragma once

#include "fastfetch.h"

#define FF_USERS_MODULE_NAME "Users"

void ffPrintUsers(FFUsersOptions* options);
void ffInitUsersOptions(FFUsersOptions* options);
void ffDestroyUsersOptions(FFUsersOptions* options);
