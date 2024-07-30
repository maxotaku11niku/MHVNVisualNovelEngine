/* GPIVIEW - reference implementation of a viewer of GPI files for DOS based systems
 * Copyright (c) 2024 Maxim Hoxha
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
 * GPI reader and decoder
 */

#pragma once

#include "platform/filehandling.h"

#define GPI_HEADER_SIZE 0x0E

#define GPI_COMPRESSION         0x01
#define GPI_COMPRESSION_LZ4     0x00
#define GPI_ENDIAN              0x04
#define GPI_ENDIAN_BIG          0x00
#define GPI_ENDIAN_LITTLE       0x04
#define GPI_BPC                 0x08
#define GPI_BPC_4               0x00
#define GPI_BPC_8               0x08

typedef struct
{
    __far unsigned char* planes[4];
    __far unsigned char* maskPlane;
    unsigned long bytesPerPlane;
    unsigned long decHeight;
    fileptr handle;
    unsigned short width;
    unsigned short byteWidth;
    unsigned short height;
    unsigned short numTiles;
    unsigned short filtPlanes;
    unsigned char flags;
    unsigned char hasMask;
    unsigned char numPlanes;
} GPIInfo;

//Set the handle and make sure that the file pointer is pointing at the beginning of the GPI file. Unset plane pointers will be allocated.
int OpenGPIFile(GPIInfo* info);
void DecompressGPIFile(GPIInfo* info);
