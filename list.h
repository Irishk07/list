#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>

#define LIST_CHECK_AND_RETURN_ERRORS(error, ...)     \
        if (error != SUCCES) {                       \
            fprintf(stderr, "Error is: %d\n", error);\
            __VA_ARGS__;                             \
            return error;                            \
        }

#define DUMP_INFO __LINE__, __func__, __FILE__

const int START_CAPACITY = 8 + 1;
const int MAX_LEN_NAME = 100;

typedef int type_t;


struct List {
    type_t* data;
    ssize_t free;
    type_t* next;
    type_t* prev;
    ssize_t size;
    ssize_t capacity;
    FILE* file;
    int num_dump;
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
    TRY_TO_DELETE_DELETED_ELEMENT = 1 << 10,
    NOT_ENOUGH_MEMORY             = 1 << 11,
    OPEN_ERROR                    = 1 << 12,
    INVALID_POSITION              = 1 << 13,
    CLOSE_ERROR                   = 1 << 14,
    EXECUTION_FAILED              = 1 << 15,
    NULL_POITER_ON_DUMP_FILE      = 1 << 16
};


list_status ListCtor(List* list, const char* dump_filename);

list_status ListVerify(List* list);

list_status InsertElement(List* list, type_t elem, int position);

list_status DeleteElement(List* list, int position);

list_status ListHTMLDump(List* list, int num_dump, const char* type_dump, int line, const char* func, const char* file);

list_status GenerateGraph(List* list, int num_dump);

list_status ListDtor(List* list);

#endif // LIST_H_