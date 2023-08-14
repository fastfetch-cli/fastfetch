#include "util/FFlist.h"
#include "util/textModifier.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

__attribute__((__noreturn__))
static void testFailed(const FFlist* list, const char* expression, int lineNo)
{
    fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stderr);
    fprintf(stderr, "[%d] %s, list:", lineNo, expression);
    for (uint32_t i = 0; i < list->length; ++i)
    {
        fprintf(stderr, "%u ", *(uint32_t*)ffListGet(list, i));
    }
    fputc('\n', stderr);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stderr);
    fputc('\n', stderr);
    exit(1);
}

static bool numEqualsAdapter(const void* first, const void* second)
{
    return *(uint32_t*)first == *(uint32_t*)second;
}

#define VERIFY(expression) if(!(expression)) testFailed(&list, #expression, __LINE__)

int main(void)
{
    FFlist list;
    uint32_t n = 0;

    //initA

    ffListInit(&list, sizeof(uint32_t));

    VERIFY(list.elementSize == sizeof(uint32_t));
    VERIFY(list.capacity == 0);
    VERIFY(list.length == 0);

    //forEach
    FF_LIST_FOR_EACH(uint32_t, item, list)
        VERIFY(false);

    //shift
    VERIFY(!ffListShift(&list, &n));
    VERIFY(list.length == 0);

    //pop
    VERIFY(!ffListPop(&list, &n));
    VERIFY(list.length == 0);

    //add
    for (uint32_t i = 1; i <= FF_LIST_DEFAULT_ALLOC + 1; ++i)
    {
        *(uint32_t*)ffListAdd(&list) = i;

        VERIFY(list.elementSize == sizeof(uint32_t));
        VERIFY(list.length == i);

        if(i <= FF_LIST_DEFAULT_ALLOC)
        {
            VERIFY(list.capacity == FF_LIST_DEFAULT_ALLOC);
        }
        else
        {
            VERIFY(list.capacity == FF_LIST_DEFAULT_ALLOC * 2);
        }

        VERIFY(*(uint32_t*)ffListGet(&list, 0) == 1);
        VERIFY(*(uint32_t*)ffListGet(&list, i - 1) == i);
    }
    VERIFY(list.length == FF_LIST_DEFAULT_ALLOC + 1);

    uint32_t sum = 0;
    //forEach
    FF_LIST_FOR_EACH(uint32_t, item, list)
        sum += *item;
    VERIFY(sum == (1 + FF_LIST_DEFAULT_ALLOC + 1) * (FF_LIST_DEFAULT_ALLOC + 1) / 2);

    // ffListFirstIndexComp
    n = 10;
    VERIFY(ffListFirstIndexComp(&list, &n, numEqualsAdapter) == 9);
    n = 999;
    VERIFY(ffListFirstIndexComp(&list, &n, numEqualsAdapter) == list.length);

    // ffListContains
    n = 10;
    VERIFY(ffListContains(&list, &n, numEqualsAdapter));
    n = 999;
    VERIFY(!ffListContains(&list, &n, numEqualsAdapter));

    //shift
    VERIFY(ffListShift(&list, &n));
    VERIFY(n == 1);
    VERIFY(list.length == FF_LIST_DEFAULT_ALLOC);
    VERIFY(*(uint32_t*) ffListGet(&list, 0) == 2);
    VERIFY(*(uint32_t*) ffListGet(&list, list.length - 1) == FF_LIST_DEFAULT_ALLOC + 1);

    //pop
    VERIFY(ffListPop(&list, &n));
    VERIFY(n == FF_LIST_DEFAULT_ALLOC + 1);
    VERIFY(list.length == FF_LIST_DEFAULT_ALLOC - 1);
    VERIFY(*(uint32_t*) ffListGet(&list, 0) == 2);
    VERIFY(*(uint32_t*) ffListGet(&list, list.length - 1) == FF_LIST_DEFAULT_ALLOC);

    //Destroy
    ffListDestroy(&list);

    VERIFY(list.elementSize == sizeof(uint32_t));
    VERIFY(list.capacity == 0);
    VERIFY(list.length == 0);

    {
        FF_LIST_AUTO_DESTROY test = ffListCreate(1);
        VERIFY(test.elementSize = 1);
        VERIFY(test.capacity == 0);
        VERIFY(test.length == 0);
    }

    //Success
    puts("\033[32mAll tests passed!"FASTFETCH_TEXT_MODIFIER_RESET);
}
