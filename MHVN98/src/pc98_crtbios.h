//PC-98 CRT BIOS calls
#pragma once

//INT 18 function 0A - Text Mode Set (with 'mode')
#define pc98crt_textmodeset(mode) asm volatile ("movb $10, %%ah\n\tmovb %0, %%al\n\tint $24" : : "rmi" (mode) : "ah", "al")
//Supporting defines
#define CRT_MODE_TEXT_25_ROWS    	0x00
#define CRT_MODE_TEXT_20_ROWS    	0x01
#define CRT_MODE_TEXT_80_COLUMNS 	0x00
#define CRT_MODE_TEXT_40_COLUMNS 	0x02
#define CRT_MODE_TEXT_KANJI      	0x00
#define CRT_MODE_TEXT_GRAPHIC    	0x04
#define CRT_MODE_TEXT_CGACCESS_CODE	0x00
#define CRT_MODE_TEXT_CGACCESS_DOT	0x08
//INT 18 function 0B - Text Mode Get (put in 'mode')
#define pc98crt_textmodeget(mode) asm volatile ("movb $11, %%ah\n\tint $24\n\tmovb %%al, %0" : "=rm" (mode) : : "ah")
//Supporting defines
#define CRT_MODE_CRTTYPE_STANDARD	0x00
#define CRT_MODE_CRTTYPE_HIGH    	0x80
//INT 18 function 0C - Turn Text Layer On
#define pc98crt_texton() asm volatile ("movb $12, %%ah\n\tint $24" : : : "ah")
//INT 18 function 0D - Turn Text Layer Off
#define pc98crt_textoff() asm volatile ("movb $13, %%ah\n\tint $24" : : : "ah")
//INT 18 function 40 - Turn Graphics Layer On
#define pc98crt_graphicon() asm volatile ("movb $64, %%ah\n\tint $24" : : : "ah")
//INT 18 function 41 - Turn Graphics Layer Off
#define pc98crt_graphicoff() asm volatile ("movb $65, %%ah\n\tint $24" : : : "ah")
//INT 18 function 42 - Graphics Mode Set
#define pc98crt_graphicmodeset(mode) asm volatile ("movb $66, %%ah\n\tmovb %0, %%ch\n\tint $24" : : "rmi" (mode) : "ah", "ch")
//Supporting defines
#define CRT_MODE_GRAPHIC_PAGE0      	0x00
#define CRT_MODE_GRAPHIC_PAGE1      	0x10
#define CRT_MODE_GRAPHIC_COLOUR     	0x00
#define CRT_MODE_GRAPHIC_MONOCHROME 	0x20
#define CRT_MODE_GRAPHIC_640x200_LOWER	0x40
#define CRT_MODE_GRAPHIC_640x200_UPPER	0x80
#define CRT_MODE_GRAPHIC_640x400    	0xC0

//The following wrappers allow for compile time type checking

//Sets the mode of the text layer
__attribute__((always_inline)) inline void textSetMode(unsigned char mode)
{
    pc98crt_textmodeset(mode);
}

//Gets the mode of the text layer
__attribute__((always_inline)) inline unsigned char textGetMode()
{
    unsigned char mode;
    pc98crt_textmodeset(mode);
    return mode;
}

//Turns the text layer on
__attribute__((always_inline)) inline void textOn()
{
    pc98crt_texton();
}

//Turns the text layer off
__attribute__((always_inline)) inline void textOff()
{
    pc98crt_textoff();
}

//Turns the graphics layer on
__attribute__((always_inline)) inline void graphicsOn()
{
    pc98crt_graphicon();
}

//Turns the graphics layer off
__attribute__((always_inline)) inline void graphicsOff()
{
    pc98crt_graphicoff();
}

//Sets the mode of the graphics layer
__attribute__((always_inline)) inline void graphicsSetMode(unsigned char mode)
{
    pc98crt_graphicmodeset(mode);
}