//Memory allocation calls wrapper
//Maxim Hoxha 2023-2024

#include "doscalls.h"

//Allocates some memory (beware that DOS can only allocate blocks 16 bytes at a time, so please allocate sizeable data structures all at once rather than variable-by-variable)
__far void* MemAlloc(unsigned long byteSize)
{
    return DOSMemAlloc((byteSize + 0xF) >> 4);
}

//Frees previously allocated memory (so don't lose your pointers!)
int MemFree(const __far void* ptr)
{
    return DOSMemFree(ptr);
}

//Reallocates memory previously allocated (currently only allows resizing in place, panics if relocation is necessary)
int MemRealloc(const __far void* ptr, unsigned long newSize)
{
    return DOSMemRealloc(ptr, (newSize + 0xF) >> 4);
}
