#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>

#define LIST_CHECK_AND_RETURN_ERRORS(error, ...)                   \
        if (error != SUCCESS) {                                    \
            fprintf(stderr, "Error is: %d, %d\n", error, __LINE__);\
            __VA_ARGS__;                                           \
            return error;                                          \
        }

#define DUMP_INFO __LINE__, __FILE__

const int START_CAPACITY = 8;
const int MAX_CAPACITY   = 1e9;
const int MAX_LEN_NAME   = 100;
const int REALLOC_COEFF  = 2;
const int DEFAULT_POISON = 0XDED;


typedef int type_t;

struct About_elem {
    type_t value;
    size_t physical_index;
};

struct List {
    type_t* data;
    size_t free;
    int* next;
    int* prev;
    size_t size;
    size_t capacity;
    FILE* dump_file;
    const char* directory;
    int num_dump;
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
    NOT_EXISTING_ELEMENT      = 1 << 8,
    NOT_ENOUGH_MEMORY         = 1 << 9,
    OPEN_ERROR                = 1 << 10,
    INVALID_POSITION          = 1 << 11,
    CLOSE_ERROR               = 1 << 12,
    EXECUTION_FAILED          = 1 << 13,
    NULL_POITER_ON_DUMP_FILE  = 1 << 14,
    CAPACITY_IS_TOO_BIG       = 1 << 15,
    CORRUPTED_CANARY          = 1 << 16,
    LIST_DATA_POISON          = 1 << 17,
    LIST_HAS_CYCLE            = 1 << 18,
    TEST_ERROR                = 1 << 19
};

enum function_name {
    INSERT_AFTER  = 0,
    DELETE        = 1,
    INSERT_BEFORE = 2
};


list_status ListCtor(List* list, const char* dump_filename, const char* directory);

void InitFreeSpace(List* list);

list_status ListVerify(List* list);

list_status InsertElementAfter(List* list, type_t elem, size_t physical_index);

list_status InsertElementBefore(List* list, type_t elem, size_t physical_index);

list_status InsertElementAfterNoDump(List* list, type_t elem, size_t physical_index);

list_status ListResize(List* list, size_t old_capacity);

list_status DeleteElement(List* list, size_t physical_index);
 
list_status GetElement(List* list, size_t physical_index, type_t* elem);

type_t ListHead(List* list);

type_t ListTail(List* list);

list_status ListHTMLDump(List* list, About_elem about_elem, const char* type_dump, int line, const char* file, function_name func_name);

list_status GenerateGraph(List* list);

list_status ListDtor(List* list);

#endif // LIST_H_