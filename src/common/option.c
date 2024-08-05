#include "fastfetch.h"
#include "common/option.h"
#include "common/color.h"
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

    if(subKey[0] == '\0')
        return subKey;

    if(subKey[0] != '-')
        return NULL;

    subKey += 1;

    return subKey;
}

bool ffOptionParseModuleArgs(const char* argumentKey, const char* subKey, const char* value, FFModuleArgs* result)
{
    if(ffStrEqualsIgnCase(subKey, "key"))
    {
        ffOptionParseString(argumentKey, value, &result->key);
        return true;
    }
    else if(ffStrEqualsIgnCase(subKey, "format"))
    {
        ffOptionParseString(argumentKey, value, &result->outputFormat);
        return true;
    }
    else if(ffStrEqualsIgnCase(subKey, "output-color"))
    {
        if(value == NULL)
        {
            fprintf(stderr, "Error: usage: %s <str>\n", argumentKey);
            exit(477);
        }
        ffOptionParseColor(value, &result->outputColor);
        return true;
    }
    else if(ffStrStartsWithIgnCase(subKey, "key-"))
    {
        const char* subKey2 = subKey + strlen("key-");
        if(ffStrEqualsIgnCase(subKey2, "color"))
        {
            if(value == NULL)
            {
                fprintf(stderr, "Error: usage: %s <str>\n", argumentKey);
                exit(477);
            }
            ffOptionParseColor(value, &result->keyColor);
            return true;
        }
        else if(ffStrEqualsIgnCase(subKey2, "width"))
        {
            result->keyWidth = ffOptionParseUInt32(argumentKey, value);
            return true;
        }
        else if(ffStrEqualsIgnCase(subKey2, "icon"))
        {
            ffOptionParseString(argumentKey, value, &result->keyIcon);
            return true;
        }
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

int32_t ffOptionParseInt32(const char* argumentKey, const char* value)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <num>\n", argumentKey);
        exit(480);
    }

    char* end;
    int32_t num = (int32_t) strtol(value, &end, 10);
    if(*end != '\0')
    {
        fprintf(stderr, "Error: usage: %s <num>\n", argumentKey);
        exit(479);
    }

    return num;
}

int ffOptionParseEnum(const char* argumentKey, const char* requestedKey, FFKeyValuePair pairs[])
{
    if(requestedKey == NULL)
    {
        fprintf(stderr, "Error: usage: %s <value>\n", argumentKey);
        exit(476);
    }

    for (const FFKeyValuePair* pPair = pairs; pPair->key; ++pPair)
    {
        if(ffStrEqualsIgnCase(requestedKey, pPair->key))
            return pPair->value;
    }

    fprintf(stderr, "Error: unknown %s value: %s\n", argumentKey, requestedKey);
    exit(478);
}

bool ffOptionParseBoolean(const char* str)
{
    return (
        !ffStrSet(str) ||
        ffStrEqualsIgnCase(str, "true") ||
        ffStrEqualsIgnCase(str, "yes")  ||
        ffStrEqualsIgnCase(str, "on")   ||
        ffStrEqualsIgnCase(str, "1")
    );
}

