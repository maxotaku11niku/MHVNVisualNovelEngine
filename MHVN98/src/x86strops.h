//x86 string instruction intrinsics
#pragma once

//16-bit pointers
//Memcpy routines (movs-based)
#define smemcpy8(src, dst, count) asm volatile ("movw %0, %%cx\n\tmovw %1, %%si\n\tmovw %2, %%di\n\trep movsb" : : "rmi" (count), "rmi" (src), "rmi" (dst) : "cx", "si", "di")
#define smemcpy16(src, dst, count) asm volatile ("movw %0, %%cx\n\tmovw %1, %%si\n\tmovw %2, %%di\n\trep movsw" : : "rmi" (count), "rmi" (src), "rmi" (dst) : "cx", "si", "di")
#define smemcpy32(src, dst, count) asm volatile ("movw %0, %%cx\n\tmovw %1, %%si\n\tmovw %2, %%di\n\trep movsl" : : "rmi" (count), "rmi" (src), "rmi" (dst) : "cx", "si", "di")
//Memset routines (stos-based)
#define smemset8(num, dst, count) asm volatile ("movw %0, %%cx\n\tmovw %1, %%di\n\trep stosb" : : "rmi" (count), "rmi" (dst), "a" (num) : "cx", "di")
#define smemset16(num, dst, count) asm volatile ("movw %0, %%cx\n\tmovw %1, %%di\n\trep stosw" : : "rmi" (count), "rmi" (dst), "a" (num) : "cx", "di")
#define smemset32(num, dst, count) asm volatile ("movw %0, %%cx\n\tmovw %1, %%di\n\trep stosl" : : "rmi" (count), "rmi" (dst), "a" (num) : "cx", "di")

//32-bit pointers
//Memcpy routines (movs-based)
#define wmemcpy8(src, dst, count) asm volatile ("movl %0, %%ecx\n\tmovl %1, %%esi\n\tmovl %2, %%edi\n\trep movsb (%%esi), (%%edi)" : : "rmi" (count), "rmi" (src), "rmi" (dst) : "ecx", "esi", "edi")
#define wmemcpy16(src, dst, count) asm volatile ("movl %0, %%ecx\n\tmovl %1, %%esi\n\tmovl %2, %%edi\n\trep movsw (%%esi), (%%edi)" : : "rmi" (count), "rmi" (src), "rmi" (dst) : "ecx", "esi", "edi")
#define wmemcpy32(src, dst, count) asm volatile ("movl %0, %%ecx\n\tmovl %1, %%esi\n\tmovl %2, %%edi\n\trep movsl (%%esi), (%%edi)" : : "rmi" (count), "rmi" (src), "rmi" (dst) : "ecx", "esi", "edi")
//Memset routines (stos-based)
#define wmemset8(num, dst, count) asm volatile ("movl %0, %%ecx\n\tmovl %1, %%edi\n\trep stosb (%%edi)" : : "rmi" (count), "rmi" (dst), "a" (num) : "ecx", "edi")
#define wmemset16(num, dst, count) asm volatile ("movl %0, %%ecx\n\tmovl %1, %%edi\n\trep stosw (%%edi)" : : "rmi" (count), "rmi" (dst), "a" (num) : "ecx", "edi")
#define wmemset32(num, dst, count) asm volatile ("movl %0, %%ecx\n\tmovl %1, %%edi\n\trep stosl (%%edi)" : : "rmi" (count), "rmi" (dst), "a" (num) : "ecx", "edi")

//The following wrappers allow for compile time type checking

//Fast copy memory 8 bits at a time, set segments before use
__attribute__((always_inline)) inline void memcpy8Seg(const void* src, void* dst, unsigned int count)
{
    smemcpy8(src, dst, count);
}

//Fast copy memory 16 bits at a time, set segments before use
__attribute__((always_inline)) inline void memcpy16Seg(const void* src, void* dst, unsigned int count)
{
    smemcpy16(src, dst, count);
}

//Fast copy memory 32 bits at a time, set segments before use
__attribute__((always_inline)) inline void memcpy32Seg(const void* src, void* dst, unsigned int count)
{
    smemcpy32(src, dst, count);
}

//Fast set memory 8 bits at a time, set segments before use
__attribute__((always_inline)) inline void memset8Seg(unsigned char num, void* dst, unsigned int count)
{
    smemset8(num, dst, count);
}

//Fast set memory 16 bits at a time, set segments before use
__attribute__((always_inline)) inline void memset16Seg(unsigned short num, void* dst, unsigned int count)
{
    smemset16(num, dst, count);
}

//Fast set memory 32 bits at a time, set segments before use
__attribute__((always_inline)) inline void memset32Seg(unsigned long num, void* dst, unsigned int count)
{
    smemset32(num, dst, count);
}

//Fast copy memory 8 bits at a time, uses 32-bit offsets
__attribute__((always_inline)) inline void memcpy8Flat(const void* src, void* dst, unsigned int count)
{
    wmemcpy8(src, dst, count);
}

//Fast copy memory 16 bits at a time, uses 32-bit offsets
__attribute__((always_inline)) inline void memcpy16Flat(const void* src, void* dst, unsigned int count)
{
    wmemcpy16(src, dst, count);
}

//Fast copy memory 32 bits at a time, uses 32-bit offsets
__attribute__((always_inline)) inline void memcpy32Flat(const void* src, void* dst, unsigned int count)
{
    wmemcpy32(src, dst, count);
}

//Fast set memory 8 bits at a time, uses 32-bit offsets
__attribute__((always_inline)) inline void memset8Flat(unsigned char num, void* dst, unsigned int count)
{
    wmemset8(num, dst, count);
}

//Fast set memory 16 bits at a time, uses 32-bit offsets
__attribute__((always_inline)) inline void memset16Flat(unsigned short num, void* dst, unsigned int count)
{
    wmemset16(num, dst, count);
}

//Fast set memory 32 bits at a time, uses 32-bit offsets
__attribute__((always_inline)) inline void memset32Flat(unsigned long num, void* dst, unsigned int count)
{
    wmemset32(num, dst, count);
}