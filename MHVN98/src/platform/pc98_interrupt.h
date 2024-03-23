//PC-98 hardware interrupts
//Maxim Hoxha 2023-2024

#pragma once

//Primary interrupt vector indices
#define INTERRUPT_VECTOR_TIMER       0x08
#define INTERRUPT_VECTOR_KEYBOARD    0x09
#define INTERRUPT_VECTOR_VSYNC       0x0A
#define INTERRUPT_VECTOR_EXPANSION_0 0x0B
#define INTERRUPT_VECTOR_RS232C      0x0C
#define INTERRUPT_VECTOR_EXPANSION_1 0x0D
#define INTERRUPT_VECTOR_EXPANSION_2 0x0E
#define INTERRUPT_VECTOR_SECONDARY   0x0F
//Secondary interrupt vector indices
#define INTERRUPT_VECTOR_PRINTER      0x10
#define INTERRUPT_VECTOR_EXPANSION_3  0x11
#define INTERRUPT_VECTOR_EXPANSION_41 0x12
#define INTERRUPT_VECTOR_EXPANSION_42 0x13
#define INTERRUPT_VECTOR_EXPANSION_5  0x14
#define INTERRUPT_VECTOR_MOUSE        0x15
#define INTERRUPT_VECTOR_NDP          0x16

//The interrupt mask is 0 for allow, 1 for forbid
//Just OR together some masks and then use this to make the proper interrupt mask for the above macros
#define INTERRUPTMASK(m) (~(m))
//Primary interrupt masks
#define INTERRUPT_MASK_TIMER       0x01
#define INTERRUPT_MASK_KEYBOARD    0x02
#define INTERRUPT_MASK_VSYNC       0x04
#define INTERRUPT_MASK_EXPANSION_0 0x08
#define INTERRUPT_MASK_RS232C      0x10
#define INTERRUPT_MASK_EXPANSION_1 0x20
#define INTERRUPT_MASK_EXPANSION_2 0x40
#define INTERRUPT_MASK_SECONDARY   0x80
//Secondary interrupt masks
#define INTERRUPT_MASK_PRINTER      0x01
#define INTERRUPT_MASK_EXPANSION_3  0x02
#define INTERRUPT_MASK_EXPANSION_41 0x04
#define INTERRUPT_MASK_EXPANSION_42 0x08
#define INTERRUPT_MASK_EXPANSION_5  0x10
#define INTERRUPT_MASK_MOUSE        0x20
#define INTERRUPT_MASK_NDP          0x40

//Set the interrupt mask for the primary controller
inline void PC98SetPrimaryInterruptMask(unsigned char mask)
{
    volatile register unsigned char m __asm("%al");
    m = mask;
    __asm volatile (
        "outb %%al, $0x02"
    : : "a" (m));
}

//Set the interrupt mask for the secondary controller
inline void PC98SetSecondaryInterruptMask(unsigned char mask)
{
    volatile register unsigned char m __asm("%al");
    m = mask;
    __asm volatile (
        "outb %%al, $0x0A"
    : : "a" (m));
}

//Get the interrupt mask from the primary controller
inline unsigned char PC98GetPrimaryInterruptMask()
{
    volatile register unsigned char mask __asm("%al");
    __asm volatile (
        "inb $0x02, %%al"
    : "=a" (mask) : );
    return mask;
}

//Get the interrupt mask from the secondary controller
inline unsigned char PC98GetSecondaryInterruptMask()
{
    volatile register unsigned char mask __asm("%al");
    __asm volatile (
        "inb $0x0A, %%al"
    : "=a" (mask) : );
    return mask;
}

//You must not use INTERRUPTMASK(m) for these functions
inline void PC98AddPrimaryInterrupts(const unsigned char mask)
{
    unsigned char maskBefore = PC98GetPrimaryInterruptMask();
    maskBefore &= ~mask;
    PC98SetPrimaryInterruptMask(maskBefore);
}

inline void PC98AddSecondaryInterrupts(const unsigned char mask)
{
    unsigned char maskBefore = PC98GetSecondaryInterruptMask();
    maskBefore &= ~mask;
    PC98SetSecondaryInterruptMask(maskBefore);
}

inline void PC98RemovePrimaryInterrupts(const unsigned char mask)
{
    unsigned char maskBefore = PC98GetPrimaryInterruptMask();
    maskBefore |= mask;
    PC98SetPrimaryInterruptMask(maskBefore);
}

inline void PC98RemoveSecondaryInterrupts(const unsigned char mask)
{
    unsigned char maskBefore = PC98GetSecondaryInterruptMask();
    maskBefore |= mask;
    PC98SetSecondaryInterruptMask(maskBefore);
}
