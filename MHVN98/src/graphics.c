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
 * Graphics functions - very PC98-specific
 */

#include "platform/memalloc.h"
#include "platform/pc98_egc.h"
#include "platform/x86segments.h"
#include "platform/x86strops.h"
#include "platform/filehandling.h"
#include "rootinfo.h"
#include "gpimage.h"
#include "graphics.h"
#include "palette.h"

#define VRAM_HIDDEN_OFFSET 0x7D00

//Default text border for testing purposes only
unsigned char tempDefaultTextBorder[1152] =
{
    //Plane 0, top-left
    0b00011000, 0b00000000,
    0b01111111, 0b11111111,
    0b01111111, 0b11111111,
    0b11111111, 0b11000000,
    0b11111111, 0b10111111,
    0b01111110, 0b01111111,
    0b00111101, 0b11111111,
    0b01011011, 0b11111111,
    0b01111011, 0b00000000,
    0b01110111, 0b00000000,
    0b01101111, 0b01111111,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b01111111,
    0b01101111, 0b00000000,
    //Plane 0, top
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000001, 0b10000000,
    0b11111111, 0b10111111,
    0b11111111, 0b10111111,
    0b11111111, 0b10111111,
    0b11111111, 0b10111111,
    0b00000001, 0b10000000,
    0b00000111, 0b11100000,
    0b11111111, 0b11111111,
    0b00000001, 0b10000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    //Plane 0, top-right
    0b00000000, 0b00011000,
    0b11111111, 0b11111110,
    0b11111111, 0b11111110,
    0b00000011, 0b11111111,
    0b11111101, 0b11111111,
    0b11111110, 0b01111110,
    0b11111111, 0b10111100,
    0b11111111, 0b11011010,
    0b00000000, 0b11111010,
    0b00000000, 0b11111100,
    0b11111110, 0b11111110,
    0b01011110, 0b11111110,
    0b00000000, 0b11111110,
    0b00000000, 0b11111110,
    0b11111110, 0b11111110,
    0b01011110, 0b11111110,
    //Plane 0, left
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b01111111,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b01111111,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b01111111,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01100000, 0b00000000,
    0b01101111, 0b01111111,
    0b01101111, 0b00000000,
    //Plane 0, center
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    //Plane 0, right
    0b00000000, 0b11111110,
    0b00000000, 0b11111110,
    0b11111110, 0b11111110,
    0b01011110, 0b11111110,
    0b00000000, 0b11011110,
    0b00000000, 0b11011110,
    0b11111110, 0b11011110,
    0b01011110, 0b11011110,
    0b00000000, 0b11110110,
    0b00000000, 0b11011110,
    0b11111110, 0b11011110,
    0b01011110, 0b11011110,
    0b00000000, 0b11011110,
    0b00000000, 0b00000110,
    0b11111110, 0b11111110,
    0b01011110, 0b11111110,
    //Plane 0, bottom-left
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b01111111,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b00000000,
    0b01101111, 0b01111111,
    0b01101111, 0b00000000,
    0b01101111, 0b11111111,
    0b01101111, 0b11111111,
    0b01101111, 0b11111111,
    0b01111111, 0b11111111,
    0b01111111, 0b11111111,
    0b00111111, 0b11111111,
    0b01001111, 0b11111111,
    0b00000000, 0b00000000,
    //Plane 0, bottom
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    //Plane 0, bottom-right
    0b00000000, 0b11111110,
    0b00000000, 0b11111110,
    0b11111110, 0b11111110,
    0b01011110, 0b11111110,
    0b00000000, 0b11111110,
    0b00000000, 0b11111110,
    0b11111110, 0b11111110,
    0b00000000, 0b11111110,
    0b11111111, 0b11111110,
    0b11111111, 0b11111110,
    0b11111111, 0b11111110,
    0b11111111, 0b11111110,
    0b11111111, 0b11111110,
    0b11111111, 0b11111100,
    0b11111111, 0b11110010,
    0b00000000, 0b00000000,

    //Plane 1, top-left
    0b11111111, 0b11111111,
    0b10111100, 0b00111111,
    0b11111110, 0b01111111,
    0b11111111, 0b11000000,
    0b11111111, 0b10000000,
    0b01111110, 0b00000000,
    0b10111100, 0b00000000,
    0b10011000, 0b00000000,
    0b10011000, 0b00000000,
    0b10110000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b01111111,
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b01111111,
    //Plane 1, top
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000001, 0b10000000,
    0b00000001, 0b10000000,
    0b00000001, 0b10000000,
    0b00000001, 0b10000000,
    0b00000001, 0b10000000,
    0b00000001, 0b10000000,
    0b00000111, 0b11100000,
    0b00000011, 0b11000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    //Plane 1, top-right
    0b11111111, 0b11111111,
    0b11111100, 0b00111100,
    0b11111110, 0b01111110,
    0b00000011, 0b11111111,
    0b00000001, 0b11111111,
    0b00000000, 0b01111110,
    0b00000000, 0b00111100,
    0b00000000, 0b00011000,
    0b00000000, 0b00011000,
    0b01011111, 0b00001100,
    0b10101111, 0b00000110,
    0b11111111, 0b00000110,
    0b00000001, 0b00000110,
    0b01011111, 0b00000110,
    0b10101111, 0b00000110,
    0b11111111, 0b00000110,
    //Plane 1, left
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11101100, 0b01111111,
    0b11101100, 0b00000000,
    0b11101100, 0b00000000,
    0b11101100, 0b00000000,
    0b11101100, 0b01111111,
    0b11101100, 0b00000000,
    0b11101100, 0b00000000,
    0b11101100, 0b00000000,
    0b11101111, 0b01111111,
    0b11101111, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b01111111,
    //Plane 1, center
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    //Plane 1, right
    0b00000001, 0b00000110,
    0b01011111, 0b00000110,
    0b10101111, 0b00000110,
    0b11111111, 0b11110110,
    0b00000001, 0b11011110,
    0b01011111, 0b11011110,
    0b10101111, 0b11011110,
    0b11111111, 0b11011110,
    0b00000001, 0b11110110,
    0b01011111, 0b11011110,
    0b10101111, 0b11011110,
    0b11111111, 0b11011110,
    0b00000001, 0b11011110,
    0b01011111, 0b00000110,
    0b10101111, 0b00000110,
    0b11111111, 0b00000110,
    //Plane 1, bottom-left
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b01111111,
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b00000000,
    0b11100000, 0b11111111,
    0b11100000, 0b00000000,
    0b11100000, 0b00001111,
    0b11100000, 0b01110000,
    0b11111111, 0b10000000,
    0b11111110, 0b00000000,
    0b11111100, 0b00000000,
    0b10111000, 0b00000000,
    0b00000000, 0b00000000,
    //Plane 1, bottom
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111000, 0b00011111,
    0b00000110, 0b01100000,
    0b00000001, 0b10000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    //Plane 1, bottom-right
    0b00000001, 0b00000110,
    0b01011111, 0b00000110,
    0b10101111, 0b00000110,
    0b11111111, 0b00000110,
    0b00000001, 0b00000110,
    0b01011111, 0b00000110,
    0b10101111, 0b00000110,
    0b11111111, 0b00000110,
    0b00000000, 0b00000110,
    0b11110000, 0b00000110,
    0b00001110, 0b00000110,
    0b00000001, 0b11111110,
    0b00000000, 0b00111110,
    0b00000000, 0b00011100,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,

    //Plane 2, top-left
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00100010, 0b00111111,
    0b00000011, 0b01000000,
    0b00000111, 0b10000000,
    0b00001110, 0b00000000,
    0b00111100, 0b00000000,
    0b00011000, 0b00000000,
    0b00001000, 0b00000000,
    0b00010000, 0b01111111,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    0b00100000, 0b00000000,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    //Plane 2, top
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111110, 0b01111111,
    0b00000000, 0b10000000,
    0b00000000, 0b10000000,
    0b00000000, 0b10000000,
    0b00000000, 0b10000000,
    0b00000000, 0b10000000,
    0b00000000, 0b10000000,
    0b11111000, 0b00111111,
    0b11111100, 0b01111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    //Plane 2, top-right
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111100, 0b00100010,
    0b00000010, 0b00000011,
    0b00000001, 0b10000111,
    0b00000000, 0b01001110,
    0b00000000, 0b00111100,
    0b00000000, 0b00011000,
    0b00000000, 0b00001000,
    0b10100000, 0b00000100,
    0b01010000, 0b00000010,
    0b11111110, 0b00000010,
    0b00000000, 0b00000010,
    0b10100000, 0b00000010,
    0b01010000, 0b00000010,
    0b11111110, 0b00000010,
    //Plane 2, left
    0b00100000, 0b00000000,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    0b00100100, 0b01111111,
    0b00100100, 0b00000000,
    0b00100100, 0b01111111,
    0b00100100, 0b01111111,
    0b00100100, 0b01111111,
    0b00100100, 0b00000000,
    0b00100100, 0b01111111,
    0b00100100, 0b01111111,
    0b00100000, 0b01111111,
    0b00101111, 0b00000000,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    //Plane 2, center
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    //Plane 2, right
    0b00000000, 0b00000010,
    0b10100000, 0b00000010,
    0b01010000, 0b00000010,
    0b11111110, 0b00010010,
    0b00000000, 0b01001010,
    0b10100000, 0b01001010,
    0b01010000, 0b01001010,
    0b11111110, 0b01001010,
    0b00000000, 0b00010010,
    0b10100000, 0b01001010,
    0b01010000, 0b01001010,
    0b11111110, 0b01001010,
    0b00000000, 0b01001010,
    0b10100000, 0b00000010,
    0b01010000, 0b00000010,
    0b11111110, 0b00000010,
    //Plane 2, bottom-left
    0b00100000, 0b00000000,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    0b00100000, 0b00000000,
    0b00100000, 0b01111111,
    0b00100000, 0b01111111,
    0b00100000, 0b00000000,
    0b00100000, 0b00000000,
    0b00100000, 0b00001111,
    0b00100000, 0b01110000,
    0b00000001, 0b10000000,
    0b01000010, 0b00000000,
    0b00111100, 0b00000000,
    0b00001000, 0b00000000,
    0b00000000, 0b00000000,
    //Plane 2, bottom
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b11111000, 0b00011111,
    0b00000110, 0b01100000,
    0b00000001, 0b10000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    //Plane 2, bottom-right
    0b00000000, 0b00000010,
    0b10100000, 0b00000010,
    0b01010000, 0b00000010,
    0b11111110, 0b00000010,
    0b00000000, 0b00000010,
    0b10100000, 0b00000010,
    0b01010000, 0b00000010,
    0b00000000, 0b00000010,
    0b00000000, 0b00000010,
    0b11110000, 0b00000010,
    0b00001110, 0b00000010,
    0b00000001, 0b11000010,
    0b00000000, 0b00100010,
    0b00000000, 0b00011100,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,

    //Plane 3, top-left
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111101, 0b11000000,
    0b11111100, 0b10111111,
    0b11111000, 0b01111111,
    0b11110001, 0b11111111,
    0b11000011, 0b11111111,
    0b11100111, 0b11111111,
    0b11110111, 0b11111111,
    0b11101111, 0b10000101,
    0b11011111, 0b10001010,
    0b11011111, 0b10000101,
    0b11011111, 0b10000000,
    0b11011111, 0b10000101,
    0b11011111, 0b10001010,
    0b11011111, 0b10000101,
    //Plane 3, top
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000001, 0b10000000,
    0b11111111, 0b01111111,
    0b11111111, 0b01111111,
    0b11111111, 0b01111111,
    0b11111111, 0b01111111,
    0b11111111, 0b01111111,
    0b11111111, 0b01111111,
    0b11111111, 0b11011111,
    0b11111111, 0b10111111,
    0b11111110, 0b01111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    //Plane 3, top-right
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000011, 0b11111101,
    0b11111101, 0b11111100,
    0b11111110, 0b01111000,
    0b11111111, 0b10110001,
    0b11111111, 0b11000011,
    0b11111111, 0b11100111,
    0b11111111, 0b11110111,
    0b10100001, 0b11111011,
    0b01010001, 0b11111101,
    0b11111111, 0b11111101,
    0b00000001, 0b11111101,
    0b10100001, 0b11111101,
    0b01010001, 0b11111101,
    0b11111111, 0b11111101,
    //Plane 3, left
    0b11011111, 0b10000000,
    0b11011111, 0b10000101,
    0b11011111, 0b10001010,
    0b11011011, 0b10000101,
    0b11011011, 0b10000000,
    0b11011011, 0b10000101,
    0b11011011, 0b10001010,
    0b11011011, 0b10000101,
    0b11011011, 0b10000000,
    0b11011011, 0b10000101,
    0b11011011, 0b10001010,
    0b11011111, 0b10000101,
    0b11010000, 0b10000000,
    0b11011111, 0b10000101,
    0b11011111, 0b10001010,
    0b11011111, 0b10000101,
    //Plane 3, center
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    //Plane 3, right
    0b00000001, 0b11111101,
    0b10100001, 0b11111101,
    0b01010001, 0b11111101,
    0b11111111, 0b11101101,
    0b00000001, 0b10110101,
    0b10100001, 0b10110101,
    0b01010001, 0b10110101,
    0b11111111, 0b10110101,
    0b00000001, 0b11101101,
    0b10100001, 0b10110101,
    0b01010001, 0b10110101,
    0b11111111, 0b10110101,
    0b00000001, 0b10110101,
    0b10100001, 0b11111101,
    0b01010001, 0b11111101,
    0b11111111, 0b11111101,
    //Plane 3, bottom-left
    0b11011111, 0b10000000,
    0b11011111, 0b10000101,
    0b11011111, 0b10001010,
    0b11011111, 0b10000101,
    0b11011111, 0b10000000,
    0b11011111, 0b10000101,
    0b11011111, 0b10001010,
    0b11011111, 0b11111111,
    0b11011111, 0b11111111,
    0b11011111, 0b11111111,
    0b11011111, 0b11110000,
    0b11111111, 0b10000000,
    0b11111110, 0b00000000,
    0b11111100, 0b00000000,
    0b11111000, 0b00000000,
    0b11100000, 0b00000000,
    //Plane 3, bottom
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000000, 0b00000000,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b00000111, 0b11100000,
    0b00000001, 0b10000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    //Plane 3, bottom-right
    0b00000001, 0b11111101,
    0b10100001, 0b11111101,
    0b01010001, 0b11111101,
    0b11111111, 0b11111101,
    0b00000001, 0b11111101,
    0b10100001, 0b11111101,
    0b01010001, 0b11111101,
    0b11111111, 0b11111101,
    0b11111111, 0b11111101,
    0b11111111, 0b11111101,
    0b00001111, 0b11111101,
    0b00000001, 0b10111101,
    0b00000000, 0b00011101,
    0b00000000, 0b00000011,
    0b00000000, 0b00001111,
    0b00000000, 0b00000111,
};

