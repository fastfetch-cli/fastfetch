#include "fastfetch.h"

void ffParseFormatStringV(FFstrbuf* buffer, FFstrbuf* formatstr, uint32_t numArgs, va_list argp)
{
    FFformatarg arguments[numArgs];
    for(uint32_t i = 0; i < numArgs; i++)
        arguments[i] = (FFformatarg) va_arg(argp, FFformatarg);

    uint32_t argCounter = 1; //First arg is 1 in fomat string

    for(uint32_t i = 0; i < formatstr->length; ++i)
    {
        if(formatstr->chars[i] != '{')
        {
            ffStrbufAppendC(buffer, formatstr->chars[i]);
            continue;
        }

        if(i == formatstr->length - 1)
        {
            ffStrbufAppendC(buffer, '{');
            continue; //This will always stop the loop
        }

        ++i;

        if(formatstr->chars[i] == '{')
        {
            ffStrbufAppendC(buffer, '{');
            continue;
        }

        uint32_t argIndex;

        if(formatstr->chars[i] == '}')
        {
            if(argCounter > numArgs)
            {
                ffStrbufAppendS(buffer, "{}");
                continue;
            }

            argIndex = argCounter++;
        }
        else
        {
            FFstrbuf argnumstr;
            ffStrbufInit(&argnumstr);

            while(formatstr->chars[i] != '}' && i < formatstr->length)
                ffStrbufAppendC(&argnumstr, formatstr->chars[i++]);

            if(
                argnumstr.length == 0 ||
                ffStrbufGetC(&argnumstr, 0) == '-' ||
                sscanf(argnumstr.chars, "%u", &argIndex) != 1 ||
                argIndex > numArgs
            ) {
                ffStrbufAppendC(buffer, '{');
                ffStrbufAppend(buffer, &argnumstr);
                if(formatstr->chars[i] == '}') // We dont have a closing { when ending because whole format string is over
                    ffStrbufAppendC(buffer, '}');
                ffStrbufDestroy(&argnumstr);
                continue;
            }

            ffStrbufDestroy(&argnumstr);
        }

        if(argIndex == 0)
            argIndex = 1;

        FFformatarg arg = arguments[argIndex - 1];

        if(arg.type == FF_FORMAT_ARG_TYPE_INT)
            ffStrbufAppendF(buffer, "%i", *(int*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_UINT)
            ffStrbufAppendF(buffer, "%u", *(uint32_t*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_UINT8)
            ffStrbufAppendF(buffer, "%u", *(uint8_t*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_STRING)
            ffStrbufAppendF(buffer, "%s", (const char*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_STRBUF)
            ffStrbufAppend(buffer, (FFstrbuf*)arg.value);
        else if(arg.type == FF_FORMAT_ARG_TYPE_DOUBLE)
            ffStrbufAppendF(buffer, "%g", *(double*)arg.value);
        else
        {
            fprintf(stderr, "Error: format string \"%s\" argument is not implemented\n", formatstr->chars);
            ffStrbufDestroy(buffer);
            exit(806);
        }
    }

    ffStrbufTrimRight(buffer, ' ');
}

void ffParseFormatString(FFstrbuf* buffer, FFstrbuf* formatstr, uint32_t numArgs, ...)
{
    va_list argp;
    va_start(argp, numArgs);
    ffParseFormatStringV(buffer, formatstr, numArgs, argp);
    va_end(argp);
}

void ffPrintFormatString(FFinstance* instance, FFstrbuf* customKey, const char* defKey, FFstrbuf* formatstr, uint32_t numArgs, ...)
{
    FFstrbuf buffer;
    ffStrbufInitA(&buffer, 256);

    va_list argp;
    va_start(argp, numArgs);

    ffParseFormatStringV(&buffer, formatstr, numArgs, argp);

    va_end(argp);

    if(buffer.length > 0)
    {
        ffPrintLogoAndKey(instance, customKey, defKey);
        ffStrbufPutTo(&buffer, stdout);
    }

    ffStrbufDestroy(&buffer);
}
