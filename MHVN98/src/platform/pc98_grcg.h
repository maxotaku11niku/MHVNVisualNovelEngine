//PC-98 graphic charger interface
//Maxim Hoxha 2023-2024

#pragma once

//#include "x86ports.h"
#include <dos.h>
#include "pc98_gdc.h"

//OUTPORT 7C - Set GRCG Mode
#define grcg_writemode(mode) outportb(0x7C, mode)
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

//OUTPORT 7E - Set GRCG Tile Register (this cycles through the planes, so be careful when using it)
#define grcg_writetile(tile) outportb(0x7E, tile)

__attribute__((always_inline)) inline void GRCGEnable()
{
    grcg_writemode(GRCG_ENABLE);
}

__attribute__((always_inline)) inline void GRCGDisable()
{
    grcg_writemode(GRCG_DISABLE);
}

//Set all 4 tile registers correctly
__attribute__((always_inline)) inline void SetGRCGTileRegisters(unsigned char tile0, unsigned char tile1, unsigned char tile2, unsigned char tile3)
{
    grcg_writetile(tile0);
    grcg_writetile(tile1);
    grcg_writetile(tile2);
    grcg_writetile(tile3);
}

//Set the tile registers so they're ready for a simple fill
__attribute__((always_inline)) inline void SetGRCGTilesToColour(unsigned char col)
{
    grcg_writetile((col & 0x01)? 0xFF : 0x00);
    grcg_writetile((col & 0x02)? 0xFF : 0x00);
    grcg_writetile((col & 0x04)? 0xFF : 0x00);
    grcg_writetile((col & 0x08)? 0xFF : 0x00);
}

//The following wrappers allow for compile time type checking

//Sets the GRCG mode
__attribute__((always_inline)) inline void SetGRCGMode(unsigned char mode)
{
    grcg_writemode(mode);
}

//Clears the screen very fast using the GRCG (must enable beforehand)
void ClearScreenGRCG(unsigned char clearCol);
//Clears some lines very fast using the GRCG (must enable beforehand)
void ClearLinesGRCG(unsigned char clearCol, unsigned short startLine, unsigned short numLines);
