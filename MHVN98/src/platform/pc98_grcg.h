//PC-98 graphic charger interface
//Maxim Hoxha 2023-2024

#pragma once

#include "pc98_gdc.h"

//OUTPORT 7C - Set GRCG Mode
//
inline void GRCGWriteMode(unsigned char mode)
{
    volatile register unsigned char m __asm("%al");
    m = mode;
    __asm volatile (
        "outb %%al, $0x7C"
    : : "a" (m));
}
//Supporting defines
//For convenience, as hardware uses bit = 0 to indicate access, we NOT a mask where bit = 1 indicates access
#define GRCG_PLANEMASK(mask) (0x0F & (~(mask)))
//Writes the literal contents of tile registers upon CPU write (ignoring whatever is sent from the CPU)
#define GRCG_MODE_TDW 0x00
//Uses the CPU data as a bitmask (this will be slower, so don't use it when you can use TDW)
#define GRCG_MODE_RMW 0x40
//Upon reading from VRAM, compares the colours in the tile register with the colours in VRAM, and sends a 1 to the CPU for every matching pixel in the block
#define GRCG_MODE_TCR 0x00
#define GRCG_DISABLE  0x00
#define GRCG_ENABLE   0x80

inline void GRCGEnable()
{
    GRCGWriteMode(GRCG_ENABLE);
}

inline void GRCGDisable()
{
    GRCGWriteMode(GRCG_DISABLE);
}

//OUTPORT 7E - Set GRCG Tile Register
//Since this register cycles through the planes, it's not ideal to write to it directly, so here are some convenience functions
//We're assuming the GRCG is already in a sane state, mind you. It should be if we've just turned it on
//Set all 4 tile registers correctly
inline void GRCGSetTileRegisters(unsigned char tile0, unsigned char tile1, unsigned char tile2, unsigned char tile3)
{
    __asm volatile (
        "movb %0, %%al\n\t"
        "outb %%al, $0x7E\n\t"
        "movb %1, %%al\n\t"
        "outb %%al, $0x7E\n\t"
        "movb %2, %%al\n\t"
        "outb %%al, $0x7E\n\t"
        "movb %3, %%al\n\t"
        "outb %%al, $0x7E\n\t"
    : : "rmi" (tile0), "rmi" (tile1), "rmi" (tile2), "rmi" (tile3) : "%al");
}

//Set the tile registers so they're ready for a simple fill
inline void GRCGSetTilesToColour(unsigned char col)
{
    unsigned char t0 = (col & 0x01) ? 0xFF : 0x00;
    unsigned char t1 = (col & 0x02) ? 0xFF : 0x00;
    unsigned char t2 = (col & 0x04) ? 0xFF : 0x00;
    unsigned char t3 = (col & 0x08) ? 0xFF : 0x00;
    GRCGSetTileRegisters(t0, t1, t2, t3);
}

//Clears the screen very fast using the GRCG (must enable beforehand)
void GRCGClearScreen(unsigned char clearCol);
//Clears some lines very fast using the GRCG (must enable beforehand)
void GRCGClearLines(unsigned char clearCol, unsigned short startLine, unsigned short numLines);
