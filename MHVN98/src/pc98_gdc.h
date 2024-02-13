//PC-98 basic GDC interface
#pragma once

//#include "x86ports.h"
#include <dos.h>
//Plane segments
#define GDC_PLANE0_SEGMENT 0xA800
#define GDC_PLANE1_SEGMENT 0xB000
#define GDC_PLANE2_SEGMENT 0xB800
#define GDC_PLANE3_SEGMENT 0xE000
//Use this to signify that the planes are being accessed in parallel (GRCG, EGC etc.)
#define GDC_PLANES_SEGMENT 0xA800
//Plane address pointers
#define GDC_PLANE0 ((unsigned __far char*)0xA8000000)
#define GDC_PLANE1 ((unsigned __far char*)0xB0000000)
#define GDC_PLANE2 ((unsigned __far char*)0xB8000000)
#define GDC_PLANE3 ((unsigned __far char*)0xE0000000)
//Use this to signify that the planes are being accessed in parallel (GRCG, EGC etc.)
#define GDC_PLANES ((unsigned __far char*)0xA8000000)

//OUTPORT 68 - Write GDC Mode 1
#define gdc_writemode1(mode) outportb(0x68, mode)
//Supporting defines
#define GDC_MODE1_ATTRIBUTE4_VERTICALLINE 0x00
#define GDC_MODE1_ATTRIBUTE4_PATTERN      0x01
#define GDC_MODE1_COLOUR                  0x02
#define GDC_MODE1_MONOCHROME              0x03
#define GDC_MODE1_80COLUMN                0x04
#define GDC_MODE1_40COLUMN                0x05
#define GDC_MODE1_6x8CHARS                0x06
#define GDC_MODE1_7x13CHARS               0x07
#define GDC_MODE1_LINEDOUBLE_OFF          0x08
#define GDC_MODE1_LINEDOUBLE_ON           0x09
#define GDC_MODE1_CHARACCESS_CODE         0x0A
#define GDC_MODE1_CHARACCESS_BITMAP       0x0B
#define GDC_MODE1_MEMSWITCH_WRITE_ON      0x0C
#define GDC_MODE1_MEMSWITCH_WRITE_OFF     0x0D
#define GDC_MODE1_DISPLAY_ON              0x0E
#define GDC_MODE1_DISPLAY_OFF             0x0F

//OUTPORT 60 - Write GDC Text Command Parameter
#define gdc_writetextcommandparam(param) outportb(0x60, param)
//OUTPORT 62 - Write GDC Text Command
#define gdc_writetextcommand(command) outportb(0x62, command)
//Supporting defines (all of these commands will be abstracted behind proper functions)
//Reinitialises this GDC
#define GDC_COMMAND_RESET 0x00
//Sets display timing parameters, which can be VERY dangerous. If you don't know what you're doing, just use the BIOS functions.
#define GDC_COMMAND_SYNC_OFF 0x0E
#define GDC_COMMAND_SYNC_ON  0x0F
//Start display of this layer
#define GDC_COMMAND_START 0x0D
//Stop display of this layer
#define GDC_COMMAND_STOP 0x0C
//Sets zoom values
#define GDC_COMMAND_ZOOM 0x46
//Sets which portions of the screen get displayed and which VRAM portions they correspond to, useful for hardware scrolling
#define GDC_COMMAND_SCROLL(numsec) (0x70 | (numsec))
//Sets the text cursor format
#define GDC_COMMAND_CSRFORM 0x4B
//Sets the number of words per line
#define GDC_COMMAND_PITCH 0x47
//Reads out the position of the light pen
#define GDC_COMMAND_LPEN 0xC0
//Prepares a draw command
#define GDC_COMMAND_VECTW 0x4C
//Executes a draw command
#define GDC_COMMAND_VECTE 0x6C
//Prepares a little pattern
#define GDC_COMMAND_TEXTW 0x78
//Draws the previously prepared pattern
#define GDC_COMMAND_TEXTE 0x68
//Sets the position of the cursor/drawing start position
#define GDC_COMMAND_CSRW 0x49
//Reads out the position of the cursor/drawing start position
#define GDC_COMMAND_CSRR 0xE0
//Sets the write mask register
#define GDC_COMMAND_MASK 0x4A
//Sets the write operation of any draw to VRAM
#define GDC_COMMAND_WRITE(mod) (0x20 | (mod))
//Sets the read operation of any read from VRAM
#define GDC_COMMAND_READ(mod) (0xA0 | (mod))
//Useless on the PC-98
#define GDC_COMMAND_WRITE_DMA(mod) (0x24 | (mod))
#define GDC_COMMAND_READ_DMA(mod) (0xA4 | (mod))
//GDC read/write mods
//DST = SRC
#define GDC_MOD_REPLACE 0x00
//DST = SRC XOR DST
#define GDC_MOD_XOR 0x01
//DST = !SRC AND DST
#define GDC_MOD_CLEAR 0x02
//DST = (!SRC AND DST) OR SRC
#define GDC_MOD_SET 0x03

//INPORT 60 - Read GDC Text Status
//#define gdc_readtextstatus(status) inportb(0x60, status)
//Supporting defines
#define GDC_STATUS_DATAREADY  0x01
#define GDC_STATUS_FIFO_FULL  0x02
#define GDC_STATUS_FIFO_EMPTY 0x04
//Only makes sense for the graphics GDC
#define GDC_STATUS_DRAWING 0x08
//Not applicable to the PC-98
#define GDC_STATUS_DMA_EXECUTE 0x10
//Please just use an interrupt if you need to VSYNC something
#define GDC_STATUS_VBLANK 0x20
//There is no HSYNC interrupt, however, so feel free to poll for HSYNC if you need to (or use a timer closely synced to the line frequency)
#define GDC_STATUS_HBLANK 0x40
//Not applicable to the PC-98 (and even if so, actually requires a CRT anyway)
#define GDC_STATUS_LIGHTPEN_DETECT 0x80
//INPORT 62 - Read GDC Text Command Data
//#define gdc_readtextcommanddata(data) inportb(0x62, data)


