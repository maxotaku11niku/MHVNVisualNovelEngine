//Wrapper for Trixter's 8086 assembly LZ4 decompression routine

#include "platform/x86segments.h"

//source must point to the 32-bit length of the block followed by the LZ4-compressed data
unsigned int LZ4Decompress(__far unsigned char* dest, __far const unsigned char* source)
{
    unsigned short destSeg = (unsigned short)((((unsigned long)(dest)) >> 16) & 0xFFFF);
    unsigned short destOffs = (unsigned short)(((unsigned long)(dest)) & 0xFFFF);
    unsigned short srcSeg = (unsigned short)((((unsigned long)(source)) >> 16) & 0xFFFF);
    unsigned short srcOffs = (unsigned short)(((unsigned long)(source)) & 0xFFFF);
    unsigned int length;
    SetES(destSeg);
    SetDS(srcSeg);
    //This function trashes a lot of registers, so we save and restore them here
    __asm volatile (
        "push %%bp\n\t"
        "push %%bx\n\t"
        "push %%cx\n\t"
        "push %%dx\n\t"
        "call lz4_decompress\n\t"
        "pop %%dx\n\t"
        "pop %%cx\n\t"
        "pop %%bx\n\t"
        "pop %%bp\n\t"
    : "=a" (length), "+D" (destOffs), "+S" (srcOffs) : );
    SetDS(GetSS());
    return length;
}
