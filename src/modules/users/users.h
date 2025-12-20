#pragma once

#include "option.h"

#define FF_USERS_MODULE_NAME "Users"

bool ffPrintUsers(FFUsersOptions* options);
void ffInitUsersOptions(FFUsersOptions* options);
void ffDestroyUsersOptions(FFUsersOptions* options);

extern FFModuleBaseInfo ffUsersModuleInfo;