//OUTPORT 64 - CRT Interrupt Reset
#define gdc_interruptreset() __asm volatile ("out %al, $100")
//OUTPORT 6C - Set Border Colour
#define gdc_setbordercolour(col) outportb(0x6C, col)
//OUTPORT 6A - Write GDC Mode 2
#define gdc_writemode2(mode) outportb(0x6A, mode)
//Supporting defines
//You probably don't want to use this mode, as the VX supports 16 colours anyway
#define GDC_MODE2_8COLOURS 0x00
//The mode usually associated with the PC-98, though does require a model that supports it
#define GDC_MODE2_16COLOURS 0x01
//Requires the EGC (put a GDC_MODE2_MODIFY through first)
#define GDC_MODE2_GRCG 0x04
#define GDC_MODE2_EGC  0x05
//Requires a suitable hardware feature that can be modified through port 6A, otherwise these are useless
#define GDC_MODE2_NOMODIFY 0x06
#define GDC_MODE2_MODIFY   0x07
//Requires 256-colour hardware (either MATE/MULTI builtin, or H98, put a GDC_MODE2_MODIFY through first)
#define GDC_MODE2_256COLOURS_OFF  0x20
#define GDC_MODE2_256COLOURS_ON   0x21
#define GDC_MODE2_PAGE_DISCONNECT 0x68
#define GDC_MODE2_PAGE_CONNECT    0x69

//OUTPORT A0 - Write GDC Graphics Command Parameter
#define gdc_writegraphiccommandparam(param) outportb(0xA0, param)
//OUTPORT A2 - Write GDC Graphics Command
#define gdc_writegraphiccommand(command) outportb(0xA2, command)
//OUTPORT A4 - Set Display Page
#define gdc_setdisplaypage(page) outportb(0xA4, page)
//OUTPORT A6 - Set Draw Page
#define gdc_setdrawpage(page) outportb(0xA6, page)

//INPORT A0 - Read GDC Graphics Status
//#define gdc_readgraphicstatus(status) inportb(0xA0, status)
//INPORT A2 - Read GDC Graphics Command Data
//#define gdc_readgraphiccommanddata(data) inportb(0xA2, data)


//Sets all 8 colours in the most basic palette. Only used if you're in 8-colour mode for some reason. Upper 4 bits for colours 0-3, lower 4 bits for colours 4-7.
inline void GDCSet8ColoursPalette(unsigned char col04, unsigned char col15, unsigned char col26, unsigned char col37)
{
    __asm volatile (
        "movb %0, %%al\n\t"
        "out %%al, $0xAE\n\t"
        "movb %1, %%al\n\t"
        "out %%al, $0xAA\n\t"
        "movb %2, %%al\n\t"
        "out %%al, $0xAC\n\t"
        "movb %3, %%al\n\t"
        "out %%al, $0xA8\n\t"
        : : "rmi" (col04), "rmi" (col15), "rmi" (col26), "rmi" (col37) );
}

//Sets a single colour in the usual 16 colour palette. Each parameter is in the range 0x0-0xF
//If in 256-colour mode, each parameter is in the range 0x00-0xFF
inline void GDCSetPaletteColour(unsigned char index, unsigned char r, unsigned char g, unsigned char b)
{
    __asm volatile (
        "movb %0, %%al\n\t"
        "out %%al, $0xA8\n\t"
        "movb %1, %%al\n\t"
        "out %%al, $0xAC\n\t"
        "movb %2, %%al\n\t"
        "out %%al, $0xAA\n\t"
        "movb %3, %%al\n\t"
        "out %%al, $0xAE\n\t"
        : : "rmi" (index), "rmi" (r), "rmi" (g), "rmi" (b) );
}

//The following wrappers allow for compile time type checking

//Sets the graphics mode first part
inline void GraphicsSetMode1(unsigned char mode)
{
    gdc_writemode1(mode);
}

//Sets the graphics mode second part
inline void GraphicsSetMode2(unsigned char mode)
{
    gdc_writemode2(mode);
}

//Resets the text GDC
inline void ResetTextGDC()
{
    gdc_writetextcommand(GDC_COMMAND_RESET);
}

//Start displaying the text layer
inline void StartTextGDC()
{
    gdc_writetextcommand(GDC_COMMAND_START);
}

//Stop displaying the text layer
inline void StopTextGDC()
{
    gdc_writetextcommand(GDC_COMMAND_STOP);
}

inline unsigned char ReadTextGDCStatus()
{
    return inportb(0x60);
}

//Resets the graphics GDC
inline void ResetGraphicsGDC()
{
    gdc_writegraphiccommand(GDC_COMMAND_RESET);
}

//Start displaying the graphics layer
inline void StartGraphicsGDC()
{
    gdc_writegraphiccommand(GDC_COMMAND_START);
}

//Stop displaying the graphics layer
inline void StopGraphicsGDC()
{
    gdc_writegraphiccommand(GDC_COMMAND_STOP);
}

inline unsigned char ReadGraphicsGDCStatus()
{
    return inportb(0xA0);
}
