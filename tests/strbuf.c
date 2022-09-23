#include "fastfetch.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

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

    //destroy 0
    ffStrbufInitA(&strbuf, 0);
    ffStrbufDestroy(&strbuf);

    //initA

    ffStrbufInitA(&strbuf, 0);

    if(strbuf.chars[0] != 0) //make sure chars[0] is accessable
        testFailed(&strbuf, "strbuf.chars[0] != 0");

    if(strbuf.allocated != 0)
        testFailed(&strbuf, "strbuf.allocated != 0");

    if(strbuf.length != 0)
        testFailed(&strbuf, "strbuf.length != 0");

    //appendS

    ffStrbufAppendS(&strbuf, "12345");

    if(strbuf.length != 5)
        testFailed(&strbuf, "strbuf.length != 5");

    if(strbuf.allocated < 6)
        testFailed(&strbuf, "strbuf.allocated < 6");

    if(ffStrbufCompS(&strbuf, "12345") != 0)
        testFailed(&strbuf, "strbuf.data != \"12345\"");

    //appendNS

    ffStrbufAppendNS(&strbuf, 4, "67890");

    if(strbuf.length != 9)
        testFailed(&strbuf, "strbuf.length != 9");

    if(strbuf.allocated < 10)
        testFailed(&strbuf, "strbuf.allocated < 10");

    if(ffStrbufCompS(&strbuf, "123456789") != 0)
        testFailed(&strbuf, "strbuf.data != \"123456789\"");

    //appendS long

    ffStrbufAppendS(&strbuf, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");

    if(strbuf.length != 109)
        testFailed(&strbuf, "strbuf.length != 109");

    if(strbuf.allocated < 110)
        testFailed(&strbuf, "strbuf.allocated < 110");

    if(strbuf.chars[strbuf.length] != 0)
        testFailed(&strbuf, "strbuf.chars[strbuf.length] != 0");

    strbuf.length = 9;

    //startsWithS

    if(!ffStrbufStartsWithS(&strbuf, "123"))
        testFailed(&strbuf, "!ffStrbufStartsWithS(&strbuf, \"123\")");

    //removeS

    ffStrbufRemoveS(&strbuf, "78");

    if(strbuf.length != 7)
        testFailed(&strbuf, "strbuf.length != 7");

    if(strcmp(strbuf.chars, "1234569") != 0)
        testFailed(&strbuf, "strcmp(strbuf.chars, \"123469\") != 0");

    //removeStrings

    ffStrbufRemoveStrings(&strbuf, 3, "23", "45", "9");

    if(strbuf.length != 2)
        testFailed(&strbuf, "strbuf.length != 2");

    if(strcmp(strbuf.chars, "16") != 0)
        testFailed(&strbuf, "strbuf.chars != \"126\"");

    //PrependS

    ffStrbufPrependS(&strbuf, "123");

    if(strbuf.length != 5)
        testFailed(&strbuf, "strbuf.length != 5");

    if(strcmp(strbuf.chars, "12316") != 0)
        testFailed(&strbuf, "strbuf.chars != \"12316\"");

    //Destroy

    ffStrbufDestroy(&strbuf);
    if(strbuf.allocated != 0)
        testFailed(&strbuf, "strbuf.allocated != 0");

    if(strbuf.length != 0)
        testFailed(&strbuf, "strbuf.length != 0");

    if(strbuf.chars != NULL)
        testFailed(&strbuf, "strbuf.chars != NULL");

    //Success
    puts("\033[32mAll tests passed!"FASTFETCH_TEXT_MODIFIER_RESET);
}
