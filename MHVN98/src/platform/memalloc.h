//DOS memory allocation calls wrapper
//Maxim Hoxha 2023-2024

#pragma once

#include "doscalls.h"

__far void* MemAlloc(unsigned long byteSize);
int MemFree(const __far void* ptr);
int MemRealloc(const __far void* ptr, unsigned long newSize);