ImageInfo bgInfo;
ImageInfo textboxInfo;
ImageInfo charnameboxInfo;
ImageInfo choiceboxInfo;
ImageInfo sprite1Info;
ImageInfo sprite2Info;
ImageInfo sprite3Info;
ImageInfo bgVariants[4];
ImageInfo sprite1Variants[4];
ImageInfo sprite2Variants[4];
ImageInfo sprite3Variants[4];
ImageInfo* allInfos[23] = {&bgInfo, &bgVariants[0], &bgVariants[1], &bgVariants[2], &bgVariants[3],
                           &sprite1Info, &sprite1Variants[0], &sprite1Variants[1], &sprite1Variants[2], &sprite1Variants[3],
                           &sprite2Info, &sprite2Variants[0], &sprite2Variants[1], &sprite2Variants[2], &sprite2Variants[3],
                           &sprite3Info, &sprite3Variants[0], &sprite3Variants[1], &sprite3Variants[2], &sprite3Variants[3],
                           &textboxInfo, &charnameboxInfo, &choiceboxInfo};

__far unsigned char* bgImageDataPlane0;
__far unsigned char* bgImageDataPlane1;
__far unsigned char* bgImageDataPlane2;
__far unsigned char* bgImageDataPlane3;

unsigned int numBg;
unsigned int numSpr;

