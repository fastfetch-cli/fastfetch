#include "fastfetch.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

static void testFailed(const FFlist* list, const char* message, ...)
{
    va_list args;
    va_start(args, message);
    fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stderr);
    vfprintf(stderr, message, args);
    for (uint32_t i = 0; i < list->length; ++i)
    {
        fprintf(stderr, "%u ", *(uint32_t*)ffListGet(list, i));
    }
    fputc('\n', stderr);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stderr);
    fputc('\n', stderr);
    va_end(args);
    exit(1);
}

int main(int argc, char** argv)
{
    FF_UNUSED(argc, argv)

    FFlist list;

    //initA

    ffListInitA(&list, sizeof(uint32_t), 0);

    if(ffListGet(&list, 0) != NULL)
        testFailed(&list, "ffListGet(&list, 0) != NULL");

    if(list.elementSize != sizeof(int))
        testFailed(&list, "list.elementSize != sizeof(int)");

    if(list.capacity != 0)
        testFailed(&list, "list.capacity != 0");

    if(list.length != 0)
        testFailed(&list, "list.length != 0");

    //add
    for (uint32_t i = 1; i <= FF_LIST_DEFAULT_ALLOC + 1; ++i)
    {
        *(uint32_t*)ffListAdd(&list) = i;

        if(list.elementSize != sizeof(uint32_t))
            testFailed(&list, "list.elementSize != sizeof(uint32_t)");

        if(list.length != i)
            testFailed(&list, "list.length != i");

        if(i <= FF_LIST_DEFAULT_ALLOC)
        {
            if(list.capacity != FF_LIST_DEFAULT_ALLOC)
                testFailed(&list, "list.length != FF_LIST_DEFAULT_ALLOC");
        }
        else
        {
            if(list.capacity != FF_LIST_DEFAULT_ALLOC * 2)
                testFailed(&list, "list.length != FF_LIST_DEFAULT_ALLOC * 2");
        }

        if(*(uint32_t*)ffListGet(&list, 0) != 1)
            testFailed(&list, "*(int*)ffListGet(&list, 0) != 1");

        if(*(uint32_t*)ffListGet(&list, i - 1) != i)
            testFailed(&list, "*(int*)ffListGet(&list, i - 1) != i");
    }

    //Destroy
    ffListDestroy(&list);

    if(list.elementSize != sizeof(uint32_t))
        testFailed(&list, "list.elementSize != sizeof(uint32_t)");

    if(list.capacity != 0)
        testFailed(&list, "list.capacity != 0");

    if(list.length != 0)
        testFailed(&list, "list.length != 0");

    //Success
    puts("\033[32mAll tests passed!"FASTFETCH_TEXT_MODIFIER_RESET);
}
