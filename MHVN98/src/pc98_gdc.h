//PC-98 basic GDC interface
#pragma once

#include "x86ports.h"
//Plane segments
#define GDC_PLANE0_SEGMENT 0xA800
#define GDC_PLANE1_SEGMENT 0xB000
#define GDC_PLANE2_SEGMENT 0xB800
#define GDC_PLANE3_SEGMENT 0xE000
//Use this to signify that the planes are being accessed in parallel (GRCG, EGC etc.)
#define GDC_PLANES_SEGMENT 0xA800
//Plane flat address pointers
#define GDC_PLANE0 ((unsigned char*)0xA8000)
#define GDC_PLANE1 ((unsigned char*)0xB0000)
#define GDC_PLANE2 ((unsigned char*)0xB8000)
#define GDC_PLANE3 ((unsigned char*)0xE0000)
//Use this to signify that the planes are being accessed in parallel (GRCG, EGC etc.)
#define GDC_PLANES ((unsigned char*)0xA8000)

//OUTPORT 68 - Write GDC Mode 1
#define gdc_writemode1(mode) outportb(0x68, mode)
//Supporting defines

//INPORT 60 - Read GDC Text Status
#define gdc_readtextstatus(status) inportb(0x60, status)
//Supporting defines

//OUTPORT 64 - CRT Interrupt Reset
#define gdc_interruptreset() asm volatile ("out %%al, $100" : : : "al")
//OUTPORT 6C - Set Border Colour
#define gdc_setbordercolour(col) outportb(0x6C, col)
//OUTPORT 6A - Write GDC Mode 2
#define gdc_writemode2(mode) outportb(0x6A, mode)
//Supporting defines
//You probably don't want to use this mode, as the VX supports 16 colours anyway
#define GDC_8COLOURS 0x00
//The mode usually associated with the PC-98
#define GDC_16COLOURS 0x01

//INPORT A0 - Read GDC Graphics Status
#define gdc_readgraphicstatus(status) inportb(0xA0, status)
//OUTPORT A4 - Set Display Page
#define gdc_setdisplaypage(page) outportb(0xA4, page)
//OUTPORT A6 - Set Draw Page
#define gdc_setdrawpage(page) outportb(0xA6, page)

//Sets all 8 colours in the most basic palette. Only used if you're in 8-colour mode for some reason. Upper 4 bits for colours 0-3, lower 4 bits for colours 4-7.
inline void gdc_set8colourspalette(unsigned char col04, unsigned char col15, unsigned char col26, unsigned char col37)
{
	portOutB(0xAE, col04);
	portOutB(0xAA, col15);
	portOutB(0xAC, col26);
	portOutB(0xA8, col37);
}

//Sets a single colour in the usual 16 colour palette. Each parameter is in the range 0x0-0xF
inline void gdc_setpalettecolour(unsigned char index, unsigned char r, unsigned char g, unsigned char b)
{
	portOutB(0xA8, index);
	portOutB(0xAC, r);
	portOutB(0xAA, g);
	portOutB(0xAE, b);
}

//The following wrappers allow for compile time type checking

//Sets the colour mode of the graphics
__attribute__((always_inline)) inline void graphicsSetMode2(unsigned char mode)
{
	gdc_writemode2(mode);
}