void UnloadImage(ImageInfo* img)
{
    if (img == 0 || !(img->flags & IMAGE_LOADED)) return; //Don't bother if image is not loaded or if pointer to image data is null (i.e. known not to be valid)
    img->flags = 0;
}

ImageInfo* LoadBGImage(unsigned int num)
{
    if (bgInfo.flags & IMAGE_LOADED)
    {
        if (bgInfo.id == num) return &bgInfo; //Do not bother to reload the BG image if the same one is requested
        UnloadImage(&bgInfo);
    }

    unsigned int realReadLen;
    unsigned long curfilepos;
    fileptr handle;
    int result = OpenFile(rootInfo.BGDataPath,  DOSFILE_OPEN_READ, &handle);
    if (result)
    {
        WriteString("Error! Could not find BG data file!", 180, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result;
    }
    unsigned long bgdatpos;
    __far unsigned char* bgp = (__far unsigned char*)&bgdatpos;
    ReadFile(handle, 4, bgp, &realReadLen);
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, bgdatpos, &curfilepos);
    unsigned long thisBgDatPos;
    __far unsigned char* tbgp = (__far unsigned char*)&thisBgDatPos;
    ReadFile(handle, 4, tbgp, &realReadLen);
    thisBgDatPos += bgdatpos + (numBg << 2);
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, thisBgDatPos, &curfilepos);
    unsigned long bgimgpos;
    __far unsigned char* bgip = (__far unsigned char*)&bgimgpos;
    ReadFile(handle, 4, bgip, &realReadLen);
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, thisBgDatPos + 6, &curfilepos);
    unsigned short palInd;
    __far unsigned char* pip = (__far unsigned char*)&palInd;
    ReadFile(handle, 2, pip, &realReadLen);
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, 8 + 24 * ((unsigned long)palInd), &curfilepos);
    unsigned char midPal[24];
    __far unsigned char* mpp = (__far unsigned char*)midPal;
    ReadFile(handle, 24, mpp, &realReadLen);
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, bgimgpos, &curfilepos);
    GPIInfo gpiinf;
    gpiinf.handle = handle;
    gpiinf.planes[0] = bgImageDataPlane0;
    gpiinf.planes[1] = bgImageDataPlane1;
    gpiinf.planes[2] = bgImageDataPlane2;
    gpiinf.planes[3] = bgImageDataPlane3;
    result = OpenGPIFile(&gpiinf);
    if (!result) DecompressGPIFile(&gpiinf);
    CloseFile(handle);
    if (!result && gpiinf.hasMask) MemFree(gpiinf.maskPlane); //Don't need mask plane for backgrounds
    ColourRGB outpal[16];
    int mpPtr = 0;
    for (int i = 0; i < 16; i++)
    {
        unsigned char cb = midPal[mpPtr];
        mpPtr++;
        outpal[i].r = (cb & 0xF) * 0x11;
        outpal[i].g = ((cb >> 4) & 0x0F) * 0x11;
        cb = midPal[mpPtr];
        mpPtr++;
        outpal[i].b = (cb & 0xF) * 0x11;
        i++;
        outpal[i].r = ((cb >> 4) & 0x0F) * 0x11;
        cb = midPal[mpPtr];
        mpPtr++;
        outpal[i].g = (cb & 0xF) * 0x11;
        outpal[i].b = ((cb >> 4) & 0x0F) * 0x11;
    }
    SetMainPalette(outpal);
    CopyMainPaletteToOut();
    SetDisplayPaletteToOut();

    bgInfo.boundRect.pos.x = 0;
    bgInfo.boundRect.pos.y = 0;
    bgInfo.boundRect.size.x = 640;
    bgInfo.boundRect.size.y = 400; //BG images MUST take up the entire screen
    bgInfo.mask = (__far unsigned char*)0; //No mask
    bgInfo.plane0 = bgImageDataPlane0;
    bgInfo.plane1 = bgImageDataPlane1;
    bgInfo.plane2 = bgImageDataPlane2;
    bgInfo.plane3 = bgImageDataPlane3;
    bgInfo.children = bgVariants;
    bgInfo.id = num;
    bgInfo.layer = 0;
    bgInfo.flags = IMAGE_TYPE_NORMAL | IMAGE_MEM_NORMAL | IMAGE_ALIGN_FIXED | IMAGE_LOADED;

    //probably temporary
    if (textboxInfo.flags & IMAGE_DRAWN) textboxInfo.flags |= IMAGE_DRAWREQ;
    if (charnameboxInfo.flags & IMAGE_DRAWN) charnameboxInfo.flags |= IMAGE_DRAWREQ;
    if (choiceboxInfo.flags & IMAGE_DRAWN) choiceboxInfo.flags |= IMAGE_DRAWREQ;

    return &bgInfo;
}

