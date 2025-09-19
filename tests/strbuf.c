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

    VERIFY(ffStrbufToDouble(&strbuf, -DBL_MAX) == 123456789.0);
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

    //ffStrbufCreateStatic / TrimSpace
    ffStrbufInitStatic(&strbuf, "\n TEST\n ");
    ffStrbufTrimSpace(&strbuf);
    VERIFY(strbuf.length == 4);
    VERIFY(strbuf.allocated > 0);
    VERIFY(ffStrbufEqualS(&strbuf, "TEST"));
    ffStrbufDestroy(&strbuf);

    //ffStrbufCreate / TrimSpace
    ffStrbufInitS(&strbuf, "\n TEST\n ");
    ffStrbufTrimSpace(&strbuf);
    VERIFY(strbuf.length == 4);
    VERIFY(strbuf.allocated > 0);
    VERIFY(ffStrbufEqualS(&strbuf, "TEST"));
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
    ffStrbufEnsureFixedLengthFree(&strbuf, 12);
    VERIFY(strbuf.length == 0);
    VERIFY(strbuf.allocated == 13);
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

    // smallest allocation test
    {
        FF_STRBUF_AUTO_DESTROY strbuf1 = ffStrbufCreateA(10);
        VERIFY(strbuf1.allocated == 10);
        ffStrbufEnsureFree(&strbuf1, 16);
        VERIFY(strbuf1.allocated == 32);

        FF_STRBUF_AUTO_DESTROY strbuf2 = ffStrbufCreate();
        VERIFY(strbuf2.allocated == 0);
        ffStrbufEnsureFree(&strbuf2, 16);
        VERIFY(strbuf2.allocated == 32);
    }

    {
        int i = 0;
        char* lineptr = NULL;
        size_t n = 0;
        const char* text = "Processor\t: ARMv7\nprocessor\t: 0\nBogoMIPS\t: 38.00\n\nprocessor\t: 1\nBogoMIPS\t: 38.00";
        ffStrbufSetS(&strbuf, text);

        while (ffStrbufGetline(&lineptr, &n, &strbuf))
        {
            ++i;
            switch (i)
            {
                case 1:
                    VERIFY(strcmp(lineptr, "Processor\t: ARMv7") == 0);
                    VERIFY(n == strlen("Processor\t: ARMv7"));
                    break;
                case 2:
                    VERIFY(strcmp(lineptr, "processor\t: 0") == 0);
                    VERIFY(n == strlen("processor\t: 0"));
                    break;
                case 3:
                    VERIFY(strcmp(lineptr, "BogoMIPS\t: 38.00") == 0);
                    VERIFY(n == strlen("BogoMIPS\t: 38.00"));
                    break;
                case 4:
                    VERIFY(strcmp(lineptr, "") == 0);
                    VERIFY(n == 0);
                    break;
                case 5:
                    VERIFY(strcmp(lineptr, "processor\t: 1") == 0);
                    VERIFY(n == strlen("processor\t: 1"));
                    break;
                case 6:
                    VERIFY(strcmp(lineptr, "BogoMIPS\t: 38.00") == 0);
                    VERIFY(n == strlen("BogoMIPS\t: 38.00"));
                    break;
                default:
                    VERIFY(false);
                    break;
            }
        }
        VERIFY(ffStrbufEqualS(&strbuf, text));
        VERIFY(*lineptr == '\0');
        VERIFY(i == 6);

        lineptr = NULL;
        n = 0;
        i = 0;
        text = "\n";
        ffStrbufSetS(&strbuf, text);
        while (ffStrbufGetline(&lineptr, &n, &strbuf))
        {
            ++i;
            switch (i)
            {
                case 1:
                    VERIFY(strcmp(lineptr, "") == 0);
                    VERIFY(n == 0);
                    break;
                default:
                    VERIFY(false);
                    break;
            }
        }
        VERIFY(ffStrbufEqualS(&strbuf, text));
        VERIFY(*lineptr == '\0');
        VERIFY(i == 1);

        lineptr = NULL;
        n = 0;
        i = 0;
        text = "abcd";
        ffStrbufSetS(&strbuf, text);
        while (ffStrbufGetline(&lineptr, &n, &strbuf))
        {
            ++i;
            switch (i)
            {
                case 1:
                    VERIFY(strcmp(lineptr, "abcd") == 0);
                    VERIFY(n == strlen("abcd"));
                    break;
                default:
                    VERIFY(false);
                    break;
            }
        }
        VERIFY(ffStrbufEqualS(&strbuf, text));
        VERIFY(*lineptr == '\0');
        VERIFY(i == 1);

        lineptr = NULL;
        n = 0;
        i = 0;
        text = "";
        ffStrbufSetS(&strbuf, text);
        while (ffStrbufGetline(&lineptr, &n, &strbuf))
        {
            ++i;
            VERIFY(false);
        }

        VERIFY(ffStrbufEqualS(&strbuf, text));
        VERIFY(*lineptr == '\0');
        VERIFY(i == 0);
    }

    ffStrbufSetS(&strbuf, "Hello World");
    VERIFY(ffStrbufRemoveDupWhitespaces(&strbuf) == false);
    VERIFY(strcmp(strbuf.chars, "Hello World") == 0);

    ffStrbufSetS(&strbuf, "Hello   World");
    VERIFY(ffStrbufRemoveDupWhitespaces(&strbuf) == true);
    VERIFY(strcmp(strbuf.chars, "Hello World") == 0);

    ffStrbufSetS(&strbuf, "   Hello World   ");
    VERIFY(ffStrbufRemoveDupWhitespaces(&strbuf) == true);
    VERIFY(strcmp(strbuf.chars, " Hello World ") == 0);

    ffStrbufSetS(&strbuf, "   Hello   World   ");
    VERIFY(ffStrbufRemoveDupWhitespaces(&strbuf) == true);
    VERIFY(strcmp(strbuf.chars, " Hello World ") == 0);

    ffStrbufSetS(&strbuf, "   ");
    VERIFY(ffStrbufRemoveDupWhitespaces(&strbuf) == true);
    VERIFY(strcmp(strbuf.chars, " ") == 0);

    ffStrbufClear(&strbuf);
    VERIFY(ffStrbufRemoveDupWhitespaces(&strbuf) == false);
    VERIFY(strcmp(strbuf.chars, "") == 0);

    ffStrbufSetStatic(&strbuf, "   ");
    VERIFY(ffStrbufRemoveDupWhitespaces(&strbuf) == false);
    VERIFY(strcmp(strbuf.chars, "   ") == 0);

    {
        ffStrbufSetStatic(&strbuf, "abcdef");
        FF_STRBUF_AUTO_DESTROY newStr = ffStrbufCreateCopy(&strbuf);
        VERIFY(newStr.allocated == 0);
        VERIFY(newStr.chars == strbuf.chars);
    }

    {
        ffStrbufSetStatic(&strbuf, "abcdef");
        FF_STRBUF_AUTO_DESTROY newStr = ffStrbufCreateS("123456");
        ffStrbufSet(&newStr, &strbuf);
        VERIFY(newStr.allocated > 0);
        VERIFY(newStr.chars != strbuf.chars);
        VERIFY(ffStrbufEqualS(&newStr, "abcdef"));
    }

    {
        ffStrbufSetStatic(&strbuf, "abcdefghijkl");
        FF_STRBUF_AUTO_DESTROY newStr = ffStrbufCreateS("123456");
        ffStrbufSet(&newStr, &strbuf);
        VERIFY(newStr.allocated > 0);
        VERIFY(newStr.chars != strbuf.chars);
        VERIFY(ffStrbufEqualS(&newStr, "abcdefghijkl"));
    }

    {
        ffStrbufClear(&strbuf);
        FF_STRBUF_AUTO_DESTROY newStr = ffStrbufCreateCopy(&strbuf);
        VERIFY(newStr.allocated == 0);
        VERIFY(newStr.chars == strbuf.chars);
        VERIFY(newStr.chars[0] == '\0');
    }

    {
        ffStrbufClear(&strbuf);
        FF_STRBUF_AUTO_DESTROY newStr = ffStrbufCreateS("123456");
        ffStrbufSet(&newStr, &strbuf);
        VERIFY(newStr.allocated > 0);
        VERIFY(newStr.chars != strbuf.chars);
        VERIFY(ffStrbufEqualS(&newStr, ""));
    }

    {
        ffStrbufSetStatic(&strbuf, "abc");
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, "abc:def:ghi", ' ') == false);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, "abc:def:ghi", ':') == true);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, "def:ghi", ' ') == false);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, "def:ghi", ':') == false);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, "def", ':') == false);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, "abc", ':') == true);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, "", ' ') == false);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, ":abc:", ':') == true);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, "abc:", ':') == true);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, ":abc", ':') == true);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, ":ABC", ':') == false);
        VERIFY(ffStrbufMatchSeparatedS(&strbuf, ":abcdef", ':') == false);
    }

    {
        ffStrbufSetStatic(&strbuf, "ABC");
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, "abc:def:ghi", ' ') == false);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, "abc:def:ghi", ':') == true);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, "def:ghi", ' ') == false);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, "def:ghi", ':') == false);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, "def", ':') == false);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, "abc", ':') == true);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, "", ' ') == false);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, ":abc:", ':') == true);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, "abc:", ':') == true);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, ":abc", ':') == true);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, ":ABC", ':') == true);
        VERIFY(ffStrbufMatchSeparatedIgnCaseS(&strbuf, ":abcdef", ':') == false);
    }

    {
        ffStrbufSetStatic(&strbuf, "abc:def:ghi");
        VERIFY(ffStrbufSeparatedContainS(&strbuf, "abc", ' ') == false);
        VERIFY(ffStrbufSeparatedContainS(&strbuf, "abc", ':') == true);
        VERIFY(ffStrbufSeparatedContainS(&strbuf, "def", ' ') == false);
        VERIFY(ffStrbufSeparatedContainS(&strbuf, "def", ':') == true);
        VERIFY(ffStrbufSeparatedContainS(&strbuf, "DEF", ':') == false);
        VERIFY(ffStrbufSeparatedContainS(&strbuf, "a", ':') == false);
        VERIFY(ffStrbufSeparatedContainS(&strbuf, "e", ':') == false);
        VERIFY(ffStrbufSeparatedContainS(&strbuf, "i", ':') == false);
    }

    {
        ffStrbufSetStatic(&strbuf, "ABC:DEF:GHI");
        VERIFY(ffStrbufSeparatedContainIgnCaseS(&strbuf, "abc", ' ') == false);
        VERIFY(ffStrbufSeparatedContainIgnCaseS(&strbuf, "abc", ':') == true);
        VERIFY(ffStrbufSeparatedContainIgnCaseS(&strbuf, "def", ' ') == false);
        VERIFY(ffStrbufSeparatedContainIgnCaseS(&strbuf, "def", ':') == true);
        VERIFY(ffStrbufSeparatedContainIgnCaseS(&strbuf, "DEF", ':') == true);
        VERIFY(ffStrbufSeparatedContainIgnCaseS(&strbuf, "a", ':') == false);
        VERIFY(ffStrbufSeparatedContainIgnCaseS(&strbuf, "e", ':') == false);
        VERIFY(ffStrbufSeparatedContainIgnCaseS(&strbuf, "i", ':') == false);
    }

    {
        ffStrbufSetStatic(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 0, 1); // start, end
        VERIFY(ffStrbufEqualS(&strbuf, "a"));

        ffStrbufSetStatic(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 1, 1);
        VERIFY(ffStrbufEqualS(&strbuf, ""));

        ffStrbufSetStatic(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 2, 1);
        VERIFY(ffStrbufEqualS(&strbuf, ""));

        ffStrbufSetStatic(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 2, 3);
        VERIFY(ffStrbufEqualS(&strbuf, "c"));

        ffStrbufSetStatic(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 0, 3);
        VERIFY(ffStrbufEqualS(&strbuf, "abc"));
    }

    {
        ffStrbufSetS(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 0, 1); // start, end
        VERIFY(ffStrbufEqualS(&strbuf, "a"));

        ffStrbufSetS(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 1, 1);
        VERIFY(ffStrbufEqualS(&strbuf, ""));

        ffStrbufSetS(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 2, 1);
        VERIFY(ffStrbufEqualS(&strbuf, ""));

        ffStrbufSetS(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 2, 3);
        VERIFY(ffStrbufEqualS(&strbuf, "c"));

        ffStrbufSetS(&strbuf, "abc");
        ffStrbufSubstr(&strbuf, 0, 3);
        VERIFY(ffStrbufEqualS(&strbuf, "abc"));

        ffStrbufDestroy(&strbuf);
    }

    {
        ffStrbufAppendUtf32CodePoint(&strbuf, 0x6587);
        ffStrbufAppendUtf32CodePoint(&strbuf, 0x6cc9);
        ffStrbufAppendUtf32CodePoint(&strbuf, 0x9a7f);
        VERIFY(ffStrbufEqualS(&strbuf, u8"文泉驿"));

        ffStrbufDestroy(&strbuf);
    }

    {
        ffStrbufAppendSInt(&strbuf, 1234567890);
        VERIFY(ffStrbufEqualS(&strbuf, "1234567890"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendSInt(&strbuf, -1234567890);
        VERIFY(ffStrbufEqualS(&strbuf, "-1234567890"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendSInt(&strbuf, 0);
        VERIFY(ffStrbufEqualS(&strbuf, "0"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendUInt(&strbuf, 1234567890);
        VERIFY(ffStrbufEqualS(&strbuf, "1234567890"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendUInt(&strbuf, 0);
        VERIFY(ffStrbufEqualS(&strbuf, "0"));

        ffStrbufDestroy(&strbuf);
    }

    {
        ffStrbufAppendDouble(&strbuf, 120.0, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.0, 1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "120.0"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.0, 5, true);
        VERIFY(ffStrbufEqualS(&strbuf, "120.00000"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.123456789, 5, true);
        VERIFY(ffStrbufEqualS(&strbuf, "120.12346"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.888888, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "121"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.999999, 2, true);
        VERIFY(ffStrbufEqualS(&strbuf, "121.00"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.123456789, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.123456789, -1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "120.123456789"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.123, 5, true);
        VERIFY(ffStrbufEqualS(&strbuf, "120.12300"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.0, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.0, 1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-120.0"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.0, 5, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-120.00000"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.123456789, 5, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-120.12346"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.123456789, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.123456789, -1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-120.123456789"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.123, 5, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-120.12300"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.888888, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-121"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.999999, 2, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-121.00"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 1.2345e50, 1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -1.2345e50, 1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 1.2345e50, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -1.2345e50, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 1.2345e50, -1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -1.2345e50, -1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 1.2345e20, 1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "123450000000000000000.0"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -1.2345e20, 1, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-123450000000000000000.0"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, +0.0, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "0"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -0.0, 0, true);
        VERIFY(ffStrbufEqualS(&strbuf, "-0"));

        ffStrbufDestroy(&strbuf);
    }

    {
        ffStrbufAppendDouble(&strbuf, 120.0, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.0, 1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.0, 5, false);
        VERIFY(ffStrbufEqualS(&strbuf, "120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.123456789, 5, false);
        VERIFY(ffStrbufEqualS(&strbuf, "120.12346"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.888888, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "121"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.999999, 2, false);
        VERIFY(ffStrbufEqualS(&strbuf, "121"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.123456789, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.123456789, -1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "120.123456789"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 120.123, 5, false);
        VERIFY(ffStrbufEqualS(&strbuf, "120.123"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.0, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.0, 1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.0, 5, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.123456789, 5, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-120.12346"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.123456789, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-120"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.123456789, -1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-120.123456789"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.123, 5, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-120.123"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.888888, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-121"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -120.999999, 2, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-121"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 1.2345e50, 1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -1.2345e50, 1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 1.2345e50, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -1.2345e50, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 1.2345e50, -1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -1.2345e50, -1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-1.2345e50"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, 1.2345e20, 1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "123450000000000000000"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -1.2345e20, 1, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-123450000000000000000"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, +0.0, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "0"));

        ffStrbufClear(&strbuf);
        ffStrbufAppendDouble(&strbuf, -0.0, 0, false);
        VERIFY(ffStrbufEqualS(&strbuf, "-0"));

        ffStrbufDestroy(&strbuf);
    }

    //Success
    puts("\e[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
}
