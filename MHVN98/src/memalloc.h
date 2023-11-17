//DOS memory allocation calls wrapper
#pragma once

#include "doscalls.h"

void* MemAlloc(unsigned long byteSize);
int MemFree(const void* ptr);
int MemRealloc(const void* ptr, unsigned long newSize);