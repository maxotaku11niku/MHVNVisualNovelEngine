/* MHVN98 - PC98 executable for the MHVN Visual Novel Engine
 * Copyright (c) 2023-2024 Maxim Hoxha
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Unicode handling
 */

#include "platform/filehandling.h"
#include "rootinfo.h"
#include "unicode.h"

#define RANGELIST_SIZE      128
#define GLYPHCACHE_ADDRBITS 9
#define GLYPHCACHE_ADDRMASK 0x01FF
#define GLYPHCACHE_SIZE     256

//Cache all the printable ASCII characters so that they are always fast to retrieve. Intialised with a basic font.
unsigned char asciiCharacterCache[94 * 16] =
{
    //0x0021, !
    0b00000000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x0022, "
    0b00000000,0b01100110,0b01100110,0b01100110,0b01100110,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x0023, #
    0b00000000,0b00110110,0b00110110,0b00110110,0b00110110,0b11111110,0b01101100,0b01101100,0b01101100,0b01101100,0b11111110,0b11011000,0b11011000,0b11011000,0b00000000,0b00000000,
    //0x0024, $
    0b00000000,0b00011000,0b01111110,0b11011011,0b11011000,0b11011000,0b11011000,0b01111110,0b00011011,0b00011011,0b00011011,0b11011011,0b01111110,0b00011000,0b00000000,0b00000000,
    //0x0025, %
    0b00000000,0b01100110,0b11010110,0b11010110,0b01101100,0b00001100,0b00011000,0b00011000,0b00110000,0b00110000,0b01101100,0b01111010,0b11011010,0b11001100,0b00000000,0b00000000,
    //0x0026, &
    0b00000000,0b01110000,0b11011000,0b11011000,0b11011000,0b11011000,0b11111000,0b01110110,0b11111100,0b11011100,0b11011000,0b11011100,0b11011100,0b01110110,0b00000000,0b00000000,
    //0x0027, '
    0b00000000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x0028, (
    0b00000000,0b00001100,0b00011000,0b00011000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00011000,0b00011000,0b00001100,0b00000000,0b00000000,
    //0x0029, )
    0b00000000,0b00110000,0b00011000,0b00011000,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00011000,0b00011000,0b00110000,0b00000000,0b00000000,
    //0x002A, *
    0b00000000,0b00011000,0b01011010,0b00111100,0b01111110,0b00111100,0b01011010,0b00011000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x002B, +
    0b00000000,0b00000000,0b00000000,0b00000000,0b00011000,0b00011000,0b00011000,0b01111110,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x002C, ,
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00011000,0b00011000,0b00011000,0b00110000,
    //0x002D, -
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111110,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x002E, .
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x002F, /
    0b00000000,0b00000110,0b00000110,0b00000110,0b00001100,0b00001100,0b00011000,0b00011000,0b00110000,0b00110000,0b01100000,0b01100000,0b11000000,0b11000000,0b00000000,0b00000000,


    //0x0030, 0
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000110,0b11001110,0b11001110,0b11010110,0b11010110,0b11100110,0b11100110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0031, 1
    0b00000000,0b00011000,0b00111000,0b01111000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x0032, 2
    0b00000000,0b01111100,0b11000110,0b11000110,0b00000110,0b00000110,0b00000110,0b00001100,0b00001100,0b00011000,0b00011000,0b00110000,0b01100000,0b11111110,0b00000000,0b00000000,
    //0x0033, 3
    0b00000000,0b01111100,0b11000110,0b11000110,0b00000110,0b00000110,0b00000110,0b00111100,0b00000110,0b00000110,0b00000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0034, 4
    0b00000000,0b00011000,0b00111000,0b01111000,0b11011000,0b11011000,0b11011000,0b11011000,0b11111110,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x0035, 5
    0b00000000,0b11111110,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11111100,0b00000110,0b00000110,0b00000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0036, 6
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000000,0b11000000,0b11000000,0b11111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0037, 7
    0b00000000,0b11111110,0b00000110,0b00000110,0b00000110,0b00001100,0b00001100,0b00001100,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x0038, 8
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0039, 9
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111110,0b00000110,0b00000110,0b00000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x003A, :
    0b00000000,0b00000000,0b00000000,0b00011000,0b00011000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00011000,0b00011000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x003B, ;
    0b00000000,0b00000000,0b00000000,0b00011000,0b00011000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00011000,0b00011000,0b00011000,0b00110000,0b00000000,0b00000000,
    //0x003C, <
    0b00000000,0b00000000,0b00000110,0b00001100,0b00011000,0b00110000,0b01100000,0b11000000,0b01100000,0b00110000,0b00011000,0b00001100,0b00000110,0b00000000,0b00000000,0b00000000,
    //0x003D, =
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111110,0b00000000,0b00000000,0b00000000,0b01111110,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x003E, >
    0b00000000,0b00000000,0b11000000,0b01100000,0b00110000,0b00011000,0b00001100,0b00000110,0b00001100,0b00011000,0b00110000,0b01100000,0b11000000,0b00000000,0b00000000,0b00000000,
    //0x003F, ?
    0b00000000,0b01111100,0b11000110,0b11000110,0b00000110,0b00001100,0b00001100,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,0b00011000,0b00011000,0b00000000,0b00000000,

    //0x0040, @
    0b00000000,0b01111110,0b10000001,0b10000001,0b10000001,0b10000001,0b10111001,0b11101101,0b11101101,0b11101101,0b11101101,0b10110110,0b10000000,0b01111111,0b00000000,0b00000000,
    //0x0041, A
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b00000000,0b00000000,
    //0x0042, B
    0b00000000,0b11111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111100,0b00000000,0b00000000,
    //0x0043, C
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0044, D
    0b00000000,0b11111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111100,0b00000000,0b00000000,
    //0x0045, E
    0b00000000,0b01111110,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11111000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b01111110,0b00000000,0b00000000,
    //0x0046, F
    0b00000000,0b01111110,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11111000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b00000000,0b00000000,
    //0x0047, G
    0b00000000,0b01111110,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11001110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0048, H
    0b00000000,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b00000000,0b00000000,
    //0x0049, I
    0b00000000,0b00111100,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00111100,0b00000000,0b00000000,
    //0x004A, J
    0b00000000,0b11111000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b11110000,0b00000000,0b00000000,
    //0x004B, K
    0b00000000,0b11000110,0b11001100,0b11011000,0b11011000,0b11110000,0b11110000,0b11100000,0b11110000,0b11110000,0b11011000,0b11011000,0b11001100,0b11000110,0b00000000,0b00000000,
    //0x004C, L
    0b00000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b01111110,0b00000000,0b00000000,
    //0x004D, M
    0b00000000,0b11000110,0b11101110,0b11101110,0b11111110,0b11111110,0b11111110,0b11111110,0b11010110,0b11010110,0b11010110,0b11000110,0b11000110,0b11000110,0b00000000,0b00000000,
    //0x004E, N
    0b00000000,0b11000110,0b11100110,0b11100110,0b11110110,0b11110110,0b11110110,0b11111110,0b11011110,0b11011110,0b11011110,0b11001110,0b11001110,0b11000110,0b00000000,0b00000000,
    //0x004F, O
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,

    //0x0050, P
    0b00000000,0b11111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111100,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b00000000,0b00000000,
    //0x0051, Q
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11110110,0b11011110,0b01111100,0b00000110,0b00000000,
    //0x0052, R
    0b00000000,0b11111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111100,0b11011000,0b11011000,0b11001100,0b11001100,0b11000110,0b11000110,0b00000000,0b00000000,
    //0x0053, S
    0b00000000,0b01111100,0b11000110,0b11000110,0b11000000,0b11000000,0b11000000,0b01111100,0b00000110,0b00000110,0b00000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0054, T
    0b00000000,0b01111110,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x0055, U
    0b00000000,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0056, V
    0b00000000,0b11000110,0b11000110,0b11000110,0b11000110,0b01101100,0b01101100,0b01101100,0b01101100,0b00111000,0b00111000,0b00111000,0b00111000,0b00010000,0b00000000,0b00000000,
    //0x0057, W
    0b00000000,0b11000110,0b11000110,0b11000110,0b11010110,0b11010110,0b11010110,0b11111110,0b11111110,0b11111110,0b11111110,0b11111110,0b11101110,0b01101100,0b00000000,0b00000000,
    //0x0058, X
    0b00000000,0b11000110,0b11000110,0b01101100,0b01101100,0b01101100,0b00111000,0b00111000,0b00111000,0b01101100,0b01101100,0b01101100,0b11000110,0b11000110,0b00000000,0b00000000,
    //0x0059, Y
    0b00000000,0b01100110,0b01100110,0b01100110,0b01100110,0b01100110,0b01100110,0b00111100,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x005A, Z
    0b00000000,0b11111110,0b00000110,0b00000110,0b00001100,0b00001100,0b00011000,0b00011000,0b00110000,0b00110000,0b01100000,0b01100000,0b11000000,0b11111110,0b00000000,0b00000000,
    //0x005B, [
    0b00000000,0b00111100,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00110000,0b00111100,0b00000000,0b00000000,
    /*0x005C, \*/
    0b00000000,0b11000000,0b11000000,0b11000000,0b01100000,0b01100000,0b00110000,0b00110000,0b00011000,0b00011000,0b00001100,0b00001100,0b00000110,0b00000110,0b00000000,0b00000000,
    //0x005D, ]
    0b00000000,0b00111100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00001100,0b00111100,0b00000000,0b00000000,
    //0x005E, ^
    0b00000000,0b00010000,0b00111000,0b01101100,0b11000110,0b10000010,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x005F, _
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11111111,0b00000000,0b00000000,

    //0x0060, `
    0b00000000,0b00110000,0b00111000,0b00011000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    //0x0061, a
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111100,0b11000110,0b00000110,0b00000110,0b01111110,0b11000110,0b11000110,0b11001110,0b01110110,0b00000000,0b00000000,
    //0x0062, b
    0b00000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11011100,0b11100110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111100,0b00000000,0b00000000,
    //0x0063, c
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111100,0b11000110,0b11000110,0b11000000,0b11000000,0b11000000,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0064, d
    0b00000000,0b00000110,0b00000110,0b00000110,0b00000110,0b01110110,0b11001110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111110,0b00000000,0b00000000,
    //0x0065, e
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111100,0b11000110,0b11000110,0b11000110,0b11111110,0b11000000,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0066, f
    0b00000000,0b00111110,0b01100000,0b01100000,0b01100000,0b01100000,0b01100000,0b11111000,0b01100000,0b01100000,0b01100000,0b01100000,0b01100000,0b01100000,0b00000000,0b00000000,
    //0x0067, g
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01110110,0b11001110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111110,0b00000110,0b11111100,
    //0x0068, h
    0b00000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11011100,0b11100110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b00000000,0b00000000,
    //0x0069, i
    0b00000000,0b00000000,0b00011000,0b00011000,0b00000000,0b00000000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x006A, j
    0b00000000,0b00000000,0b00011000,0b00011000,0b00000000,0b00000000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b01110000,
    //0x006B, k
    0b00000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000110,0b11001100,0b11011000,0b11110000,0b11100000,0b11110000,0b11011000,0b11001100,0b11000110,0b00000000,0b00000000,
    //0x006C, l
    0b00000000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00001100,0b00000000,0b00000000,
    //0x006D, m
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11011100,0b11111110,0b11010110,0b11010110,0b11010110,0b11010110,0b11010110,0b11010110,0b11010110,0b00000000,0b00000000,
    //0x006E, n
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11011100,0b11100110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b00000000,0b00000000,
    //0x006F, o
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111100,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,

    //0x0070, p
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11011100,0b11100110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11111100,0b11000000,0b11000000,
    //0x0071, q
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01110110,0b11001110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b01111110,0b00000110,0b00000110,
    //0x0072, r
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11011110,0b11100000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b11000000,0b00000000,0b00000000,
    //0x0073, s
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01111100,0b11000110,0b11000110,0b11000000,0b01111100,0b00000110,0b11000110,0b11000110,0b01111100,0b00000000,0b00000000,
    //0x0074, t
    0b00000000,0b00000000,0b00000000,0b00011000,0b00011000,0b01111110,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00001110,0b00000000,0b00000000,
    //0x0075, u
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11001110,0b01110110,0b00000000,0b00000000,
    //0x0076, v
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11000110,0b11000110,0b11000110,0b01101100,0b01101100,0b01101100,0b00111000,0b00111000,0b00010000,0b00000000,0b00000000,
    //0x0077, w
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11010110,0b11010110,0b11111110,0b01101100,0b00000000,0b00000000,
    //0x0078, x
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11000110,0b11000110,0b01101100,0b01101100,0b00111000,0b01101100,0b01101100,0b11000110,0b11000110,0b00000000,0b00000000,
    //0x0079, y
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11000110,0b11001110,0b01110110,0b00000110,0b11111100,
    //0x007A, z
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11111110,0b00000110,0b00000110,0b00001100,0b00011000,0b00110000,0b01100000,0b11000000,0b11111110,0b00000000,0b00000000,
    //0x007B, {
    0b00000000,0b00001100,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00110000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00001100,0b00000000,0b00000000,
    //0x007C, |
    0b00000000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00000000,0b00000000,
    //0x007D, }
    0b00000000,0b00110000,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00001100,0b00011000,0b00011000,0b00011000,0b00011000,0b00011000,0b00110000,0b00000000,0b00000000,
    //0x007E, ~
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01110000,0b11011011,0b00001110,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000
};

typedef struct
{
    unsigned short first;
    unsigned short last;
} RangeEntryFile;

typedef struct
{
    unsigned short firstCode;
    unsigned short firstEntry;
    unsigned short length;
} RangeEntry;

static RangeEntry rangeList[RANGELIST_SIZE];
static unsigned int realRangeSize;
static unsigned long fontInfoListFilePtr;
static unsigned long fontGlyphDataFilePtr;
static unsigned char glyphCache[32 * GLYPHCACHE_SIZE];
static unsigned char glyphInfoCache[GLYPHCACHE_SIZE];
static unsigned int nextGlyphIndex;

typedef struct
{
    unsigned short codepoint;
    unsigned short cacheIndex;
} IndexBufferEntry;

static IndexBufferEntry glyphIndexBuffer[GLYPHCACHE_SIZE];
static unsigned int nextBufferIndex;

typedef struct
{
    unsigned short codepoint;
    unsigned short indexBufferPos;
} PresenceMapEntry;

static PresenceMapEntry glyphPresenceMap[GLYPHCACHE_ADDRMASK + 1];

//This hash function may work well with Unicode codepoints (especially if only one small non-ASCII range is used)
static inline unsigned int HashInt16ToPresenceMap(unsigned short x)
{
    return (x & GLYPHCACHE_ADDRMASK);
}

void InitFontFile()
{
    realRangeSize = 0;
    nextGlyphIndex = 0;
    nextBufferIndex = 0;
    unsigned int realReadLen;
    fileptr handle;
    int result = OpenFile(rootInfo.fontDataPath, DOSFILE_OPEN_READ, &handle);
    if (result) return;
    __far unsigned char* fb = rangeList;
    ReadFile(handle, sizeof(rangeList), fb, &realReadLen);
    CloseFile(handle);

    unsigned int totalNumChars = 0;
    RangeEntryFile* fRangeList = (RangeEntryFile*)rangeList;
    for (int i = 0; i < RANGELIST_SIZE; i++) //Find number of characters and true number of range entries
    {
        RangeEntryFile range = fRangeList[i];
        if (range.last == 0x0000 && range.first == 0x0000) break;
        totalNumChars += range.last - range.first + 1;
        realRangeSize++;
    }
    fontInfoListFilePtr = realRangeSize * 4;
    fontGlyphDataFilePtr = fontInfoListFilePtr + totalNumChars * 4;
    unsigned short listPtr = 0;
    for (int i = realRangeSize - 1; i >= 0; i--) //In place modify range entries to be easier to work with
    {
        RangeEntryFile rangeInFile = fRangeList[i];
        RangeEntry range;
        range.firstCode = rangeInFile.first;
        range.length = rangeInFile.last - rangeInFile.first + 1;
        range.firstEntry = listPtr;
        rangeList[i] = range;
        listPtr += range.length;
    }

    //Load all ASCII glyphs. Not necessarily the fastest way to do so but it's small due to code reuse.
    unsigned char glyphBuffer[32];
    for (int i = 0x21; i < 0x7F; i++)
    {
        unsigned char glyphInfo = LoadGlyphFromFile(i, glyphBuffer);
        if (glyphInfo != GLYPHCACHEINFO_INVALID)
        {
            unsigned char* asciiCharPtr = asciiCharacterCache + 16 * (i - 0x21);
            for (int i = 0; i < 16; i++)
            {
                asciiCharPtr[i] = glyphBuffer[2 * i];
            }
        }
    }
}


unsigned int UTF8CharacterDecode(const unsigned char** pstr)
{
    const unsigned char* str = *pstr;
    unsigned char inchar = *str++;
    unsigned int outchar = 0;
    while (1)
    {
        if (inchar < 0x80) //ASCII range -> 1 byte
        {
            outchar = inchar;
            break;
        }
        else if (inchar < 0xC0) //Continuation byte found in inappropriate place -> ignore it
        {
            inchar = *str++;
        }
        else if (inchar < 0xE0) //2 byte character
        {
            outchar = ((unsigned int)(inchar & 0x1F)) << 6;
            inchar = *str++;
            if (inchar >= 0x80 && inchar < 0xC0) //Check for valid continuation byte
            {
                outchar |= ((unsigned int)(inchar & 0x3F));
                break;
            }
        }
        else if (inchar < 0xF0) //3 byte character
        {
            outchar = ((unsigned int)(inchar & 0x0F)) << 12;
            inchar = *str++;
            if (inchar >= 0x80 && inchar < 0xC0) //Check for valid continuation byte
            {
                outchar |= ((unsigned int)(inchar & 0x3F)) << 6;
                inchar = *str++;
                if (inchar >= 0x80 && inchar < 0xC0) //Check for valid continuation byte
                {
                    outchar |= ((unsigned int)(inchar & 0x3F));
                    break;
                }
            }
        }
        else //Do not handle 4 byte characters - they are outside the BMP
        {
            inchar = *str++;
        }
    }
    *pstr = str;
    return outchar;
}

unsigned char LoadGlyphFromFile(unsigned int code, unsigned char* buffer)
{
    //Binary search for appropriate range
    unsigned int checkRangeEntry = realRangeSize >> 1;
    unsigned int checkLower = 0;
    unsigned int checkUpper = realRangeSize - 1;
    while (1)
    {
        if (checkUpper < checkLower) return GLYPHCACHEINFO_INVALID; //Not in file
        RangeEntry range = rangeList[checkRangeEntry];
        if (code < range.firstCode)
        {
            checkUpper = checkRangeEntry - 1;
        }
        else
        {
            unsigned short lastCode = range.firstCode + range.length - 1;
            if (code > lastCode)
            {
                checkLower = checkRangeEntry + 1;
            }
            else
            {
                break;
            }
        }
        checkRangeEntry = ((checkUpper - checkLower) >> 2) + checkLower;
    }

    //Read in glyph data
    RangeEntry range = rangeList[checkRangeEntry];
    unsigned short codeInRange = code - range.firstCode;
    unsigned short infoInd = range.firstEntry + codeInRange;
    unsigned int realReadLen;
    fileptr handle;
    int result = OpenFile(rootInfo.fontDataPath, DOSFILE_OPEN_READ, &handle);
    if (result) return GLYPHCACHEINFO_INVALID;
    unsigned char glyphBuffer[32];
    unsigned long newpos;
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, fontInfoListFilePtr + infoInd * 4, &newpos);
    unsigned long charEntry;
    ReadFile(handle, 4, (__far unsigned char*)(&charEntry), &realReadLen);
    unsigned long charAddress =   charEntry & 0x001FFFFF;
    unsigned int charWidth    = ((charEntry & 0x00800000) >> 23) + 1;
    unsigned int charHeight   = ((charEntry & 0xF0000000) >> 28) + 1;
    unsigned int charYoffset  =  (charEntry & 0x0F000000) >> 24;
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, fontGlyphDataFilePtr + charAddress, &newpos);
    ReadFile(handle, 32, (__far unsigned char*)(glyphBuffer), &realReadLen);
    CloseFile(handle);

    //Transform to appropriate form
    Memset16Near(0x0000, buffer, 16);
    if (charWidth == 1)
    {
        for (int i = 0; i < charHeight; i++)
        {
            buffer[2 * (i + charYoffset)] = glyphBuffer[i];
        }
    }
    else if (charWidth == 2)
    {
        Memcpy16Near(glyphBuffer, buffer + (charYoffset * 2), charHeight);
    }
    return ((charWidth * 8) - 1) & GLYPHCACHEINFO_WIDTHMASK;
}

