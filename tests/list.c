#include "common/FFlist.h"
#include "common/textModifier.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdnoreturn.h>

noreturn static void testFailed(const FFlist* list, const char* expression, int lineNo) {
    fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stderr);
    fprintf(stderr, "[%d] %s, list:", lineNo, expression);
    for (uint32_t i = 0; i < list->length; ++i) {
        fprintf(stderr, "%u ", *FF_LIST_GET(uint32_t, *list, i));
    }
    fputc('\n', stderr);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stderr);
    fputc('\n', stderr);
    exit(1);
}

static bool numEqualsAdapter(const uint32_t* first, const uint32_t* second) {
    return *first == *second;
}

#define VERIFY(expression) \
    if (!(expression)) testFailed(&list, #expression, __LINE__)

int main(void) {
    FFlist list;
    uint32_t n = 0;

    // initA

    ffListInit(&list);

    VERIFY(list.capacity == 0);
    VERIFY(list.length == 0);

    // forEach
    FF_LIST_FOR_EACH (uint32_t, item, list) {
        VERIFY(false);
    }

    // shift
    VERIFY(!FF_LIST_SHIFT(list, &n));
    VERIFY(list.length == 0);

    // pop
    VERIFY(!FF_LIST_POP(list, &n));
    VERIFY(list.length == 0);

    // add
    for (uint32_t i = 1; i <= FF_LIST_DEFAULT_ALLOC + 1; ++i) {
        *FF_LIST_ADD(uint32_t, list) = i;

        VERIFY(list.length == i);

        if (i <= FF_LIST_DEFAULT_ALLOC) {
            VERIFY(list.capacity == FF_LIST_DEFAULT_ALLOC);
        } else {
            VERIFY(list.capacity == FF_LIST_DEFAULT_ALLOC * 2);
        }

        VERIFY(*FF_LIST_GET(uint32_t, list, 0) == 1);
        VERIFY(*FF_LIST_GET(uint32_t, list, i - 1) == i);
    }
    VERIFY(list.length == FF_LIST_DEFAULT_ALLOC + 1);

    uint32_t sum = 0;
    // forEach
    FF_LIST_FOR_EACH (uint32_t, item, list) {
        sum += *item;
    }
    VERIFY(sum == (1 + FF_LIST_DEFAULT_ALLOC + 1) * (FF_LIST_DEFAULT_ALLOC + 1) / 2);

    // ffListFirstIndexComp
    n = 10;
    VERIFY(ffListFirstIndexComp(&list, sizeof(n), &n, (void*) numEqualsAdapter) == 9);
    n = 999;
    VERIFY(ffListFirstIndexComp(&list, sizeof(n), &n, (void*) numEqualsAdapter) == list.length);

    // ffListContains
    n = 10;
    VERIFY(FF_LIST_CONTAINS(list, &n, numEqualsAdapter));
    n = 999;
    VERIFY(!FF_LIST_CONTAINS(list, &n, numEqualsAdapter));

    // shift
    VERIFY(FF_LIST_SHIFT(list, &n));
    VERIFY(n == 1);
    VERIFY(list.length == FF_LIST_DEFAULT_ALLOC);
    VERIFY(*FF_LIST_GET(uint32_t, list, 0) == 2);
    VERIFY(*FF_LIST_GET(uint32_t, list, list.length - 1) == FF_LIST_DEFAULT_ALLOC + 1);

    // pop
    VERIFY(FF_LIST_POP(list, &n));
    VERIFY(n == FF_LIST_DEFAULT_ALLOC + 1);
    VERIFY(list.length == FF_LIST_DEFAULT_ALLOC - 1);
    VERIFY(*FF_LIST_GET(uint32_t, list, 0) == 2);
    VERIFY(*FF_LIST_GET(uint32_t, list, list.length - 1) == FF_LIST_DEFAULT_ALLOC);

    // insertAt
    ffListClear(&list);
    for (uint32_t i = 1; i <= 5; ++i) {
        *FF_LIST_ADD(uint32_t, list) = i;
    }
    // list = [1,2,3,4,5]

    {
        uint32_t v = 0;
        FF_LIST_INSERT_AT(uint32_t, list, 0, &v); // insert at head
    }
    VERIFY(list.length == 6);
    VERIFY(*FF_LIST_GET(uint32_t, list, 0) == 0);
    VERIFY(*FF_LIST_GET(uint32_t, list, 1) == 1);
    VERIFY(*FF_LIST_GET(uint32_t, list, 5) == 5);
    // list = [0,1,2,3,4,5]

    {
        uint32_t v = 99;
        FF_LIST_INSERT_AT(uint32_t, list, 3, &v); // insert in the middle
    }
    VERIFY(list.length == 7);
    VERIFY(*FF_LIST_GET(uint32_t, list, 2) == 2);
    VERIFY(*FF_LIST_GET(uint32_t, list, 3) == 99);
    VERIFY(*FF_LIST_GET(uint32_t, list, 4) == 3);
    VERIFY(*FF_LIST_GET(uint32_t, list, 6) == 5);
    // list = [0,1,2,99,3,4,5]

    {
        uint32_t v = 6;
        FF_LIST_INSERT_AT(uint32_t, list, list.length, &v); // insert at tail
    }
    VERIFY(list.length == 8);
    VERIFY(*FF_LIST_GET(uint32_t, list, 3) == 99);
    VERIFY(*FF_LIST_GET(uint32_t, list, 7) == 6);
    // list = [0,1,2,99,3,4,5,6]

    ffListClear(&list);
    {
        uint32_t v = 42;
        FF_LIST_INSERT_AT(uint32_t, list, 0, &v); // insert into empty list
    }
    VERIFY(list.length == 1);
    VERIFY(*FF_LIST_GET(uint32_t, list, 0) == 42);

    // removeAt
    FF_LIST_REMOVE_AT(uint32_t, list, 0); // remove the only element
    VERIFY(list.length == 0);

    for (uint32_t i = 1; i <= 5; ++i) {
        *FF_LIST_ADD(uint32_t, list) = i;
    }
    // list = [1,2,3,4,5]

    FF_LIST_REMOVE_AT(uint32_t, list, 0); // remove head
    VERIFY(list.length == 4);
    VERIFY(*FF_LIST_GET(uint32_t, list, 0) == 2);
    VERIFY(*FF_LIST_GET(uint32_t, list, list.length - 1) == 5);
    // list = [2,3,4,5]

    FF_LIST_REMOVE_AT(uint32_t, list, 1); // remove in the middle
    VERIFY(list.length == 3);
    VERIFY(*FF_LIST_GET(uint32_t, list, 0) == 2);
    VERIFY(*FF_LIST_GET(uint32_t, list, 1) == 4);
    VERIFY(*FF_LIST_GET(uint32_t, list, 2) == 5);
    // list = [2,4,5]

    FF_LIST_REMOVE_AT(uint32_t, list, list.length - 1); // remove tail
    VERIFY(list.length == 2);
    VERIFY(*FF_LIST_GET(uint32_t, list, 0) == 2);
    VERIFY(*FF_LIST_GET(uint32_t, list, 1) == 4);
    // list = [2,4]

    // Destroy
    ffListDestroy(&list);

    VERIFY(list.capacity == 0);
    VERIFY(list.length == 0);

    {
        FF_LIST_AUTO_DESTROY test = ffListCreate();
        VERIFY(test.capacity == 0);
        VERIFY(test.length == 0);
    }

    // Success
    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
}
