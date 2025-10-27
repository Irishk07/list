#ifndef STRING_FUNCTIONS_
#define STRING_FUNCTIONS_

#include <stdio.h>

#include "list.h"

void* my_recalloc(void* ptr, size_t new_size, size_t old_size);

void InitWithPoisons(type_t* ptr, size_t size);

#endif // STRING_FUNCTIONS_