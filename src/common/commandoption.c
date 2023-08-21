#include "commandoption.h"
#include "util/stringUtils.h"

#include <ctype.h>

bool ffParseModuleCommand(const char* type)
{
    if(!isalpha(type[0])) return false;

    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(type[0]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        if (ffStrEqualsIgnCase(type, baseInfo->name))
        {
            baseInfo->printModule(baseInfo);
            return true;
        }
    }
    return false;
}

bool ffParseModuleOptions(const char* key, const char* value)
{
    if (!ffStrStartsWith(key, "--") || !isalpha(key[2])) return false;

    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(key[2]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        if (baseInfo->parseCommandOptions(baseInfo, key, value)) return true;
    }
    return false;
}
