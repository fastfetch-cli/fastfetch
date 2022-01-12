#include "fastfetch.h"

#include <string.h>
#include <stdarg.h>

static void testFailed(const FFstrbuf* strbuf, const char* message, ...)
{
    va_list args;
    va_start(args, message);
    fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stderr);
    vfprintf(stderr, message, args);
    fputs(", strbuf: ", stderr);
    ffStrbufWriteTo(strbuf, stderr);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stderr);
    fputc('\n', stderr);
    va_end(args);
    exit(1);
}

int main(int argc, char** argv)
{
    FF_UNUSED(argc, argv)

    FFstrbuf strbuf;
    ffStrbufInitA(&strbuf, 64);

    if(strbuf.allocated != 64)
        testFailed(&strbuf, "strbuf.allocated != 64");

    if(strbuf.length != 0)
        testFailed(&strbuf, "testSrbuf.length != 0");

    ffStrbufAppendS(&strbuf, "123456789");

    if(strbuf.length != 9)
        testFailed(&strbuf, "strbuf.length != 9");

    if(strcmp(strbuf.chars, "123456789") != 0)
        testFailed(&strbuf, "strbuf.data != \"123456789\"");

    if(!ffStrbufStartsWithS(&strbuf, "123"))
        testFailed(&strbuf, "!ffStrbufStartsWithS(&strbuf, \"123\")");

    ffStrbufRemoveS(&strbuf, "78");

    if(strbuf.length != 7)
        testFailed(&strbuf, "strbuf.length != 7");

    if(strcmp(strbuf.chars, "1234569") != 0)
        testFailed(&strbuf, "strcmp(strbuf.chars, \"123469\") != 0");

    ffStrbufRemoveStrings(&strbuf, 3, "23", "45", "9");

    if(strbuf.length != 2)
        testFailed(&strbuf, "strbuf.length != 2");

    if(strcmp(strbuf.chars, "16") != 0)
        testFailed(&strbuf, "strbuf.chars != \"126\"");

    puts("\033[32mAll tests passed!"FASTFETCH_TEXT_MODIFIER_RESET);
}
