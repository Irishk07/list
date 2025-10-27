#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "string_functions.h"

#include "list.h"

void* my_recalloc(void* ptr, size_t new_size, size_t old_size) {
    assert(ptr);
    assert(new_size > 0);
    assert(new_size < MAX_CAPACITY);
    assert(old_size > 0);
    assert(old_size < MAX_CAPACITY);

    char* temp = (char*)realloc(ptr, new_size);

    if (temp == NULL) {
        return NULL;
    }

    if (new_size <= old_size) {
        return temp;
    }

    memset(temp + old_size, 0, new_size - old_size);

    return temp;
}