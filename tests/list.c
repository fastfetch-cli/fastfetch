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

    //initA

    ffListInit(&list, sizeof(uint32_t));

    VERIFY(list.elementSize == sizeof(uint32_t));
    VERIFY(list.capacity == 0);
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

    // ffListFirstIndexComp
    uint32_t n = 10;
    VERIFY(ffListFirstIndexComp(&list, &n, numEqualsAdapter) == 9);
    n = 999;
    VERIFY(ffListFirstIndexComp(&list, &n, numEqualsAdapter) == list.length);

    //Destroy
    ffListDestroy(&list);

    VERIFY(list.elementSize == sizeof(uint32_t));
    VERIFY(list.capacity == 0);
    VERIFY(list.length == 0);

    //Success
    puts("\033[32mAll tests passed!"FASTFETCH_TEXT_MODIFIER_RESET);
}
