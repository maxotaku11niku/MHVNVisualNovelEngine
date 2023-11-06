//DOS memory allocation calls wrapper
#pragma once

#include "doscalls.h"

void* memAlloc(int byteSize);
int memFree(const void* ptr);
int memRealloc(const void* ptr, int newSize);