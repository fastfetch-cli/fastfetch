#pragma once

#include "option.h"

bool ffPrintUsers(FFUsersOptions* options);
void ffInitUsersOptions(FFUsersOptions* options);
void ffDestroyUsersOptions(FFUsersOptions* options);

extern FFModuleBaseInfo ffUsersModuleInfo;
