#include "common/format.h"
#include "util/textModifier.h"
#include "fastfetch.h"

static void verify(const char* format, const char* arg, const char* expected, int lineNo)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY formatter = ffStrbufCreateStatic(format);
    const FFformatarg arguments[] = {
        { .type = FF_FORMAT_ARG_TYPE_STRING, arg }
    };
    ffParseFormatString(&result, &formatter, 1, arguments);
    if (!ffStrbufEqualS(&result, expected))
    {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s: expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, format, expected, result.chars);
        exit(1);
    }
}

#define VERIFY(format, argument, expected) verify((format), (argument), (expected), __LINE__)

int main(void)
{
    instance.config.display.pipe = true;

    {
    VERIFY("output({})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1})", "12345 67890", "output(12345 67890)");
    VERIFY("output({})", "", "output()");
    VERIFY("output({1})", "", "output()");
    }

    {
    VERIFY("output({1:20})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1:11})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1:-11})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1:6})", "12345 67890", "output(12345)");
    VERIFY("output({:6})", "12345 67890", "output(12345)");
    VERIFY("output({:-6})", "12345 67890", "output(12345…)");
    VERIFY("output({:0})", "12345 67890", "output()");
    VERIFY("output({:})", "12345 67890", "output()");
    }

    {
    VERIFY("output({1<20})", "12345 67890", "output(12345 67890         )");
    VERIFY("output({1<-20})", "12345 67890", "output(12345 67890         )");
    VERIFY("output({1<11})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1<-11})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1<6})", "12345 67890", "output(12345 )");
    VERIFY("output({<6})", "12345 67890", "output(12345 )");
    VERIFY("output({<-6})", "12345 67890", "output(12345…)");
    VERIFY("output({<0})", "12345 67890", "output()");
    VERIFY("output({<})", "12345 67890", "output()");
    }

    {
    VERIFY("output({1>20})", "12345 67890", "output(         12345 67890)");
    VERIFY("output({1>-20})", "12345 67890", "output(         12345 67890)");
    VERIFY("output({1>11})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1>-11})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1>6})", "12345 67890", "output(12345 )");
    VERIFY("output({>6})", "12345 67890", "output(12345 )");
    VERIFY("output({>-6})", "12345 67890", "output(12345…)");
    VERIFY("output({>0})", "12345 67890", "output()");
    VERIFY("output({>})", "12345 67890", "output()");
    }

    {
    VERIFY("output({1n>20})", "12345 67890", "output({1n>20})");
    VERIFY("output({120})", "12345 67890", "output({120})");
    VERIFY("output({1:11})", "", "output()");
    VERIFY("output({2:2})", "", "output({2:2})");
    }

    {
    VERIFY("output({1~0})", "12345 67890", "output(12345 67890)");
    VERIFY("output({1~1})", "12345 67890", "output(2345 67890)");
    VERIFY("output({1~10})", "12345 67890", "output(0)");
    VERIFY("output({1~20})", "12345 67890", "output()");
    VERIFY("output({1~-5})", "12345 67890", "output(67890)");
    VERIFY("output({1~-50})", "12345 67890", "output()");
    VERIFY("output({1~0,1})", "12345 67890", "output(1)");
    VERIFY("output({1~0,6})", "12345 67890", "output(12345 )");
    VERIFY("output({1~0,10})", "12345 67890", "output(12345 6789)");
    VERIFY("output({1~5,10})", "12345 67890", "output( 6789)");
    VERIFY("output({1~5,100})", "12345 67890", "output( 67890)");
    VERIFY("output({1~10,10})", "12345 67890", "output()");
    VERIFY("output({1~10,5})", "12345 67890", "output()");
    VERIFY("output({1~3,-3})", "12345 67890", "output(45 67)");
    VERIFY("output({1~-5,-3})", "12345 67890", "output(67)");
    VERIFY("output({1~-0,10})", "12345 67890", "output(12345 6789)");
    VERIFY("output({1~-3,-5})", "12345 67890", "output()");
    VERIFY("output({1~})", "12345 67890", "output(12345 67890)"); // Same as {1~0}
    VERIFY("output({1~-0})", "12345 67890", "output(12345 67890)"); // Same as {1~0}
    VERIFY("output({1~,-1})", "12345 67890", "output(12345 6789)"); // Same as {1~0,-1}
    VERIFY("output({1~,})", "12345 67890", "output()"); // Same as {1~0,0}
    }

    {
    VERIFY("output({1n~0})", "12345 67890", "output({1n~0})");
    VERIFY("output({1~<0})", "12345 67890", "output({1~<0})");
    VERIFY("output({1~-})", "12345 67890", "output({1~-})");
    VERIFY("output({1~-,1})", "12345 67890", "output({1~-,1})");
    VERIFY("output({1~0,,1})", "12345 67890", "output({1~0,,1})");
    VERIFY("output({1~0,1,})", "12345 67890", "output({1~0,1,})");
    VERIFY("output({1~,0,1})", "12345 67890", "output({1~,0,1})");
    VERIFY("output({2,0})", "12345 67890", "output({2,0})");
    }

    {
    VERIFY("output({1:20}{1<20}{1>20})", "12345 67890", "output(12345 6789012345 67890                  12345 67890)");
    VERIFY("output({?1}OK{?}{/1}NOT OK{/})", "12345 67890", "output(OK)");
    VERIFY("output({?1}OK{?}{/1}NOT OK{/})", "", "output(NOT OK)");
    }

    //Success
    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
}
