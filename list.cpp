#include <stdlib.h>

#include "list.h"

void ListCtor(List* list) {
    list->capacity = START_CAPACITY;
    list->size     = 0;
    list->free     = 1;

    list->data = (type_t*)calloc((size_t)list->capacity, sizeof(type_t));
    list->next = (type_t*)calloc((size_t)list->capacity, sizeof(type_t));
    list->prev = (type_t*)calloc((size_t)list->capacity, sizeof(type_t));

    for (int i = 1; i < list->capacity ; ++i) {
        list->next[i] = i + 1;
        list->prev[i] = -1;
    }
    list->next[list->capacity - 1] = 0;
}

list_status ListVerify(List* list) {
    if (list == NULL)                return NULL_POINTER_ON_STRUCT;

    if (list->data == NULL)          return NULL_POINTER_ON_DATA;

    if (list->next == NULL)          return NULL_POINTER_ON_NEXT;

    if (list->prev == NULL)          return NULL_POINTER_ON_PREV;

    if (list->size > list->capacity) return SIZE_BIGGER_THAN_CAPACITY;

    if (list->size < 0)              return NEGATIVE_SIZE;

    if (list->capacity < 0)          return NEGATIVE_CAPACITY;

    if (list->free < 0)              return NEGATIVE_FREE;

    if (list->next[0] < 0)           return NEGATIVE_HEAD;

    if (list->prev[0] < 0)           return NEGATIVE_TAIL;

    return SUCCES;
} 

void ListDump(List* list) {
    fprintf(stderr, "Capacity: %zd\n", list->capacity);

    fprintf(stderr, "Size: %zd\n", list->size);

    fprintf(stderr, "Service: %d\n", list->data[0]);

    fprintf(stderr, "Head: %d\n", list->next[0]);
        
    fprintf(stderr, "Tail: %d\n", list->prev[0]);

    fprintf(stderr, "Free: %zd\n", list->free);

    fprintf(stderr, "Indexes | ");

    for (ssize_t i = 1; i < list->capacity; ++i) {
        fprintf(stderr, "%2zd | ", i);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "Data    | ");
    for (ssize_t i = 1; i < list->capacity; ++i) {
        fprintf(stderr, "%2d | ", list->data[i]);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "Next    | ");
    for (ssize_t i = 1; i < list->capacity; ++i) {
        fprintf(stderr, "%2d | ", list->next[i]);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "Prev    | ");
    for (ssize_t i = 1; i < list->capacity; ++i) {
        fprintf(stderr, "%2d | ", list->prev[i]);
    }
    fprintf(stderr, "\n\n");
}

// need insert element after position
list_status InsertElement(List* list, type_t elem, int position) { // TODO rename position maybe
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    list->data[list->free] = elem;

    type_t current_free = (type_t)list->free;
    list->free = list->next[list->free];

    // list->next[0] == head
    // list->prev[0] === tail
    if (list->size == 0) {
        list->next[0] = current_free;
        list->prev[0] = current_free;

        list->next[list->next[0]] = 0;
        list->prev[list->prev[0]] = 0;
    }

    // list->next[0] == head
    else if (position == 0) {
        list->prev[current_free]  = list->prev[list->next[0]];
        list->prev[list->next[0]] = current_free;

        list->next[current_free] = list->next[0];
        list->next[0] = current_free;
    }

    // list->prev[0] === tail
    else if (position == list->prev[0]) {
        list->next[current_free]  = list->next[list->prev[0]];
        list->next[list->prev[0]] = current_free;

        list->prev[current_free] = list->prev[0];
        list->prev[0] = current_free;
    }

    else {
        list->prev[list->next[position]] = current_free;
        list->prev[current_free] = position;

        list->next[current_free] = list->next[position];
        list->next[position] = current_free;
    }

    list->size++;

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    return SUCCES;
}

// need delete element from position
list_status DeleteElement(List* list, int position) {
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    if (list->size == 0 || list->prev[position] == -1) {
        LIST_CHECK_AND_RETURN_ERRORS(TRY_TO_DELETE_DELETED_ELEMENT);
    }

    if (position == list->next[0]) {
        list->next[0] = list->next[position];
    }

    if (position == list->prev[0]) {
        list->prev[0] = list->prev[position];
    }

    list->data[position] = 0;

    list->next[list->prev[position]] = list->next[position];
    list->prev[list->next[position]] = list->prev[position];

    list->prev[position] = -1;
    list->next[position] = (type_t)list->free;

    list->free = position; 

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    return SUCCES;
}


void ListDtor(List* list) {
    free(list->data);
    free(list->next);
    free(list->prev);
}