//DOS memory allocation calls wrapper
//Maxim Hoxha 2023-2024

/*/
#include "x86segments.h"
#include "doscalls.h"

//Allocates some memory (beware that DOS can only allocate blocks 16 bytes at a time, so please allocate sizeable data structures all at once rather than variable-by-variable)
__far void* MemAlloc(unsigned long byteSize)
{
    unsigned short allocSeg = 0;
    unsigned short segSize = (byteSize + 0xF) >> 4;
    int21_memalloc(segSize, allocSeg);
    return ((unsigned long)allocSeg) << 16;
}

//Frees previously allocated memory (so don't lose your pointers!)
int MemFree(const __far void* ptr)
{
    unsigned short allocSeg = (unsigned short)((unsigned long)ptr >> 16);
    int errcode;
    int21_memfree(allocSeg, errcode);
    return 0;
}

//Reallocates memory previously allocated (currently only allows resizing in place, panics if relocation is necessary)
int MemRealloc(const __far void* ptr, unsigned long newSize)
{
    unsigned short allocSeg = (unsigned short)((unsigned long)ptr >> 16);
    int errcode;
    unsigned short segSize = (newSize + 0xF) >> 4;
    int21_memrealloc(segSize, allocSeg, errcode);
    return 0;
}
//*/
