#ifndef CANARY_H_
#define CANARY_H_

#include <stdio.h>

#include "list.h"


#ifdef CANARIES
const size_t CNT_CANARIES = 2;
#else
const size_t CNT_CANARIES = 0;
#endif // _CANARY

#ifdef CANARIES
#define ON_CANARY(...) __VA_ARGS__
#else // NOT CANARIES
#define ON_CANARY(...)
#endif // CANARIES


const int CANARY = 0XEDA;


void SettingCanariesToBegin(type_t* ptr);

void SettingCanariesToEnd(type_t* ptr, size_t capacity);

size_t RealSizeList(size_t capacity, size_t count_canaries);

size_t OffsetDueCanaries(size_t count_canaries);

size_t OffsetToLastElement(size_t size, size_t count_canaries);

size_t OffsetToNewElement(size_t size, size_t count_canaries);


#endif //CANARY_H_