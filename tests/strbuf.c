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

    //append(N)C

    ffStrbufAppendC(&strbuf, '1');
    VERIFY(ffStrbufEqualS(&strbuf, "1"));
    VERIFY(strbuf.allocated >= 1);
    ffStrbufAppendNC(&strbuf, 5, '2');
    VERIFY(ffStrbufEqualS(&strbuf, "122222"));
    VERIFY(strbuf.allocated >= 6);
    ffStrbufClear(&strbuf);

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

    VERIFY(ffStrbufSubstrBefore(&strbuf, 9));
    VERIFY(strbuf.length == 9);
    VERIFY(strbuf.allocated >= 110);
    VERIFY(strbuf.chars[strbuf.length] == 0);
    VERIFY(ffStrbufEqualS(&strbuf, "123456789"));

    VERIFY(!ffStrbufSubstrBeforeFirstC(&strbuf, '0'));
    VERIFY(!ffStrbufSubstrBeforeLastC(&strbuf, '0'));
    VERIFY(!ffStrbufSubstrAfterFirstC(&strbuf, '0'));
    VERIFY(!ffStrbufSubstrAfterLastC(&strbuf, '0'));
    VERIFY(ffStrbufEqualS(&strbuf, "123456789"));

    VERIFY(ffStrbufSubstrBeforeFirstC(&strbuf, '9'));
    VERIFY(ffStrbufSubstrBeforeLastC(&strbuf, '8'));
    VERIFY(ffStrbufSubstrAfterFirstC(&strbuf, '1'));
    VERIFY(ffStrbufSubstrAfterLastC(&strbuf, '2'));
    VERIFY(ffStrbufEqualS(&strbuf, "34567"));

    ffStrbufSetS(&strbuf, "123456789");

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
    VERIFY(ffStrbufToUInt(&strbuf, 999) == 123456789);

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

    //trimRight
    ffStrbufTrimRight(&strbuf, '$');
    VERIFY(ffStrbufEqualS(&strbuf, "AbCdEfG"));
    ffStrbufTrimRight(&strbuf, '$');
    VERIFY(ffStrbufEqualS(&strbuf, "AbCdEfG"));

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

    //trim
    ffStrbufTrim(&strbuf, '-');
    VERIFY(ffStrbufEqualS(&strbuf, "234567890-234567890-234567890"));

    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateS
    {
        FF_STRBUF_AUTO_DESTROY testCreate = ffStrbufCreateS("TEST");
        VERIFY(ffStrbufEqualS(&testCreate, "TEST"));
    }

    //ffStrbufCreateStatic
    ffStrbufInitStatic(&strbuf, "TEST");
    VERIFY(ffStrbufEqualS(&strbuf, "TEST"));
    VERIFY(strbuf.length == 4);
    VERIFY(strbuf.allocated == 0);

    ffStrbufDestroy(&strbuf);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 0);

    //ffStrbufCreateStatic / Allocate
    ffStrbufInitStatic(&strbuf, "TEST");
    ffStrbufEnsureFree(&strbuf, 0);
    VERIFY(ffStrbufEqualS(&strbuf, "TEST"));
    VERIFY(strbuf.length == 4);
    VERIFY(strbuf.allocated > 0);

    //ffStrbufCreateStatic / Append
    ffStrbufInitStatic(&strbuf, "TEST");
    ffStrbufAppendS(&strbuf, "_TEST");
    VERIFY(ffStrbufEqualS(&strbuf, "TEST_TEST"));
    VERIFY(strbuf.length == 9);
    VERIFY(strbuf.allocated >= 10);
    ffStrbufAppendC(&strbuf, '_');
    VERIFY(ffStrbufEqualS(&strbuf, "TEST_TEST_"));
    ffStrbufDestroy(&strbuf);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 0);

    //ffStrbufCreateStatic / Prepend
    ffStrbufInitStatic(&strbuf, "TEST");
    ffStrbufPrependS(&strbuf, "TEST_");
    VERIFY(ffStrbufEqualS(&strbuf, "TEST_TEST"));
    VERIFY(strbuf.length == 9);
    VERIFY(strbuf.allocated >= 10);
    ffStrbufPrependC(&strbuf, '_');
    VERIFY(ffStrbufEqualS(&strbuf, "_TEST_TEST"));
    ffStrbufDestroy(&strbuf);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 0);

    //ffStrbufCreateStatic / Clear
    ffStrbufInitStatic(&strbuf, "TEST");
    ffStrbufClear(&strbuf);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateStatic / Set
    ffStrbufInitStatic(&strbuf, "TEST"); // static
    ffStrbufSetStatic(&strbuf, "test");
    VERIFY(ffStrbufEqualS(&strbuf, "test"));
    VERIFY(strbuf.length == 4);
    VERIFY(strbuf.allocated == 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateStatic / Set
    ffStrbufInitS(&strbuf, "TEST"); // allocated
    ffStrbufSetStatic(&strbuf, "test");
    VERIFY(ffStrbufEqualS(&strbuf, "test"));
    VERIFY(strbuf.length == 4);
    VERIFY(strbuf.allocated == 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateStatic / TrimL
    ffStrbufInitStatic(&strbuf, "_TEST_");
    ffStrbufTrimLeft(&strbuf, '_');
    VERIFY(ffStrbufEqualS(&strbuf, "TEST_"));
    VERIFY(strbuf.length == 5);
    VERIFY(strbuf.allocated == 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateStatic / TrimR
    ffStrbufInitStatic(&strbuf, "_TEST_");
    ffStrbufTrimRight(&strbuf, ' ');
    VERIFY(strbuf.allocated == 0);
    VERIFY(ffStrbufEqualS(&strbuf, "_TEST_"));
    ffStrbufTrimRight(&strbuf, '_');
    VERIFY(ffStrbufEqualS(&strbuf, "_TEST"));
    VERIFY(strbuf.length == 5);
    VERIFY(strbuf.allocated > 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateStatic / Substr
    ffStrbufInitStatic(&strbuf, "__TEST__");
    VERIFY(ffStrbufRemoveSubstr(&strbuf, 0, 6));
    VERIFY(ffStrbufEqualS(&strbuf, "__"));
    VERIFY(strbuf.length == 2);
    VERIFY(strbuf.allocated > 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateStatic / Substr
    ffStrbufInitStatic(&strbuf, "__TEST__");
    VERIFY(ffStrbufRemoveSubstr(&strbuf, 2, 8));
    VERIFY(ffStrbufEqualS(&strbuf, "__"));
    VERIFY(strbuf.length == 2);
    VERIFY(strbuf.allocated > 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateStatic / Substr
    ffStrbufInitStatic(&strbuf, "__TEST__");
    VERIFY(ffStrbufRemoveSubstr(&strbuf, 2, 6));
    VERIFY(ffStrbufEqualS(&strbuf, "____"));
    VERIFY(strbuf.length == 4);
    VERIFY(strbuf.allocated > 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreateStatic / ReplaceAllC
    ffStrbufInitStatic(&strbuf, "__TEST__");
    ffStrbufReplaceAllC(&strbuf, '_', '-');
    VERIFY(ffStrbufEqualS(&strbuf, "--TEST--"));
    VERIFY(strbuf.length == 8);
    VERIFY(strbuf.allocated > 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufEnsureFixedLengthFree / empty buffer
    ffStrbufInit(&strbuf);
    ffStrbufEnsureFixedLengthFree(&strbuf, 10);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 11);
    ffStrbufDestroy(&strbuf);
    ffStrbufInitA(&strbuf, 10);
    ffStrbufEnsureFixedLengthFree(&strbuf, 10);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 11);
    ffStrbufDestroy(&strbuf);

    //ffStrbufEnsureFixedLengthFree / empty buffer with zero free length
    ffStrbufInit(&strbuf);
    ffStrbufEnsureFixedLengthFree(&strbuf, 0);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 0);
    ffStrbufDestroy(&strbuf);

    //ffStrbufEnsureFixedLengthFree / empty buffer but oldFree >= newFree
    ffStrbufInitA(&strbuf, 11);
    ffStrbufEnsureFixedLengthFree(&strbuf, 10);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 11);
    ffStrbufDestroy(&strbuf);
    ffStrbufInitA(&strbuf, 12);
    ffStrbufEnsureFixedLengthFree(&strbuf, 10);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 12);
    ffStrbufDestroy(&strbuf);

    //ffStrbufEnsureFixedLengthFree / non empty buffer
    ffStrbufAppendF(&strbuf, "%s", "1234567890");
    VERIFY(strbuf.length == 10);
    VERIFY(strbuf.allocated == 32);
    ffStrbufEnsureFixedLengthFree(&strbuf, 0);
    VERIFY(strbuf.length == 10);
    VERIFY(strbuf.allocated == 32);
    ffStrbufEnsureFixedLengthFree(&strbuf, 20); // less than oldFree (=21)
    VERIFY(strbuf.length == 10);
    VERIFY(strbuf.allocated == 32);
    ffStrbufEnsureFixedLengthFree(&strbuf, 21); // equal to oldFree (=21)
    VERIFY(strbuf.length == 10);
    VERIFY(strbuf.allocated == 32);
    ffStrbufEnsureFixedLengthFree(&strbuf, 22); // greater than oldFree (=21)
    VERIFY(strbuf.length == 10);
    VERIFY(strbuf.allocated == 33);
    ffStrbufDestroy(&strbuf);

    //ffStrbufEnsureFixedLengthFree / static buffer
    ffStrbufInitStatic(&strbuf, "__TEST__");
    VERIFY(strbuf.length > 0);
    VERIFY(strbuf.allocated == 0);
    ffStrbufEnsureFixedLengthFree(&strbuf, 10);
    VERIFY(strbuf.length == strlen("__TEST__"));
    VERIFY(strbuf.allocated == strlen("__TEST__") + 1 + 10);
    VERIFY(ffStrbufEqualS(&strbuf, "__TEST__"));
    ffStrbufDestroy(&strbuf);

    //ffStrbufInsertNC
    ffStrbufInitStatic(&strbuf, "123456");
    ffStrbufInsertNC(&strbuf, 0, 2, 'A');
    VERIFY(ffStrbufEqualS(&strbuf, "AA123456"));
    ffStrbufInsertNC(&strbuf, 4, 2, 'B');
    VERIFY(ffStrbufEqualS(&strbuf, "AA12BB3456"));
    ffStrbufInsertNC(&strbuf, strbuf.length, 2, 'C');
    VERIFY(ffStrbufEqualS(&strbuf, "AA12BB3456CC"));
    ffStrbufInsertNC(&strbuf, 999, 2, 'D');
    VERIFY(ffStrbufEqualS(&strbuf, "AA12BB3456CCDD"));
    ffStrbufDestroy(&strbuf);

    //Success
    puts("\e[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
}