ImageInfo* LoadSprite(unsigned int num, unsigned int slot)
{
    ImageInfo* selInfo;
    ImageInfo* selChild;
    switch (slot)
    {
        case 0:
            selInfo = &sprite1Info;
            selChild = sprite1Variants;
            break;
        case 1:
            selInfo = &sprite2Info;
            selChild = sprite2Variants;
            break;
        case 2:
            selInfo = &sprite3Info;
            selChild = sprite3Variants;
            break;
        default:
            return (ImageInfo*)0;
    }

    if (selInfo->flags & IMAGE_LOADED)
    {
        UnloadImage(selInfo);
    }
    unsigned char intoLayer = 8 + 8 * slot;

    selInfo->boundRect.pos.x = 0;
    selInfo->boundRect.pos.y = 0;
    selInfo->boundRect.size.x = 0;
    selInfo->boundRect.size.y = 0;
    selInfo->mask = (__far unsigned char*)0;
    selInfo->plane0 = (__far unsigned char*)0;
    selInfo->plane1 = (__far unsigned char*)0;
    selInfo->plane2 = (__far unsigned char*)0;
    selInfo->plane3 = (__far unsigned char*)0;
    selInfo->children = selChild;
    selInfo->id = num;
    selInfo->layer = intoLayer;
    selInfo->flags = IMAGE_TYPE_NORMAL | IMAGE_MEM_NORMAL | IMAGE_ALIGN_FREE | IMAGE_LOADED;
    return selInfo;
}

void LoadStd9SliceBoxIntoVRAM()
{
    EGCDisable();

    __asm (
        "movw $0xA800, %%ax\n\t" //Plane 0 segment
        "movw %%ax, %%es\n\t"
        "movw %0, %%si\n\t"
        "movw %1, %%di\n\t"
        "movw $144, %%cx\n\t"
        "rep movsw\n\t"
        "movw $0xB000, %%ax\n\t" //Plane 1 segment
        "movw %%ax, %%es\n\t"
        "movw %1, %%di\n\t"
        "movw $144, %%cx\n\t"
        "rep movsw\n\t"
        "movw $0xB800, %%ax\n\t" //Plane 2 segment
        "movw %%ax, %%es\n\t"
        "movw %1, %%di\n\t"
        "movw $144, %%cx\n\t"
        "rep movsw\n\t"
        "movw $0xE000, %%ax\n\t" //Plane 3 segment
        "movw %%ax, %%es\n\t"
        "movw %1, %%di\n\t"
        "movw $144, %%cx\n\t"
        "rep movsw\n\t"
    : : "rmi" (tempDefaultTextBorder), "i" (VRAM_HIDDEN_OFFSET) : "%ax", "%si", "%di", "%cx", "%es");

    EGCEnable();
    EGCSetPlaneAccess(0xF);
    EGCSetMask(0xFFFF);
    EGCSetBGColour(0xC);
    EGCSetToMonochromeDrawMode();
}

static void RegisterStd9SliceBox(unsigned char layer, ImageInfo* img, const Rect2Int* rect)
{
    img->boundRect = *rect;
    img->mask = (__far unsigned char*)0; //No mask
    img->plane0 = (__far unsigned char*)(0xA8000000 + VRAM_HIDDEN_OFFSET);
    img->plane1 = (__far unsigned char*)(0xB0000000 + VRAM_HIDDEN_OFFSET);
    img->plane2 = (__far unsigned char*)(0xB8000000 + VRAM_HIDDEN_OFFSET);
    img->plane3 = (__far unsigned char*)(0xE0000000 + VRAM_HIDDEN_OFFSET);
    img->children = (ImageInfo*)0; //No children
    img->layer = layer;
    img->flags = IMAGE_TYPE_9SLICE | IMAGE_MEM_VRAM | IMAGE_ALIGN_FIXED | IMAGE_LOADED;
}

ImageInfo* RegisterTextBox(const Rect2Int* rect)
{
    RegisterStd9SliceBox(128, &textboxInfo, rect);
    return &textboxInfo;
}

ImageInfo* RegisterCharNameBox(const Rect2Int* rect)
{
    RegisterStd9SliceBox(129, &charnameboxInfo, rect);
    return &charnameboxInfo;
}

