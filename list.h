#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>

#define LIST_CHECK_AND_RETURN_ERRORS(error, ...)     \
        if (error != SUCCESS) {                       \
            fprintf(stderr, "Error is: %d\n", error);\
            __VA_ARGS__;                             \
            return error;                            \
        }

#define DUMP_INFO __LINE__, __func__, __FILE__

const int START_CAPACITY = 8;
const int MAX_CAPACITY   = 1e9;
const int MAX_LEN_NAME   = 100;
const int REALLOC_COEFF  = 2;

typedef int type_t;

struct About_elem {
    type_t value;
    size_t position;
};

struct List {
    type_t* data;
    size_t free;
    type_t* next;
    type_t* prev;
    size_t size;
    size_t capacity;
    FILE* file;
    int num_dump;
    About_elem about_elem;
};


enum list_status {
    SUCCESS                   = 0,
    NULL_POINTER_ON_STRUCT    = 1 << 0,
    NULL_POINTER_ON_DATA      = 1 << 1,
    NULL_POINTER_ON_NEXT      = 1 << 2,
    NULL_POINTER_ON_PREV      = 1 << 3,
    SIZE_BIGGER_THAN_CAPACITY = 1 << 4,
    INVALIDE_FREE             = 1 << 5,
    INVALIDE_HEAD             = 1 << 6,
    INVALIDE_TAIL             = 1 << 7,
    NOT_EXISTENS_ELEMENT      = 1 << 8,
    NOT_ENOUGH_MEMORY         = 1 << 9,
    OPEN_ERROR                = 1 << 10,
    INVALID_POSITION          = 1 << 11,
    CLOSE_ERROR               = 1 << 12,
    EXECUTION_FAILED          = 1 << 13,
    NULL_POITER_ON_DUMP_FILE  = 1 << 14,
    CAPACITY_IS_TOO_BIG       = 1 << 15
};


list_status ListCtor(List* list, const char* dump_filename);

void InitNextPrev(List* list);

list_status ListVerify(List* list);

list_status InsertElement(List* list, type_t elem, size_t position);

list_status ListResize(List* list, size_t old_capacity);

list_status DeleteElement(List* list, size_t position);
 
list_status GetElement(List* list, size_t position, type_t* elem);

list_status ListHTMLDump(List* list, const char* type_dump, int line, const char* func, const char* file);

list_status GenerateGraph(List* list);

list_status ListDtor(List* list);

#endif // LIST_H_