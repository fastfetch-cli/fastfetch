#include "util/FFstrbuf.h"
#include "util/textModifier.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

__attribute__((__noreturn__))
static void testFailed(const FFstrbuf* strbuf, const char* expression, int lineNo)
{
    fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stderr);
    fprintf(stderr, "[%d] %s, strbuf:", lineNo, expression);
    ffStrbufWriteTo(strbuf, stderr);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stderr);
    fputc('\n', stderr);
    exit(1);
}

#define VERIFY(expression) if(!(expression)) testFailed(&strbuf, #expression, __LINE__)

int shouldNotBeCalled(int c) {
    (void)c;
    exit(1);
}

int main(void)
{
    FFstrbuf strbuf;

    //destroy 0
    ffStrbufInit(&strbuf);
    ffStrbufDestroy(&strbuf);

    //initA

    ffStrbufInit(&strbuf);

    VERIFY(strbuf.chars[0] == 0);
    VERIFY(strbuf.allocated == 0);
    VERIFY(strbuf.length == 0);

    VERIFY(!ffStrbufStartsWithC(&strbuf, '0'));
    VERIFY(!ffStrbufEndsWithC(&strbuf, '0'));
    VERIFY(!ffStrbufStartsWithS(&strbuf, "0"));
    VERIFY(!ffStrbufEndsWithS(&strbuf, "0"));
    VERIFY(ffStrbufCountC(&strbuf, '0') == 0);

    //Ensure following functions work with non-allocated string
    ffStrbufAppendS(&strbuf, "");
    ffStrbufAppendF(&strbuf, "%s", "");
    ffStrbufAppendTransformS(&strbuf, "", shouldNotBeCalled);
    ffStrbufPrependS(&strbuf, "");
    ffStrbufTrim(&strbuf, ' ');

    VERIFY(strbuf.chars[0] == 0);
    VERIFY(strbuf.allocated == 0);
    VERIFY(strbuf.length == 0);

    //appendS

    ffStrbufAppendS(&strbuf, "12345");
    ffStrbufAppendS(&strbuf, NULL);

    VERIFY(strbuf.length == 5);
    VERIFY(strbuf.allocated >= 6);
    VERIFY(ffStrbufEqualS(&strbuf, "12345"));

    //appendNS

    ffStrbufAppendNS(&strbuf, 4, "67890");
    ffStrbufAppendNS(&strbuf, 0, NULL);

    VERIFY(strbuf.length == 9);
    VERIFY(strbuf.allocated >= 10);
    VERIFY(ffStrbufEqualS(&strbuf, "123456789"));

    //appendS long

    ffStrbufAppendS(&strbuf, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
    VERIFY(strbuf.length == 109);
    VERIFY(strbuf.allocated >= 110);
    VERIFY(strbuf.chars[strbuf.length] == 0);

    //substr

    ffStrbufSubstrBefore(&strbuf, 9);
    VERIFY(strbuf.length == 9);
    VERIFY(strbuf.allocated >= 110);
    VERIFY(strbuf.chars[strbuf.length] == 0);
    VERIFY(ffStrbufEqualS(&strbuf, "123456789"));

    //startsWithC

    VERIFY(ffStrbufStartsWithC(&strbuf, '1'));
    VERIFY(!ffStrbufStartsWithC(&strbuf, '2'));

    //startsWithS

    VERIFY(ffStrbufStartsWithS(&strbuf, "123"));
    VERIFY(ffStrbufStartsWithS(&strbuf, "123456789"));
    VERIFY(!ffStrbufStartsWithS(&strbuf, "1234567890123"));

    //endsWithC
    VERIFY(ffStrbufEndsWithC(&strbuf, '9'));
    VERIFY(!ffStrbufEndsWithC(&strbuf, '1'));

    //endsWithS

    VERIFY(ffStrbufEndsWithS(&strbuf, "789"));
    VERIFY(ffStrbufEndsWithS(&strbuf, "123456789"));
    VERIFY(!ffStrbufEndsWithS(&strbuf, "1234567890123"));

    //toNumber

    VERIFY(ffStrbufToDouble(&strbuf) == 123456789.0);
    VERIFY(ffStrbufToUInt16(&strbuf, 999) == 999); //overflow

    //countC

    VERIFY(ffStrbufCountC(&strbuf, '1') == 1);
    VERIFY(ffStrbufCountC(&strbuf, '0') == 0);

    //removeS

    ffStrbufRemoveS(&strbuf, "78");

    VERIFY(strbuf.length == 7);
    VERIFY(strcmp(strbuf.chars, "1234569") == 0);

    //removeStrings

    ffStrbufRemoveStrings(&strbuf, 3, (const char*[]) { "23", "45", "9" });

    VERIFY(strbuf.length == 2);
    VERIFY(strcmp(strbuf.chars, "16") == 0);

    //PrependS

    ffStrbufPrependS(&strbuf, "123");
    ffStrbufPrependS(&strbuf, NULL);

    VERIFY(strbuf.length == 5);
    VERIFY(strcmp(strbuf.chars, "12316") == 0);

    //indexC
    VERIFY(ffStrbufFirstIndexC(&strbuf, '1') == 0);
    VERIFY(ffStrbufNextIndexC(&strbuf, 1, '1') == 3);
    VERIFY(ffStrbufNextIndexC(&strbuf, 4, '1') == 5);
    VERIFY(ffStrbufLastIndexC(&strbuf, '1') == 3);
    VERIFY(ffStrbufPreviousIndexC(&strbuf, 2, '1') == 0);
    VERIFY(ffStrbufPreviousIndexC(&strbuf, 0, '1') == 0);
    VERIFY(ffStrbufPreviousIndexC(&strbuf, 0, '2') == 5);

    //indexS
    VERIFY(ffStrbufFirstIndexS(&strbuf, "12316") == 0);
    VERIFY(ffStrbufNextIndexS(&strbuf, 1, "1") == 3);
    VERIFY(ffStrbufNextIndexS(&strbuf, 4, "1") == 5);

    //ignCase
    ffStrbufSetS(&strbuf, "AbCdEfG");

    VERIFY(ffStrbufIgnCaseCompS(&strbuf, "aBcDeFg") == 0);
    VERIFY(ffStrbufStartsWithIgnCaseS(&strbuf, "ABCD"));
    VERIFY(ffStrbufEndsWithIgnCaseS(&strbuf, "defg"));
    VERIFY(!ffStrbufStartsWithIgnCaseS(&strbuf, "aBcDeFgH"));
    VERIFY(!ffStrbufEndsWithIgnCaseS(&strbuf, "0aBcDeFg"));

    //ensure
    ffStrbufEnsureEndsWithC(&strbuf, '$');
    VERIFY(ffStrbufEqualS(&strbuf, "AbCdEfG$"));
    ffStrbufEnsureEndsWithC(&strbuf, '$');
    VERIFY(ffStrbufEqualS(&strbuf, "AbCdEfG$"));

    //clear
    ffStrbufClear(&strbuf);
    VERIFY(strbuf.allocated > 0);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.chars && strbuf.chars[0] == 0);

    //Destroy

    ffStrbufDestroy(&strbuf);

    VERIFY(strbuf.allocated == 0);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.chars && strbuf.chars[0] == 0);

    //initA
    ffStrbufInitA(&strbuf, 32);

    VERIFY(strbuf.allocated == 32);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.chars && strbuf.chars[0] == 0);

    //appendF
    ffStrbufAppendF(&strbuf, "%s", "1234567890123456789012345678901");
    VERIFY(strbuf.allocated == 32);
    VERIFY(ffStrbufEqualS(&strbuf, "1234567890123456789012345678901"));

    ffStrbufDestroy(&strbuf);

    //initF
    ffStrbufInitF(&strbuf, "%s", "1234567890123456789012345678901");
    VERIFY(strbuf.allocated == 32);
    VERIFY(ffStrbufEqualS(&strbuf, "1234567890123456789012345678901"));

    //containC
    VERIFY(ffStrbufContainC(&strbuf, '1'));
    VERIFY(!ffStrbufContainC(&strbuf, '-'));

    //replaceAllC
    ffStrbufReplaceAllC(&strbuf, '1', '-');
    VERIFY(ffStrbufEqualS(&strbuf, "-234567890-234567890-234567890-"));
    ffStrbufReplaceAllC(&strbuf, '1', '-');
    VERIFY(ffStrbufEqualS(&strbuf, "-234567890-234567890-234567890-"));

    ffStrbufDestroy(&strbuf);

    {
        FF_STRBUF_AUTO_DESTROY testCreate = ffStrbufCreateS("TEST");
        VERIFY(ffStrbufEqualS(&testCreate, "TEST"));
    }

    //Success
    puts("\033[32mAll tests passed!"FASTFETCH_TEXT_MODIFIER_RESET);
}
