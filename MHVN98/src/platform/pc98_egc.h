//PC-98 EGC interface
//Maxim Hoxha 2023-2024

#pragma once

#include "pc98_gdc.h"

//OUTPORT 04A0 - EGC Plane Access Register
//For convenience, as hardware uses bit = 0 to indicate access, we NOT a mask where bit = 1 indicates access
inline void EGCSetPlaneAccess(unsigned short planemask)
{
    planemask = ~planemask;
    __asm volatile (
        "movw $0x04A0, %%dx\n\t"
        "out %w0, %%dx\n\t"
    : : "a" (planemask) : "%dx");
}

//OUTPORT 04A2 - EGC Pattern Data And Read Setting Register
//Set the source used for 'pattern data' and the source for single-plane reads
inline void EGCSetPatternAndReadSource(unsigned short mode)
{
    mode |= 0x00FF; //Apparently the lower 8 bits must be 1
    __asm volatile (
        "movw $0x04A2, %%dx\n\t"
        "out %w0, %%dx\n\t"
    : : "a" (mode) : "%dx");
}
//Supporting defines
#define EGC_PATTERNSOURCE_PATREG   0x0000
#define EGC_PATTERNSOURCE_BGCOLOUR 0x2000
#define EGC_PATTERNSOURCE_FGCOLOUR 0x4000
#define EGC_READSOURCE_PLANE(n) (((n) & 0x7) << 8)

//OUTPORT 04A4 - EGC Read/Write Mode Register
//Set how data is read from and written to VRAM
inline void EGCSetReadWriteMode(unsigned short mode)
{
    __asm volatile (
        "movw $0x04A4, %%dx\n\t"
        "out %w0, %%dx\n\t"
    : : "a" (mode) : "%dx");
}
//Supporting defines
#define EGC_READ_SINGLEPLANE 0x0000
#define EGC_READ_COMPARE     0x2000
#define EGC_WRITE_NOMODIFY   0x0000
#define EGC_WRITE_ROPSHIFT   0x0800
#define EGC_WRITE_PATSHIFT   0x1000
#define EGC_SOURCE_VRAM      0x0000
#define EGC_SOURCE_CPU       0x0400
#define EGC_PATSET_NONE      0x0000
#define EGC_PATSET_SOURCE    0x0100
#define EGC_PATSET_VRAM      0x0200
//The raster operation part is such that we can take these defines and then apply bitwise operations, which should represent logically what we want to do!
#define EGC_ROP_0   0x0000
#define EGC_ROP_1   0x00FF
#define EGC_ROP_NOP 0x00CC
#define EGC_ROP_SRC 0x00F0
#define EGC_ROP_DST 0x00CC
#define EGC_ROP_PAT 0x00AA
#define EGC_ROP(rop) ((rop) & 0x00FF)

//OUTPORT 04A6 - EGC Foreground Colour Register
//Sets the 'foreground' colour. Note that this won't immediately change anything on the screen, unfortunately
inline void EGCSetFGColour(unsigned short col)
{
    __asm volatile (
        "movw $0x04A6, %%dx\n\t"
        "out %w0, %%dx\n\t"
    : : "a" (col) : "%dx");
}

//OUTPORT 04A8 - EGC Mask Register
//Sets the mask for all write operations. For some reason though, if you do set it then you have to reset nearly every other register as well for some reason
inline void EGCSetMask(unsigned short mask)
{
    __asm volatile (
        "movw $0x04A8, %%dx\n\t"
        "out %w0, %%dx\n\t"
    : : "a" (mask) : "%dx");
}

//OUTPORT 04AA - EGC Background Colour Register
//Sets the 'background' colour. Note that this won't immediately change anything on the screen, unfortunately
inline void EGCSetBGColour(unsigned short col)
{
    __asm volatile (
        "movw $0x04AA, %%dx\n\t"
        "out %w0, %%dx\n\t"
    : : "a" (col) : "%dx");
}

//OUTPORT 04AC - EGC Bit Address And Block Transfer Register
//Sets the block transfer direction and both the source and destination bit addresses
//This is important for fast unaligned drawing, though if the source bit address is not 0 then you'll have to do a single word transfer before starting a line
inline void EGCSetBitAddressTransferDirection(unsigned short mode)
{
    __asm volatile (
        "movw $0x04AC, %%dx\n\t"
        "out %w0, %%dx\n\t"
    : : "a" (mode) : "%dx");
}
//Supporting defines
#define EGC_BLOCKTRANSFER_FORWARD  0x0000
#define EGC_BLOCKTRANSFER_BACKWARD 0x1000
#define EGC_BITADDRESS_DEST(n) (((n) & 0xF) << 4)
#define EGC_BITADDRESS_SRC(n) ((n) & 0xF)

//OUTPORT 04AE - EGC Bit Length Register
//There is an implicit +1 to the length, so we account for that
inline void EGCSetBitLength(unsigned short len)
{
    len--;
    __asm volatile (
        "movw $0x04AE, %%dx\n\t"
        "out %w0, %%dx\n\t"
    : : "a" (len) : "%dx");
}

//Clears the screen very fast using the EGC (must enable beforehand)
void EGCClearScreen();
//Clears some lines very fast using the EGC (must enable beforehand)
void EGCClearLines(unsigned short startLine, unsigned short numLines);
//Set some EGC registers to the right setting to allow a simple clear with the background colour
void EGCSetToBackgroundClearMode();
//Set some EGC registers to the right setting to allow shapes to be drawn in any single colour (the foreground colour) by using just data from the CPU
void EGCSetToMonochromeDrawMode();
//Set some EGC registers to the right setting to allow VRAM-to-VRAM copies
void EGCSetToVRAMBlit();
void EGCEnable();
void EGCDisable();
