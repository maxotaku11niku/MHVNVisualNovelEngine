//PC-98 CRT BIOS calls
//Maxim Hoxha 2023-2024

#pragma once

//INT 18 function 0A - Text Mode Set (with 'mode')
//Sets the mode of the text layer
inline void PC98BIOSTextSetMode(unsigned char mode)
{
    register volatile unsigned char m __asm("%al");
    m = mode;
    __asm volatile (
        "movb $0x0A, %%ah\n\t"
        "int $0x18\n\t"
    : : "a" (m));
}
//Supporting defines
#define CRT_MODE_TEXT_25_ROWS       0x00
#define CRT_MODE_TEXT_20_ROWS       0x01
#define CRT_MODE_TEXT_80_COLUMNS    0x00
#define CRT_MODE_TEXT_40_COLUMNS    0x02
#define CRT_MODE_TEXT_KANJI         0x00
#define CRT_MODE_TEXT_GRAPHIC       0x04
#define CRT_MODE_TEXT_CGACCESS_CODE 0x00
#define CRT_MODE_TEXT_CGACCESS_DOT  0x08

//INT 18 function 0B - Text Mode Get (put in 'mode')
//Gets the mode of the text layer
inline unsigned char PC98BIOSTextGetMode()
{
    register volatile unsigned char mode __asm("%al");
    __asm volatile (
        "movb $0x0B, %%ah\n\t"
        "int $0x18\n\t"
    : "=a" (mode) : );
    return mode;
}
//Supporting defines
#define CRT_MODE_CRTTYPE_STANDARD 0x00
#define CRT_MODE_CRTTYPE_HIGH     0x80

//INT 18 function 0C - Turn Text Layer On
//Turns the text layer on
inline void PC98BIOSTextOn()
{
    __asm volatile (
        "movb $0x0C, %%ah\n\t"
        "int $0x18\n\t"
    : : : "%ah");
}

//INT 18 function 0D - Turn Text Layer Off
//Turns the text layer off
inline void PC98BIOSTextOff()
{
    __asm volatile (
        "movb $0x0D, %%ah\n\t"
        "int $0x18\n\t"
    : : : "%ah");
}

//INT 18 function 40 - Turn Graphics Layer On
//Turns the graphics layer on
inline void PC98BIOSGraphicsOn()
{
    __asm volatile (
        "movb $0x40, %%ah\n\t"
        "int $0x18\n\t"
    : : : "%ah");
}

//INT 18 function 41 - Turn Graphics Layer Off
//Turns the graphics layer off
inline void PC98BIOSGraphicsOff()
{
    __asm volatile (
        "movb $0x41, %%ah\n\t"
        "int $0x18\n\t"
    : : : "%ah");
}

//INT 18 function 42 - Graphics Mode Set
//Sets the mode of the graphics layer
inline void PC98BIOSGraphicsSetMode(unsigned char mode)
{
    register volatile unsigned char m __asm("%ch");
    m = mode;
    __asm volatile (
        "movb $0x42, %%ah\n\t"
        "int $0x18\n\t"
    : : "c" (m) : "ah");
}
//Supporting defines
#define CRT_MODE_GRAPHIC_PAGE0         0x00
#define CRT_MODE_GRAPHIC_PAGE1         0x10
#define CRT_MODE_GRAPHIC_COLOUR        0x00
#define CRT_MODE_GRAPHIC_MONOCHROME    0x20
#define CRT_MODE_GRAPHIC_640x200_LOWER 0x40
#define CRT_MODE_GRAPHIC_640x200_UPPER 0x80
#define CRT_MODE_GRAPHIC_640x400       0xC0