ImageInfo* RegisterChoiceBox(const Rect2Int* rect)
{
    RegisterStd9SliceBox(130, &choiceboxInfo, rect);
    return &choiceboxInfo;
}

void DrawNormalImage(ImageInfo* img, Rect2Int* outRect)
{
    int dx = img->boundRect.pos.x;
    int dy = img->boundRect.pos.y;
    int sw = img->boundRect.size.x;
    int sh = img->boundRect.size.y;
    int x, y, w, h;
    int sx, sy;
    if (outRect == 0) //Draw whole image in normal rectangle
    {
        x = dx; y= dy; w = sw; h = sh;
        sx = 0; sy = 0;
    }
    else
    {
        x = outRect->pos.x;
        y = outRect->pos.y;
        w = outRect->size.x;
        h = outRect->size.y;
        if (x + w > dx + sw) w = dx + sw - x;
        if (x < dx) x = dx;
        if (y + h > dy + sh) h = dy + sh - y;
        if (y < dy) y = dy;
        sx = x - dx; sy = y - dy;
        if (sx <= 0 || sy <= 0) return; //Rectangle irretrievably outside range, so don't bother to draw
    }
    int dbytex = ((x + 0xF) & 0xFFF0) >> 3;
    int dwordw = ((w + 0xF) & 0xFFF0) >> 4;
    int dsttlpos = dbytex + 80 * y;
    int dstaddamt = 2 * (40 - dwordw);
    int sbytex = ((sx + 0xF) & 0xFFF0) >> 3;
    int swordw = ((sw + 0xF) & 0xFFF0) >> 4;
    int srctlpos = sbytex + 2 * swordw * sy;
    int srcaddamt = 2 * (swordw - dwordw);
    int xinblock = x & 0xF;

    if (xinblock) //Unaligned
    {
        if (img->mask == 0) //No mask
        {
            return; //Not implemented yet
        }
        else //Yes mask
        {
            return; //Not implemented yet
        }
    }
    else //Aligned
    {
        if (img->mask == 0) //No mask
        {
            EGCDisable(); //Better to turn off the EGC for a bit if we don't even need it

            if (img->plane0 != 0)
            {
                unsigned short planeOffset = ((unsigned short)img->plane0) + srctlpos;
                unsigned short planeSegment = (unsigned short)(((unsigned long)img->plane0) >> 16);
                __asm (
                    "movw $0xA800, %%ax\n\t"
                    "movw %%ax, %%es\n\t"
                    "movw %0, %%si\n\t"
                    "movw %1, %%di\n\t"
                    "movw %2, %%dx\n\t"
                    "movw %3, %%bx\n\t"
                    "movw %5, %%ax\n\t"
                    "movw %%ax, %%cs:.add1%=+2\n\t"
                    "movw %6, %%ax\n\t"
                    "movw %%ax, %%cs:.add2%=+2\n\t"
                    "movw %4, %%ax\n\t"
                    "movw %%ax, %%ds\n\t"
                    ".loop%=: mov %%dx, %%cx\n\t"
                    "rep movsw\n\t"
                    ".add1%=: addw $0x6969, %%si\n\t" //self-modifying code, immediate overwritten with srcaddamt
                    ".add2%=: addw $0x6969, %%di\n\t" //self-modifying code, immediate overwritten with dstaddamt
                    "decw %%bx\n\t"
                    "jnz .loop%=\n\t"
                : : "m" (planeOffset), "m" (dsttlpos), "m" (dwordw), "m" (h), "m" (planeSegment), "m" (srcaddamt), "m" (dstaddamt) : "%ax", "%cx", "%dx", "%bx", "%si", "%di", "%ds", "%es" );
            }

            if (img->plane1 != 0)
            {
                unsigned short planeOffset = ((unsigned short)img->plane1) + srctlpos;
                unsigned short planeSegment = (unsigned short)(((unsigned long)img->plane1) >> 16);
                __asm (
                    "movw $0xB000, %%ax\n\t"
                    "movw %%ax, %%es\n\t"
                    "movw %0, %%si\n\t"
                    "movw %1, %%di\n\t"
                    "movw %2, %%dx\n\t"
                    "movw %3, %%bx\n\t"
                    "movw %5, %%ax\n\t"
                    "movw %%ax, %%cs:.add1%=+2\n\t"
                    "movw %6, %%ax\n\t"
                    "movw %%ax, %%cs:.add2%=+2\n\t"
                    "movw %4, %%ax\n\t"
                    "movw %%ax, %%ds\n\t"
                    ".loop%=: mov %%dx, %%cx\n\t"
                    "rep movsw\n\t"
                    ".add1%=: addw $0x6969, %%si\n\t" //self-modifying code, immediate overwritten with srcaddamt
                    ".add2%=: addw $0x6969, %%di\n\t" //self-modifying code, immediate overwritten with dstaddamt
                    "decw %%bx\n\t"
                    "jnz .loop%=\n\t"
                : : "m" (planeOffset), "m" (dsttlpos), "m" (dwordw), "m" (h), "m" (planeSegment), "m" (srcaddamt), "m" (dstaddamt) : "%ax", "%cx", "%dx", "%bx", "%si", "%di", "%ds", "%es" );
            }

            if (img->plane2 != 0)
            {
                unsigned short planeOffset = ((unsigned short)img->plane2) + srctlpos;
                unsigned short planeSegment = (unsigned short)(((unsigned long)img->plane2) >> 16);
                __asm (
                    "movw $0xB800, %%ax\n\t"
                    "movw %%ax, %%es\n\t"
                    "movw %0, %%si\n\t"
                    "movw %1, %%di\n\t"
                    "movw %2, %%dx\n\t"
                    "movw %3, %%bx\n\t"
                    "movw %5, %%ax\n\t"
                    "movw %%ax, %%cs:.add1%=+2\n\t"
                    "movw %6, %%ax\n\t"
                    "movw %%ax, %%cs:.add2%=+2\n\t"
                    "movw %4, %%ax\n\t"
                    "movw %%ax, %%ds\n\t"
                    ".loop%=: mov %%dx, %%cx\n\t"
                    "rep movsw\n\t"
                    ".add1%=: addw $0x6969, %%si\n\t" //self-modifying code, immediate overwritten with srcaddamt
                    ".add2%=: addw $0x6969, %%di\n\t" //self-modifying code, immediate overwritten with dstaddamt
                    "decw %%bx\n\t"
                    "jnz .loop%=\n\t"
                : : "m" (planeOffset), "m" (dsttlpos), "m" (dwordw), "m" (h), "m" (planeSegment), "m" (srcaddamt), "m" (dstaddamt) : "%ax", "%cx", "%dx", "%bx", "%si", "%di", "%ds", "%es" );
            }

            if (img->plane3 != 0)
            {
                unsigned short planeOffset = ((unsigned short)img->plane3) + srctlpos;
                unsigned short planeSegment = (unsigned short)(((unsigned long)img->plane3) >> 16);
                __asm (
                    "movw $0xE000, %%ax\n\t"
                    "movw %%ax, %%es\n\t"
                    "movw %0, %%si\n\t"
                    "movw %1, %%di\n\t"
                    "movw %2, %%dx\n\t"
                    "movw %3, %%bx\n\t"
                    "movw %5, %%ax\n\t"
                    "movw %%ax, %%cs:.add1%=+2\n\t"
                    "movw %6, %%ax\n\t"
                    "movw %%ax, %%cs:.add2%=+2\n\t"
                    "movw %4, %%ax\n\t"
                    "movw %%ax, %%ds\n\t"
                    ".loop%=: mov %%dx, %%cx\n\t"
                    "rep movsw\n\t"
                    ".add1%=: addw $0x6969, %%si\n\t" //self-modifying code, immediate overwritten with srcaddamt
                    ".add2%=: addw $0x6969, %%di\n\t" //self-modifying code, immediate overwritten with dstaddamt
                    "decw %%bx\n\t"
                    "jnz .loop%=\n\t"
                : : "m" (planeOffset), "m" (dsttlpos), "m" (dwordw), "m" (h), "m" (planeSegment), "m" (srcaddamt), "m" (dstaddamt) : "%ax", "%cx", "%dx", "%bx", "%si", "%di", "%ds", "%es" );
            }

            EGCEnable();
        }
        else //Yes mask
        {
            return; //Not implemented yet
        }
    }

    EGCSetPlaneAccess(0xF);
    EGCSetMask(0xFFFF);
    EGCSetBGColour(0xC);
    EGCSetToMonochromeDrawMode();
}

