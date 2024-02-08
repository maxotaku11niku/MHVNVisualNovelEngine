/*/
#pragma once

#include "unrealhwaddr.h"
#include "x86segments.h"
#include "x86interrupt.h"

//Get the raw entry of an interrupt vector
__attribute__((always_inline)) inline unsigned long GetInterruptFunctionRaw(unsigned char num)
{
    return ivt_relptr[num];
}

//Set the raw entry of an interrupt vector
__attribute__((always_inline)) inline void SetInterruptFunctionRaw(unsigned char num, unsigned long ptr)
{
    ivt_relptr[num] = ptr;
}

//Set the entry of an interrupt vector with a function pointer in the same segment
__attribute__((always_inline)) inline void SetInterruptFunction(unsigned char num, InterruptFuncPtr ptr)
{
    unsigned short codeSeg;
    getcs(codeSeg);
    ivt_relptr[num] = ((unsigned long)ptr & 0x0000FFFF) | ((unsigned long)codeSeg << 16);
}
//*/