int LoadGlyphCacheWithCharacter(unsigned int code)
{
    if (code < 0x80) return -1; //Permanent cache or unprintable ASCII -> already loaded

    //Check for cache hit/miss
    unsigned int mapInd = HashInt16ToPresenceMap(code);
    PresenceMapEntry* mapPtr = glyphPresenceMap + mapInd;
    while (mapPtr->codepoint != 0xFFFF)
    {
        if (mapPtr->codepoint == code) break; //Cache hit
        else //Hash collision
        {
            mapInd++;
            mapInd &= GLYPHCACHE_ADDRMASK;
            mapPtr = glyphPresenceMap + mapInd;
        }
    }
    IndexBufferEntry* nextIndPtr = glyphIndexBuffer + nextBufferIndex;
    if (mapPtr->codepoint == 0xFFFF) //Cache miss
    {
        unsigned char glyphInfo = LoadGlyphFromFile(code, glyphCache + (nextGlyphIndex * 32));
        if (glyphInfo == GLYPHCACHEINFO_INVALID) return -1; //Not found in file
        mapPtr->codepoint = code;
        glyphInfoCache[nextGlyphIndex] = glyphInfo;
        if (nextIndPtr->codepoint != 0xFFFF) //Cache eviction
        {
            unsigned int evictCode = nextIndPtr->codepoint;
            unsigned int evictInd = HashInt16ToPresenceMap(evictCode);
            PresenceMapEntry* evictPtr = glyphPresenceMap + evictInd;
            while (evictPtr->codepoint != evictCode) //Check for hash collisions
            {
                evictInd++;
                evictInd &= GLYPHCACHE_ADDRMASK;
                evictPtr = glyphPresenceMap + evictInd;
            }
            evictPtr->codepoint = 0xFFFF;

            //Clean up resolved collisions
            PresenceMapEntry* fixPtr = evictPtr;
            evictInd++;
            evictInd &= GLYPHCACHE_ADDRMASK;
            evictPtr = glyphPresenceMap + (evictInd & GLYPHCACHE_ADDRMASK);
            while (evictPtr->codepoint != 0xFFFF)
            {
                unsigned int fixInd = HashInt16ToPresenceMap(evictPtr->codepoint);
                if (fixInd != evictInd) //Resolved collision
                {
                    *fixPtr = *evictPtr;
                    evictPtr->codepoint = 0xFFFF;
                    fixPtr = evictPtr;
                }
                evictInd++;
                evictInd &= GLYPHCACHE_ADDRMASK;
                evictPtr = glyphPresenceMap + evictInd;
            }
        }
        nextIndPtr->codepoint = code;
        nextIndPtr->cacheIndex = nextGlyphIndex;
        nextGlyphIndex++;
        nextGlyphIndex %= GLYPHCACHE_SIZE;
    }
    else //Cache hit, no load needed
    {
        unsigned int bufInd = mapPtr->indexBufferPos;
        IndexBufferEntry* indPtr = glyphIndexBuffer + bufInd;
        *nextIndPtr = *indPtr;
        indPtr->codepoint = 0xFFFF;
    }
    mapPtr->indexBufferPos = nextBufferIndex;
    nextBufferIndex++;
    nextBufferIndex %= GLYPHCACHE_SIZE;
    return nextIndPtr->cacheIndex;
}

