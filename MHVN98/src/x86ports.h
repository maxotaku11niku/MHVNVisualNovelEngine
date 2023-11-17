//x86 port access intrinsics
#pragma once

//Port access, using 8-bit immediate address
#define outportb(port, val) asm inline ("out %b0, %1" : : "a" (val), "i" (port))
#define outportw(port, val) asm inline ("out %w0, %1" : : "a" (val), "i" (port))
#define outportd(port, val) asm inline ("out %k0, %1" : : "a" (val), "i" (port))

#define inportb(port, val) asm volatile ("in %1, %b0" : "=a" (val) : "i" (port))
#define inportw(port, val) asm volatile ("in %1, %w0" : "=a" (val) : "i" (port))
#define inportd(port, val) asm volatile ("in %1, %k0" : "=a" (val) : "i" (port))

//Port access, using 16-bit address in dx
#define outportb_ext(port, val) asm inline ("out %b0, %w1" : : "a" (val), "d" (port))
#define outportw_ext(port, val) asm inline ("out %w0, %w1" : : "a" (val), "d" (port))
#define outportd_ext(port, val) asm inline ("out %k0, %w1" : : "a" (val), "d" (port))

#define inportb_ext(port, val) asm volatile ("in %w1, %b0" : "=a" (val) : "d" (port))
#define inportw_ext(port, val) asm volatile ("in %w1, %w0" : "=a" (val) : "d" (port))
#define inportd_ext(port, val) asm volatile ("in %w1, %k0" : "=a" (val) : "d" (port))

//The following wrappers allow for compile time type checking

//Output a byte to the given I/O port (0x00 - 0xFF)
__attribute__((always_inline)) inline void PortOutB(unsigned char port, unsigned char val)
{
    outportb(port, val);
}

//Output a 16-bit number to the given I/O port (0x00 - 0xFF)
__attribute__((always_inline)) inline void PortOutW(unsigned char port, unsigned short val)
{
    outportw(port, val);
}

//Output a 32-bit number to the given I/O port (0x00 - 0xFF)
__attribute__((always_inline)) inline void PortOutD(unsigned char port, unsigned long val)
{
    outportd(port, val);
}

//Get a byte from the given I/O port (0x00 - 0xFF)
__attribute__((always_inline)) inline unsigned char PortInB(unsigned char port)
{
    unsigned char val;
    inportb(port, val);
    return val;
}

//Get a 16-bit number from the given I/O port (0x00 - 0xFF)
__attribute__((always_inline)) inline unsigned short PortInW(unsigned char port)
{
    unsigned short val;
    inportw(port, val);
    return val;
}

//Get a 32-bit number from the given I/O port (0x00 - 0xFF)
__attribute__((always_inline)) inline unsigned long PortInD(unsigned char port)
{
    unsigned long val;
    inportd(port, val);
    return val;
}

//Output a byte to the given I/O port (0x0000 - 0xFFFF)
__attribute__((always_inline)) inline void PortOutBExt(unsigned short port, unsigned char val)
{
    outportb_ext(port, val);
}

//Output a 16-bit number to the given I/O port (0x0000 - 0xFFFF)
__attribute__((always_inline)) inline void PortOutWExt(unsigned short port, unsigned short val)
{
    outportw_ext(port, val);
}

//Output a 32-bit number to the given I/O port (0x0000 - 0xFFFF)
__attribute__((always_inline)) inline void PortOutDExt(unsigned short port, unsigned long val)
{
    outportd_ext(port, val);
}

//Get a byte from the given I/O port (0x0000 - 0xFFFF)
__attribute__((always_inline)) inline unsigned char PortInBExt(unsigned short port)
{
    unsigned char val;
    inportb_ext(port, val);
    return val;
}

//Get a 16-bit number from the given I/O port (0x0000 - 0xFFFF)
__attribute__((always_inline)) inline unsigned short PortInWExt(unsigned short port)
{
    unsigned short val;
    inportw_ext(port, val);
    return val;
}

//Get a 32-bit number from the given I/O port (0x0000 - 0xFFFF)
__attribute__((always_inline)) inline unsigned long PortInDExt(unsigned short port)
{
    unsigned long val;
    inportd_ext(port, val);
    return val;
}