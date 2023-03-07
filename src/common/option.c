#include "common/option.h"
#include "util/stringUtils.h"

// Return start position of the inner key if the argument key belongs to the module specified, NULL otherwise
const char* ffOptionTestPrefix(const char* argumentKey, const char* moduleName)
{
    const char* subKey = argumentKey;
    if(!(subKey[0] == '-' && subKey[1] == '-'))
        return NULL;

    subKey += 2;
    uint32_t moduleNameLen = (uint32_t)strlen(moduleName);
    if(strncasecmp(subKey, moduleName, moduleNameLen) != 0)
        return NULL;

    subKey += moduleNameLen;
    if(subKey[0] != '-')
        return NULL;

    subKey += 1;

    return subKey;
}

bool ffOptionParseModuleArgs(const char* argumentKey, const char* subKey, const char* value, FFModuleArgs* result)
{
    if(strcasecmp(subKey, "key") == 0)
    {
        ffOptionParseString(argumentKey, value, &result->key);
        return true;
    }
    else if(strcasecmp(subKey, "format") == 0)
    {
        ffOptionParseString(argumentKey, value, &result->outputFormat);
        return true;
    }
    else if(strcasecmp(subKey, "error") == 0)
    {
        ffOptionParseString(argumentKey, value, &result->errorFormat);
        return true;
    }
    return false;
}

void ffOptionParseString(const char* argumentKey, const char* value, FFstrbuf* buffer)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <str>\n", argumentKey);
        exit(477);
    }

    ffStrbufSetS(buffer, value);
}

uint32_t ffOptionParseUInt32(const char* argumentKey, const char* value)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <num>\n", argumentKey);
        exit(480);
    }

    char* end;
    uint32_t num = (uint32_t) strtoul(value, &end, 10);
    if(*end != '\0')
    {
        fprintf(stderr, "Error: usage: %s <num>\n", argumentKey);
        exit(479);
    }

    return num;
}

void ffOptionParseEnum(const char* argumentKey, const char* requestedKey, void* result, ...)
{
    if(requestedKey == NULL)
    {
        fprintf(stderr, "Error: usage: %s <value>\n", argumentKey);
        exit(476);
    }

    va_list args;
    va_start(args, result);

    while(true)
    {
        const char* key = va_arg(args, const char*);
        if(key == NULL)
            break;

        int value = va_arg(args, int); //C standard guarantees that enumeration constants are presented as ints

        if(strcasecmp(requestedKey, key) == 0)
        {
            *(int*)result = value;
            va_end(args);
            return;
        }
    }

    va_end(args);

    fprintf(stderr, "Error: unknown %s value: %s\n", argumentKey, requestedKey);
    exit(478);
}

bool ffOptionParseBoolean(const char* str)
{
    return (
        !ffStrSet(str) ||
        strcasecmp(str, "true") == 0 ||
        strcasecmp(str, "yes")  == 0 ||
        strcasecmp(str, "on")   == 0 ||
        strcasecmp(str, "1")    == 0
    );
}

void ffOptionInitModuleArg(FFModuleArgs* args)
{
    ffStrbufInit(&args->key);
    ffStrbufInit(&args->outputFormat);
    ffStrbufInit(&args->errorFormat);
}

void ffOptionDestroyModuleArg(FFModuleArgs* args)
{
    ffStrbufDestroy(&args->key);
    ffStrbufDestroy(&args->outputFormat);
    ffStrbufDestroy(&args->errorFormat);
}