int UnicodeGetCharacterWidth(unsigned int code)
{
    if (code < 0x80) //Permanent cache or unprintable ASCII
    {
        if (code >= 0x21 && code < 0x7F) return 8;
        else return -1;
    }
    else
    {
        int gCacheInd = LoadGlyphCacheWithCharacter(code);
        if (gCacheInd < 0) return -1;
        unsigned char gInfo = glyphInfoCache[gCacheInd];
        if (gInfo == GLYPHCACHEINFO_INVALID) return -1;
        else
        {
            return (gInfo & GLYPHCACHEINFO_WIDTHMASK) + 1;
        }
    }
}

int UnicodeGetCharacterData(unsigned int code, unsigned long* buffer)
{
    if (code < 0x80) //Permanent cache or unprintable ASCII
    {
        if (code >= 0x21 && code < 0x7F)
        {
            unsigned char* curChar = asciiCharacterCache + (code - 0x21) * 16;
            for (unsigned char i = 0; i < 16; i++)
            {
                unsigned long row = curChar[i];
                buffer[i] = (row << 24);
            }
            return 8;
        }
        else return -1;
    }
    else
    {
        int gCacheInd = LoadGlyphCacheWithCharacter(code);
        if (gCacheInd < 0) return -1;
        unsigned char* curChar = glyphCache + gCacheInd * 32;
        unsigned char gInfo = glyphInfoCache[gCacheInd];
        if (gInfo == GLYPHCACHEINFO_INVALID) return -1;
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned short row = curChar[2*i + 1]; //Read in right half
            unsigned short rowhalf = curChar[2*i];
            row |= (rowhalf << 8); //Read in left half
            ((unsigned short*)(&buffer[i]))[0] = 0;
            ((unsigned short*)(&buffer[i]))[1] = row;
        }
        return (gInfo & GLYPHCACHEINFO_WIDTHMASK) + 1;
    }
}

void SwapCharDataFormats(unsigned long* buffer, int bits32)
{
    if (!bits32) //Also pack to 16 bits per character row, one way operation
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned short row = ((unsigned short*)buffer)[2*i + 1];
            unsigned short temprow = (row & 0x00FF) << 8;
            ((unsigned short*)buffer)[i] = temprow | ((row & 0xFF00) >> 8);
        }
    }
    else
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned long row = buffer[i];
            unsigned long temprow = (row & 0x000000FF) << 24;
            temprow |= (row & 0x0000FF00) << 8;
            temprow |= (row & 0x00FF0000) >> 8;
            buffer[i] = temprow | ((row & 0xFF000000) >> 24);
        }
    }
}
