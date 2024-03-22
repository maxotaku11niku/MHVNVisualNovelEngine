//PC-98 hardware interrupts
//Maxim Hoxha 2023-2024

#pragma once

//#include "x86ports.h"
#include <dos.h>

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
#define setinterruptmask_primary(mask) outportb(0x02, mask)
#define setinterruptmask_secondary(mask) outportb(0x0A, mask)
//#define getinterruptmask_primary(mask) inportb(0x02, mask)
//#define getinterruptmask_secondary(mask) inportb(0x0A, mask)
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

//The following wrappers allow for compile time type checking

//Set the interrupt mask for the primary controller
inline void SetPrimaryInterruptMask(unsigned char mask)
{
    setinterruptmask_primary(mask);
}

//Set the interrupt mask for the secondary controller
inline void SetSecondaryInterruptMask(unsigned char mask)
{
    setinterruptmask_secondary(mask);
}

//Get the interrupt mask from the primary controller
inline unsigned char GetPrimaryInterruptMask()
{
    return inportb(0x02);
}

//Get the interrupt mask from the secondary controller
inline unsigned char GetSecondaryInterruptMask()
{
    return inportb(0x0A);
}

//You must not use INTERRUPTMASK(m) for these functions
inline void AddPrimaryInterrupts(const unsigned char mask)
{
    unsigned char maskBefore = GetPrimaryInterruptMask();
    maskBefore &= ~mask;
    SetPrimaryInterruptMask(maskBefore);
}

inline void AddSecondaryInterrupts(const unsigned char mask)
{
    unsigned char maskBefore = GetSecondaryInterruptMask();
    maskBefore &= ~mask;
    SetSecondaryInterruptMask(maskBefore);
}

inline void RemovePrimaryInterrupts(const unsigned char mask)
{
    unsigned char maskBefore = GetPrimaryInterruptMask();
    maskBefore |= mask;
    SetPrimaryInterruptMask(maskBefore);
}

inline void RemoveSecondaryInterrupts(const unsigned char mask)
{
    unsigned char maskBefore = GetSecondaryInterruptMask();
    maskBefore |= mask;
    SetSecondaryInterruptMask(maskBefore);
}
