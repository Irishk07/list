#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>

#define LIST_CHECK_AND_RETURN_ERRORS(error, ...)     \
        if (error != SUCCES) {                       \
            fprintf(stderr, "Error is: %d\n", error) \
            __VA_ARGS__;                             \
            return error;                            \
        }


const int START_CAPACITY = 32 + 1;

typedef int type_t;


struct List {
    type_t* data;
    ssize_t free;
    type_t* next;
    type_t* prev;
    ssize_t size;
    ssize_t capacity;
};


enum list_status {
    SUCCES                        = 0,
    NULL_POINTER_ON_STRUCT        = 1 << 0,
    NULL_POINTER_ON_DATA          = 1 << 1,
    NULL_POINTER_ON_NEXT          = 1 << 2,
    NULL_POINTER_ON_PREV          = 1 << 3,
    SIZE_BIGGER_THAN_CAPACITY     = 1 << 4,
    NEGATIVE_SIZE                 = 1 << 5,
    NEGATIVE_CAPACITY             = 1 << 6,
    NEGATIVE_FREE                 = 1 << 7,
    NEGATIVE_HEAD                 = 1 << 8,
    NEGATIVE_TAIL                 = 1 << 9,
    TRY_TO_DELETE_DELETED_ELEMENT = 1 << 10
};


void ListCtor(List* list);

list_status ListVerify(List* list);

void ListDump(List* list);

list_status InsertElement(List* list, type_t elem, int position);

list_status DeleteElement(List* list, int position);

void GetElement();

void ListDtor(List* list);

#endif // LIST_H_