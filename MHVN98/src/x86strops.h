//x86 string instruction intrinsics
#pragma once

//16-bit pointers
//Memcpy routines (movs-based)
#define smemcpy8(src, dst, count) asm volatile ("rep movsb" : "+c" (count), "+S" (src), "+D" (dst) : )
#define smemcpy16(src, dst, count) asm volatile ("rep movsw" : "+c" (count), "+S" (src), "+D" (dst) : )
#define smemcpy32(src, dst, count) asm volatile ("rep movsl" : "+c" (count), "+S" (src), "+D" (dst) : )
//Memset routines (stos-based)
#define smemset8(num, dst, count) asm volatile ("rep stosb" : "+c" (count), "+D" (dst) : "a" (num))
#define smemset16(num, dst, count) asm volatile ("rep stosw" : "+c" (count), "+D" (dst) : "a" (num))
#define smemset32(num, dst, count) asm volatile ("rep stosl" : "+c" (count), "+D" (dst) : "a" (num))

//32-bit pointers
//Memcpy routines (movs-based)
#define wmemcpy8(src, dst, count) asm volatile ("rep movsb (%%esi), (%%edi)" : "+c" (count), "+S" (src), "+D" (dst) : )
#define wmemcpy16(src, dst, count) asm volatile ("rep movsw (%%esi), (%%edi)" : "+c" (count), "+S" (src), "+D" (dst) : )
#define wmemcpy32(src, dst, count) asm volatile ("rep movsl (%%esi), (%%edi)" : "+c" (count), "+S" (src), "+D" (dst) : )
//Memset routines (stos-based)
#define wmemset8(num, dst, count) asm volatile ("rep stosb (%%edi)" : "+c" (count), "+D" (dst) : "a" (num))
#define wmemset16(num, dst, count) asm volatile ("rep stosw (%%edi)" : "+c" (count), "+D" (dst) : "a" (num))
#define wmemset32(num, dst, count) asm volatile ("rep stosl (%%edi)" : "+c" (count), "+D" (dst) : "a" (num))

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

//Fast copy memory with count always in bytes, set segments before use
__attribute__((always_inline)) inline void memcpySeg(const void* src, void* dst, unsigned int count)
{
	unsigned int mincount = count & 0x3;
    smemcpy8(src, dst, mincount);
	const unsigned char* movsrc = src; movsrc += mincount;
	const unsigned char* movdst = dst; movdst += mincount;
	unsigned int intcount = count >> 2;
	smemcpy32(movsrc, movdst, intcount);
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

//Fast set memory with count always in bytes, set segments before use, can only fill with the same byte
__attribute__((always_inline)) inline void memsetSeg(unsigned char num, void* dst, unsigned int count)
{
	num |= num << 8;
	num |= num << 16;
	unsigned int mincount = count & 0x3;
    smemset8(num, dst, mincount);
	const unsigned char* movdst = dst; movdst += mincount;
	unsigned int intcount = count >> 2;
	smemset32(num, movdst, intcount);
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

//Fast copy memory with count always in bytes, uses 32-bit offsets
__attribute__((always_inline)) inline void memcpyFlat(const void* src, void* dst, unsigned int count)
{
	unsigned int mincount = count & 0x3;
    wmemcpy8(src, dst, mincount);
	const unsigned char* movsrc = src; movsrc += mincount;
	const unsigned char* movdst = dst; movdst += mincount;
	unsigned int intcount = count >> 2;
	wmemcpy32(movsrc, movdst, intcount);
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

//Fast set memory with count always in bytes, uses 32-bit offsets, can only fill with the same byte
__attribute__((always_inline)) inline void memsetFlat(unsigned char num, void* dst, unsigned int count)
{
	num |= num << 8;
	num |= num << 16;
	unsigned int mincount = count & 0x3;
    wmemset8(num, dst, mincount);
	const unsigned char* movdst = dst; movdst += mincount;
	unsigned int intcount = count >> 2;
	wmemset32(num, movdst, intcount);
}