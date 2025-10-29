#include "test.h"

#include "canary.h"
#include "list.h"
#include "string_functions.h"

#include "list.h"
#include "canary.h"
#include "string_functions.h"
#include <assert.h>

list_status Tests() {
    LIST_CHECK_AND_RETURN_ERRORS(Test1());
    LIST_CHECK_AND_RETURN_ERRORS(Test2());
    LIST_CHECK_AND_RETURN_ERRORS(Test3());
    LIST_CHECK_AND_RETURN_ERRORS(Test4());

    return SUCCESS;
}

list_status Test1() {
    List list = {};
    LIST_CHECK_AND_RETURN_ERRORS(ListCtor(&list, "test1.html", "tests/test1"));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 1, 0), ListDtor(&list););
    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, 1), ListDtor(&list););

    const type_t correct[] = {DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON};

    for (size_t i = 1; i <= list.capacity; ++i) {
        if (correct[i - 1] != list.data[i]) {
            fprintf(stderr, "%d != %d at position %zu in test 1\n", correct[i - 1], list.data[i], i);

            return TEST_ERROR;
        }
    }

    LIST_CHECK_AND_RETURN_ERRORS(ListDtor(&list));

    printf("Test1: All right\n");

    return SUCCESS;
} 

list_status Test2() {
    List list = {};
    LIST_CHECK_AND_RETURN_ERRORS(ListCtor(&list, "test2.html", "tests/test2"));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 2, 0));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 3, 1));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementBefore(&list, 1, 2));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 4, 3));
    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, (size_t)ListTail(&list)));
    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, (size_t)ListHead(&list)));

    const type_t correct[] = {DEFAULT_POISON, DEFAULT_POISON, 1, 4, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON};

    for (size_t i = 1; i <= list.capacity; ++i) {
        if (correct[i - 1] != list.data[i]) {
            fprintf(stderr, "%d != %d at position %zu in test 2\n", correct[i - 1], list.data[i], i);

            return TEST_ERROR;
        }
    }

    LIST_CHECK_AND_RETURN_ERRORS(ListDtor(&list));

    printf("Test2: All right\n");

    return SUCCESS;
}

list_status Test3() {
    List list = {};
    LIST_CHECK_AND_RETURN_ERRORS(ListCtor(&list, "test3.html", "tests/test3"));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 2, 0));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 3, 1));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementBefore(&list, 1, 1));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 4, 3));
    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, 1));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 100, (size_t)ListTail(&list)));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementBefore(&list, 101, (size_t)ListHead(&list)));

    const type_t correct[] = {100, 3, 1, 4, 101, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON};

    for (size_t i = 1; i <= list.capacity; ++i) {
        if (correct[i - 1] != list.data[i]) {
            printf("%d != %d at position %zu in test 3\n", correct[i - 1], list.data[i], i);

            return TEST_ERROR;
        }
    }

    LIST_CHECK_AND_RETURN_ERRORS(ListDtor(&list));

    printf("Test3: All right\n");

    return SUCCESS;
}

list_status Test4() {
    List list = {};
    LIST_CHECK_AND_RETURN_ERRORS(ListCtor(&list, "test4.html", "tests/test4"));

    for (size_t i = 0; i < 9; ++i) {
        LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, (type_t)i + 1, i));
    }

    const type_t correct[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON, DEFAULT_POISON};

    for (size_t i = 1; i <= list.capacity; ++i) {
        if (correct[i - 1] != list.data[i]) {
            printf("%d != %d at position %zu in test 4\n", correct[i - 1], list.data[i], i);

            return TEST_ERROR;
        }
    }

    LIST_CHECK_AND_RETURN_ERRORS(ListDtor(&list));

    printf("Test4: All right\n");

    return SUCCESS;
}