//DOS memory allocation calls wrapper

#include "x86segments.h"
#include "doscalls.h"

//Allocates some memory (beware that DOS can only allocate blocks 16 bytes at a time, so please allocate sizeable data structures all at once rather than variable-by-variable)
void* memAlloc(int byteSize)
{
    int iserr;
    unsigned short codeSeg;
    getcs(codeSeg);
    unsigned short allocSeg = 0;
    unsigned short segSize = (byteSize + 0xF) >> 4;
    int21_memalloc(segSize, allocSeg, iserr);
    if (iserr)
    {
        return 0; //null pointer -> error
    }
    allocSeg -= codeSeg; //Compensate for flat addressing
    return allocSeg << 4;
}

//Frees previously allocated memory (so don't lose your pointers!)
int memFree(const void* ptr)
{
    int iserr;
    unsigned short codeSeg;
    getcs(codeSeg);
    unsigned short allocSeg = (unsigned short)((int)ptr >> 4);
    allocSeg += codeSeg; //Compensate for flat addressing
    int errcode;
    int21_memfree(allocSeg, errcode, iserr);
    if (iserr)
    {
        return errcode;
    }
    return 0;
}

//Reallocates memory previously allocated (currently only allows resizing in place, panics if relocation is necessary)
int memRealloc(const void* ptr, int newSize)
{
    int iserr;
    unsigned short codeSeg;
    getcs(codeSeg);
    unsigned short allocSeg = (unsigned short)((int)ptr >> 4);
    allocSeg += codeSeg; //Compensate for flat addressing
    int errcode;
    unsigned short segSize = (newSize + 0xF) >> 4;
    int21_memrealloc(segSize, allocSeg, errcode, iserr);
    if (iserr)
    {
        return errcode;
    }
    return 0;
}