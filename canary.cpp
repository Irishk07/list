#include <assert.h>
#include <stdio.h>

#include "canary.h"

#include "list.h"


void SettingCanariesToBegin(type_t* ptr) {
    assert(ptr);
    
    *ptr = (type_t)CANARY;
}

void SettingCanariesToEnd(type_t* ptr, size_t capacity) {
    assert(ptr);
    assert(capacity > 0);
    assert(capacity < MAX_CAPACITY);

    *(ptr + capacity + 1) = (type_t)CANARY;
}

size_t RealSizeList(size_t capacity, size_t count_canaries) {
    return (capacity + count_canaries);
}

size_t OffsetDueCanaries(size_t count_canaries) {
    return (count_canaries == 0) ? 0 : 1;
}

size_t OffsetToLastElement(size_t size, size_t count_canaries) {
    return size + OffsetDueCanaries(count_canaries) - 1;
}

size_t OffsetToNewElement(size_t size, size_t count_canaries) {
    return size + OffsetDueCanaries(count_canaries);
}