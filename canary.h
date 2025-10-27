#ifndef CANARY_H_
#define CANARY_H_

#include <stdio.h>

#include "list.h"


const int CANARY = 0XEDA; // TODO: what if type_t is not an integer type?


void SettingCanariesToBegin(type_t* ptr);

void SettingCanariesToEnd(type_t* ptr, size_t capacity);

size_t RealSizeList(size_t capacity, size_t count_canaries);

size_t OffsetDueCanaries(size_t count_canaries);

size_t OffsetToLastElement(size_t size, size_t count_canaries);

size_t OffsetToNewElement(size_t size, size_t count_canaries);


#endif //CANARY_H_