void Draw9SliceBoxVRAM(ImageInfo* img, Rect2Int* outRect)
{
    int x = img->boundRect.pos.x;
    int y = img->boundRect.pos.y;
    int w = img->boundRect.size.x;
    int h = img->boundRect.size.y;
    EGCSetToVRAMBlit();
    EGCSetBitLength(16);
    int bytex = ((x + 0xF) & 0xFFF0) >> 3;
    int bytew = ((w + 0xF) & 0xFFF0) >> 3;
    int wordw = ((w + 0xF) & 0xFFF0) >> 4;
    int trueh = (h + 0xF) & 0xFFF0;
    int tileh = trueh >> 4;
    int tlpos = bytex + 80 * y;
    int trpos = tlpos + bytew - 2;
    int blpos = bytex + 80 * (y + trueh - 16);
    int brpos = blpos + bytew - 2;
    int tpos = tlpos + 2;
    int bpos = blpos + 2;
    int lpos = tlpos + 1280;
    int rpos = trpos + 1280;
    int cpos = lpos + 2;
    int unrolledIter = ((tileh - 3) >> 2) + 1;
    int jumpAmt = (3 - ((tileh - 3) & 0x3)) * 5; //stosw + add r16, imm16 is 5 bytes
    int centerh = tileh - 2;
    int centerw = wordw - 2;
    int addamt = 2 * (40 - centerw);
    int extAddamt = addamt + 1200;
    int vertSubAmt = 1280 * centerh - 80;

    unsigned short tileaddr[9];
    tileaddr[0] = (unsigned short)img->plane0;
    for (int i = 1; i < 9; i++)
    {
        tileaddr[i] = tileaddr[0] + i * 0x20;
    }

    __asm (
        "movw $0xA800, %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t" //Point both data segments to plane 0
        "movw %16, %%si\n\t" //Top left tile
        "movw %0, %%di\n\t"
        "leaw 0x500(%%di), %%bx\n\t"
        "movw $78, %%dx\n\t"
        ".loop1%=: movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "cmpw %%bx, %%di\n\t"
        "jne .loop1%=\n\t"

        "movw %18, %%si\n\t" //Top right tile
        "movw %1, %%di\n\t"
        "leaw 0x500(%%di), %%bx\n\t"
        ".loop2%=: movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "cmpw %%bx, %%di\n\t"
        "jne .loop2%=\n\t"

        "movw %22, %%si\n\t" //Bottom left tile
        "movw %2, %%di\n\t"
        "leaw 0x500(%%di), %%bx\n\t"
        ".loop3%=: movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "cmpw %%bx, %%di\n\t"
        "jne .loop3%=\n\t"

        "movw %24, %%si\n\t" //Bottom right tile
        "movw %3, %%di\n\t"
        "leaw 0x500(%%di), %%bx\n\t"
        ".loop4%=: movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "movsw\n\t"
        "addw %%dx, %%di\n\t"
        "cmpw %%bx, %%di\n\t"
        "jne .loop4%=\n\t"

        "movw %19, %%si\n\t" //Left tile
        "movw %4, %%di\n\t"
        "movw $.loop51%=, %%bx\n\t"
        "addw %10, %%bx\n\t"
        "movw $16, %%dx\n\t"
        ".loop5%=: lodsw\n\t"
        "movw %9, %%cx\n\t"
        "jmp *%%bx\n\t"
        ".loop51%=: stosw\n\t"
        "addw $1278, %%di\n\t"
        "stosw\n\t"
        "addw $1278, %%di\n\t"
        "stosw\n\t"
        "addw $1278, %%di\n\t"
        "stosw\n\t"
        "addw $1278, %%di\n\t"
        "decw %%cx\n\t"
        "jnz .loop51%=\n\t"
        "subw %13, %%di\n\t"
        "decw %%dx\n\t"
        "jnz .loop5%=\n\t"

        "movw %21, %%si\n\t" //Right tile
        "movw %5, %%di\n\t"
        "movw $.loop61%=, %%bx\n\t"
        "addw %10, %%bx\n\t"
        "movw $16, %%dx\n\t"
        ".loop6%=: lodsw\n\t"
        "movw %9, %%cx\n\t"
        "jmp *%%bx\n\t"
        ".loop61%=: stosw\n\t"
        "addw $1278, %%di\n\t"
        "stosw\n\t"
        "addw $1278, %%di\n\t"
        "stosw\n\t"
        "addw $1278, %%di\n\t"
        "stosw\n\t"
        "addw $1278, %%di\n\t"
        "decw %%cx\n\t"
        "jnz .loop61%=\n\t"
        "subw %13, %%di\n\t"
        "decw %%dx\n\t"
        "jnz .loop6%=\n\t"

        "movw %17, %%si\n\t" //Top tile
        "movw %6, %%di\n\t"
        "leaw 0x500(%%di), %%bx\n\t"
        "movw %12, %%dx\n\t"
        "push %%bp\n\t"
        "movw %11, %%bp\n\t"
        ".loop7%=: lodsw\n\t"
        "movw %%bp, %%cx\n\t"
        "rep stosw\n\t"
        "addw %%dx, %%di\n\t"
        "cmpw %%bx, %%di\n\t"
        "jne .loop7%=\n\t"
        "pop %%bp\n\t"

        "movw %23, %%si\n\t" //Bottom tile
        "movw %7, %%di\n\t"
        "leaw 0x500(%%di), %%bx\n\t"
        "movw %12, %%dx\n\t"
        "push %%bp\n\t"
        "movw %11, %%bp\n\t"
        ".loop8%=: lodsw\n\t"
        "movw %%bp, %%cx\n\t"
        "rep stosw\n\t"
        "addw %%dx, %%di\n\t"
        "cmpw %%bx, %%di\n\t"
        "jne .loop8%=\n\t"
        "pop %%bp\n\t"

        "movw %20, %%si\n\t" //Center tile
        "movw %8, %%di\n\t"
        "movw $16, %%dx\n\t"
        "movw %13, %%ax\n\t"
        "movw %%ax, %%cs:.sub1%=+2\n\t"
        "movw %14, %%ax\n\t"
        "movw %%ax, %%cs:.mov1%=+1\n\t"
        "movw %15, %%ax\n\t"
        "movw %%ax, %%cs:.add1%=+2\n\t"
        "push %%bp\n\t"
        "movw %11, %%bp\n\t"
        ".loop91%=: lodsw\n\t"
        ".mov1%=: movw $0x6969, %%bx\n\t" //self-modifying code, immediate overwritten with centerh
        ".loop9%=: movw %%bp, %%cx\n\t"
        "rep stosw\n\t"
        ".add1%=: addw $0x6969, %%di\n\t" //self-modifying code, immediate overwritten with extAddamt
        "decw %%bx\n\t"
        "jnz .loop9%=\n\t"
        ".sub1%=: subw $0x6969, %%di\n\t" //self-modifying code, immediate overwritten with vertSubAmt
        "decw %%dx\n\t"
        "jnz .loop91%=\n\t"
        "pop %%bp\n\t"
    : : "m" (tlpos), "m" (trpos), "m" (blpos), "m" (brpos), "m" (lpos), "m" (rpos), "m" (tpos), "m" (bpos), "m" (cpos), "m" (unrolledIter), "m" (jumpAmt), "m" (centerw), "m" (addamt), "m" (vertSubAmt), "m" (centerh), "m" (extAddamt), "m" (tileaddr[0]), "m" (tileaddr[1]), "m" (tileaddr[2]), "m" (tileaddr[3]), "m" (tileaddr[4]), "m" (tileaddr[5]), "m" (tileaddr[6]), "m" (tileaddr[7]), "m" (tileaddr[8]) : "%ax", "%cx", "%dx", "%bx", "%si", "%di", "%ds", "%es");

    EGCSetToMonochromeDrawMode();
}