void ffOptionParseColorNoClear(const char* value, FFstrbuf* buffer)
{
    // If value is already an ANSI escape code, use it
    if (value[0] == '\e' && value[1] == '[')
    {
        ffStrbufAppendS(buffer, value + 2);
        ffStrbufTrimRight(buffer, 'm');
        return;
    }

    ffStrbufEnsureFree(buffer, 63);

    while(*value != '\0')
    {
        #define FF_APPEND_COLOR_CODE_COND(prefix, code) \
            if(ffStrStartsWithIgnCase(value, #prefix)) { ffStrbufAppendS(buffer, code); value += strlen(#prefix); continue; }
        #define FF_APPEND_COLOR_PROP_COND(prefix, prop) \
            if(ffStrStartsWithIgnCase(value, #prefix)) { if (instance.config.display.prop.length) ffStrbufAppend(buffer, &instance.config.display.prop); else ffStrbufAppendS(buffer, FF_COLOR_FG_DEFAULT); value += strlen(#prefix); continue; }

        if (ffCharIsEnglishAlphabet(value[0]))
        {
            FF_APPEND_COLOR_CODE_COND(reset_, FF_COLOR_MODE_RESET)
            else FF_APPEND_COLOR_CODE_COND(bright_, FF_COLOR_MODE_BOLD)
            else FF_APPEND_COLOR_CODE_COND(dim_, FF_COLOR_MODE_DIM)
            else FF_APPEND_COLOR_CODE_COND(italic_, FF_COLOR_MODE_ITALIC)
            else FF_APPEND_COLOR_CODE_COND(underline_, FF_COLOR_MODE_UNDERLINE)
            else FF_APPEND_COLOR_CODE_COND(blink_, FF_COLOR_MODE_BLINK)
            else FF_APPEND_COLOR_CODE_COND(inverse_, FF_COLOR_MODE_INVERSE)
            else FF_APPEND_COLOR_CODE_COND(hidden_, FF_COLOR_MODE_HIDDEN)
            else FF_APPEND_COLOR_CODE_COND(strike_, FF_COLOR_MODE_STRIKETHROUGH)
            else FF_APPEND_COLOR_CODE_COND(black, FF_COLOR_FG_BLACK)
            else FF_APPEND_COLOR_CODE_COND(red, FF_COLOR_FG_RED)
            else FF_APPEND_COLOR_CODE_COND(green, FF_COLOR_FG_GREEN)
            else FF_APPEND_COLOR_CODE_COND(yellow, FF_COLOR_FG_YELLOW)
            else FF_APPEND_COLOR_CODE_COND(blue, FF_COLOR_FG_BLUE)
            else FF_APPEND_COLOR_CODE_COND(magenta, FF_COLOR_FG_MAGENTA)
            else FF_APPEND_COLOR_CODE_COND(cyan, FF_COLOR_FG_CYAN)
            else FF_APPEND_COLOR_CODE_COND(white, FF_COLOR_FG_WHITE)
            else FF_APPEND_COLOR_CODE_COND(default, FF_COLOR_FG_DEFAULT)
            else FF_APPEND_COLOR_CODE_COND(light_black, FF_COLOR_FG_LIGHT_BLACK)
            else FF_APPEND_COLOR_CODE_COND(light_red, FF_COLOR_FG_LIGHT_RED)
            else FF_APPEND_COLOR_CODE_COND(light_green, FF_COLOR_FG_LIGHT_GREEN)
            else FF_APPEND_COLOR_CODE_COND(light_yellow, FF_COLOR_FG_LIGHT_YELLOW)
            else FF_APPEND_COLOR_CODE_COND(light_blue, FF_COLOR_FG_LIGHT_BLUE)
            else FF_APPEND_COLOR_CODE_COND(light_magenta, FF_COLOR_FG_LIGHT_MAGENTA)
            else FF_APPEND_COLOR_CODE_COND(light_cyan, FF_COLOR_FG_LIGHT_CYAN)
            else FF_APPEND_COLOR_CODE_COND(light_white, FF_COLOR_FG_LIGHT_WHITE)
            else FF_APPEND_COLOR_PROP_COND(keys, colorKeys)
            else FF_APPEND_COLOR_PROP_COND(title, colorTitle)
            else FF_APPEND_COLOR_PROP_COND(output, colorOutput)
            else FF_APPEND_COLOR_PROP_COND(separator, colorSeparator)
            else
            {
                fprintf(stderr, "Error: invalid color code found: %s\n", value);
                exit(479);
            }
        }
        ffStrbufAppendC(buffer, *value);
        ++value;

        #undef FF_APPEND_COLOR_CODE_COND
        #undef FF_APPEND_COLOR_PROP_COND
    }
}
