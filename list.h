#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>

#define LIST_CHECK_AND_RETURN_ERRORS(error, ...)                        \
        {                                                               \
            list_status now_error = error;                              \
            if (now_error != SUCCESS) {                                 \
                fprintf(stderr, "Error is: %d, %d\n", error, __LINE__); \
                __VA_ARGS__;                                            \
                return now_error;                                       \
            }                                                           \
        }

#define DUMP_AND_RETURN_ERRORS(error, ...)                                                                              \
        {                                                                                                               \
            list_status now_error = error;                                                                              \
            ListHTMLDump(list, {.value = DEFAULT_POISON, .physical_index = 0}, NULL, DUMP_INFO, ERROR_DUMP, now_error); \
            return now_error;                                                                                           \
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
    SUCCESS                     = 0,
    NULL_POINTER_ON_DATA        = 1,
    NULL_POINTER_ON_NEXT        = 2,
    NULL_POINTER_ON_PREV        = 3,
    SIZE_BIGGER_THAN_CAPACITY   = 4,
    INVALIDE_FREE               = 5,
    INVALIDE_HEAD               = 6,
    INVALIDE_TAIL               = 7,
    NOT_EXISTING_ELEMENT        = 8,
    NOT_ENOUGH_MEMORY           = 9,
    OPEN_ERROR                  = 10,
    INVALID_POSITION            = 11,
    CLOSE_ERROR                 = 12,
    EXECUTION_FAILED            = 13,
    NULL_POITER_ON_DUMP_FILE    = 14,
    CAPACITY_IS_TOO_BIG         = 15,
    CORRUPTED_CANARY            = 16,
    LIST_DATA_POISON            = 17,
    FREE_HAS_CYCLE              = 18,
    TEST_ERROR                  = 19,
    NEXT_HAS_CYCLE              = 20,
    PREV_HAS_CYCLE              = 21,
    NULL_POINTER_ON_STRUCT      = 22,
    NOT_ENOUGH_ELEMENTS_IN_NEXT = 23,
    NOT_ENOUGH_ELEMENTS_IN_PREV = 24,
    NOT_ENOUGH_ELEMENTS_IN_FREE = 25,
    UNEQUAL_CNT_ELEMENTS_NP     = 26,
    UNEQUAL_CNT_ELEMENTS_ND     = 27,
    UNEQUAL_CNT_ELEMENTS_PD     = 28
};

enum type_of_dump {
    INSERT_AFTER  = 0,
    DELETE        = 1,
    INSERT_BEFORE = 2,
    ERROR_DUMP    = 3
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

list_status ListHTMLDump(List* list, About_elem about_elem, const char* before_or_after, int line, const char* file, type_of_dump type_dump, list_status status);

list_status GenerateGraph(List* list);

void PrintErrors(int error, FILE* stream);

list_status ListDtor(List* list);

#endif // LIST_H_