void Draw9SliceBoxInnerRegion(ImageInfo* img)
{
    int x = img->boundRect.pos.x;
    int y = img->boundRect.pos.y;
    int w = img->boundRect.size.x;
    int h = img->boundRect.size.y;
    EGCSetToVRAMBlit();
    EGCSetBitLength(16);
    int bytex = ((x + 0xF) & 0xFFF0) >> 3;
    int bytew = ((w + 0xF) & 0xFFF0) >> 3;
    int wordw = ((w + 0xF) & 0xFFF0) >> 4;
    int trueh = (h + 0xF) & 0xFFF0;
    int tileh = trueh >> 4;
    int tlpos = bytex + 80 * y;
    int blpos = bytex + 80 * (y + trueh - 16);
    int bpos = blpos + 2;
    int cpos = tlpos + 1282;
    int centerh = tileh - 2;
    int centerw = wordw - 2;
    int extAddamt = 2 * (40 - centerw) + 1200;
    int vertSubAmt = 1280 * centerh - 80;

    unsigned short tileaddr[2];
    tileaddr[0] = (unsigned short)img->plane0 + 0x80;
    tileaddr[1] = tileaddr[0] + 0x60;

    __asm (
        "movw $0xA800, %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t" //Point both data segments to plane 0
        "movw %6, %%si\n\t" //Center tile
        "movw %0, %%di\n\t"
        "movw $16, %%dx\n\t"
        "movw %3, %%ax\n\t"
        "movw %%ax, %%cs:.sub1%=+2\n\t"
        "movw %4, %%ax\n\t"
        "movw %%ax, %%cs:.mov1%=+1\n\t"
        "movw %5, %%ax\n\t"
        "movw %%ax, %%cs:.add1%=+2\n\t"
        "push %%bp\n\t"
        "movw %2, %%bp\n\t"
        ".loop1%=: lodsw\n\t"
        ".mov1%=: movw $0x6969, %%bx\n\t" //self-modifying code, immediate overwritten with centerh
        ".loop0%=: movw %%bp, %%cx\n\t"
        "rep stosw\n\t"
        ".add1%=: addw $0x6969, %%di\n\t" //self-modifying code, immediate overwritten with extAddamt
        "decw %%bx\n\t"
        "jnz .loop0%=\n\t"
        ".sub1%=: subw $0x6969, %%di\n\t" //self-modifying code, immediate overwritten with vertSubAmt
        "decw %%dx\n\t"
        "jnz .loop1%=\n\t"
        "pop %%bp\n\t"

        "movw %7, %%si\n\t" //Bottom tile (only top row needed, shadow wiping)
        "movw %1, %%di\n\t"
        "lodsw\n\t"
        "movw %2, %%cx\n\t"
        "rep stosw\n\t"
    : : "m" (cpos), "m" (bpos), "m" (centerw), "m" (vertSubAmt), "m" (centerh), "m" (extAddamt), "m" (tileaddr[0]), "m" (tileaddr[1]) : "%ax", "%cx", "%dx", "%bx", "%si", "%di", "%ds", "%es");

    EGCSetToMonochromeDrawMode();
}

void DrawImage(ImageInfo* img)
{
    //Basic validation
    if (img == 0) return; //Don't bother if pointer to image data is null (i.e. known not to be valid)
    unsigned char fl = img->flags;
    if (!(fl & IMAGE_LOADED) || fl & IMAGE_DRAWN) return; //Don't bother if image is not loaded or already drawn
    if (fl & IMAGE_ALIGN_FIXED) //Force alignment
    {
        img->boundRect.pos.x &= 0xFFF0;
    }

    //Determine what of the full rectangle to draw
    //TODO: write this code

    //Draw the image
    if (fl & IMAGE_TYPE_9SLICE)
    {
        if (fl & IMAGE_MEM_VRAM)
        {
            Draw9SliceBoxVRAM(img, (Rect2Int*)0);
        }
        else
        {
            return; //Not valid, there are so few 9-sliced box types right now that's it's much better to just put them into VRAM
        }
    }
    else
    {
        if (fl & IMAGE_MEM_VRAM)
        {
            return; //Not valid, free VRAM leftover is too small for this to be practical with generic images, so we don't support it here
        }
        else
        {
            DrawNormalImage(img, (Rect2Int*)0);
        }
    }

    img->flags |= IMAGE_DRAWN;
}

void UndrawImage(ImageInfo* img)
{
    //Basic validation
    if (img == 0) return; //Don't bother if pointer to image data is null (i.e. known not to be valid)
    unsigned char fl = img->flags;
    if (!(fl & IMAGE_LOADED) || !(fl & IMAGE_DRAWN)) return; //Don't bother if image is not loaded or not drawn

    Rect2Int overdrawRect = img->boundRect;
    int lx = overdrawRect.pos.x;
    int rx = lx + overdrawRect.size.x;
    lx &= 0xFFF0;
    rx = ((rx + 0xF) & 0xFFF0);
    overdrawRect.size.x = rx - lx;
    overdrawRect.pos.x = lx;
    DrawNormalImage(&bgInfo, &overdrawRect); //Temporary simple assumption

    img->flags &= ~(IMAGE_DRAWN);
}

void DoDrawRequests()
{
    for (int i = 0; i < 23; i++)
    {
        ImageInfo* img = allInfos[i];
        unsigned char fl = img->flags;
        if (!(fl & IMAGE_LOADED)) continue; //Don't bother if image is not loaded
        fl ^= img->flags >> 1; //Does IMAGE_DRAWN ^ IMAGE_DRAWREQ
        if (!(fl & IMAGE_DRAWREQ)) continue; //Don't bother if the image's draw state is not going to change
        fl = img->flags & IMAGE_DRAWREQ;
        if (fl) DrawImage(img);
        else UndrawImage(img);
    }
}

void RedrawEverything()
{
    for (int i = 0; i < 23; i++)
    {
        ImageInfo* img = allInfos[i];
        unsigned char fl = img->flags;
        if (!(fl & IMAGE_LOADED)) continue; //Don't bother if image is not loaded
        if (fl & IMAGE_DRAWREQ) img->flags |= IMAGE_DRAWN; //Pass through draw request
        fl = img->flags & IMAGE_DRAWN;
        if (fl)
        {
            img->flags &= ~(IMAGE_DRAWN); //Mark as not drawn so that DrawImage can do its magic
            DrawImage(img);
        }
    }
}


int InitialiseGraphicsSystem()
{
    //Check if key files exist and get key numbers
    unsigned int realReadLen;
    unsigned long curfilepos;
    fileptr handle;
    int result = OpenFile(rootInfo.BGDataPath,  DOSFILE_OPEN_READ, &handle);
    if (result)
    {
        WriteString("Error! Could not find BG data file!", 180, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result;
    }
    __far unsigned char* bgnp = (__far unsigned char*)&numBg;
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, 4, &curfilepos);
    ReadFile(handle, 2, bgnp, &realReadLen);
    CloseFile(handle);
    result = OpenFile(rootInfo.spriteDataPath,  DOSFILE_OPEN_READ, &handle);
    if (result)
    {
        WriteString("Error! Could not find sprite data file!", 174, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result;
    }
    __far unsigned char* spnp = (__far unsigned char*)&numSpr;
    ReadFile(handle, 2, spnp, &realReadLen);
    CloseFile(handle);

    LoadStd9SliceBoxIntoVRAM();
    bgImageDataPlane0 = (__far unsigned char*)MemAlloc(32000);
    bgImageDataPlane1 = (__far unsigned char*)MemAlloc(32000);
    bgImageDataPlane2 = (__far unsigned char*)MemAlloc(32000);
    bgImageDataPlane3 = (__far unsigned char*)MemAlloc(32000); //There will always be a background image loaded at some point, so allocate these now

    //Flag all image infos as unloaded
    bgInfo.flags = 0;
    bgInfo.id = 0xFFFF;
    textboxInfo.flags = 0;
    charnameboxInfo.flags = 0;
    choiceboxInfo.flags = 0;
    sprite1Info.flags = 0;
    sprite1Info.id = 0xFFFF;
    sprite2Info.flags = 0;
    sprite2Info.id = 0xFFFF;
    sprite3Info.flags = 0;
    sprite3Info.id = 0xFFFF;
    for (int i = 0; i < 4; i++)
    {
        bgVariants[i].flags = 0;
        bgVariants[i].id = 0xFFFF;
        sprite1Variants[i].flags = 0;
        sprite1Variants[i].id = 0xFFFF;
        sprite2Variants[i].flags = 0;
        sprite2Variants[i].id = 0xFFFF;
        sprite3Variants[i].flags = 0;
        sprite3Variants[i].id = 0xFFFF;
    }

    return 0;
}

void FreeGraphicsSystem()
{
    MemFree(bgImageDataPlane0);
    MemFree(bgImageDataPlane1);
    MemFree(bgImageDataPlane2);
    MemFree(bgImageDataPlane3);
}
