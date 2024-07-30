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

#include "platform/x86strops.h"
#include "platform/memalloc.h"
#include "lz4.h"
#include "gpimage.h"

static const char gpiMagicNumber[3] = {'G', 'P', 'I'};

unsigned char filterBuffer[1024];

int OpenGPIFile(GPIInfo* info)
{
    unsigned char header[GPI_HEADER_SIZE];
    unsigned short bytesRead;
    ReadFile(info->handle, GPI_HEADER_SIZE, (__far unsigned char*)header, &bytesRead);
    //Check magic number to determine if this is a genuine GPI file
    for (int i = 0; i < 3; i++)
    {
        if (header[i] != gpiMagicNumber[i])
        {
            return -1; //Oops, no it isn't
        }
    }
    info->flags                    = header[0x03];
    unsigned short w               = *((unsigned short*)(&header[0x04])) + 1;
    unsigned short h               = *((unsigned short*)(&header[0x06])) + 1;
    unsigned short nT              = *((unsigned short*)(&header[0x08])) + 1;
    unsigned short planeMask       = *((unsigned short*)(&header[0x0A]));
    unsigned short planeFilterMask = *((unsigned short*)(&header[0x0C]));
    info->width = w;
    info->height = h;
    info->numTiles = nT;
    unsigned short bw = (w + 7)/8;
    info->byteWidth = bw;
    unsigned long dh = ((unsigned long)h) * ((unsigned long)nT);
    info->decHeight = dh;
    unsigned long bpp = ((unsigned long)bw) * dh;
    info->bytesPerPlane = bpp;

    int np = 0;
    unsigned short pfm = 0;
    unsigned short pfBit = 0x01;
    unsigned short pTest = 0x0001;
    for (int i = 0; i < 4; i++)
    {
        if (planeMask & pTest)
        {
            if (!info->planes[np]) info->planes[np] = (__far unsigned char*)MemAlloc(bpp);
            if (planeFilterMask & pTest) pfm |= pfBit;
            pfBit <<= 1;
            np++;
        }
        pTest <<= 1;
    }
    if (planeMask & 0x0100)
    {
        info->hasMask = 1;
        if (!info->maskPlane) info->maskPlane = (__far unsigned char*)MemAlloc(bpp);
        if (planeFilterMask & 0x0100) pfm |= 0x0100;
        np++;
    }
    else info->hasMask = 0;
    info->filtPlanes = pfm;
    info->numPlanes = np;

    return 0; //File ready to read into planes
}

void DecompressGPIFile(GPIInfo* info)
{
    unsigned short h = info->height;
    unsigned short pw = info->byteWidth;
    unsigned short dh = info->decHeight;
    unsigned short filt = info->filtPlanes;
    unsigned long planeSize = info->bytesPerPlane;
    //Each plane is decompressed the same way
    unsigned short filtCheck = 0x01;
    for (int i = 0; i < info->numPlanes; i++)
    {
        __far unsigned char* pptr;
        unsigned char isFiltered;
        if (info->hasMask)
        {
            if (i == 0)
            {
                pptr = info->maskPlane;
                isFiltered = (unsigned char)((filt & 0x0100) != 0);
            }
            else
            {
                pptr = info->planes[i - 1];
                isFiltered = (unsigned char)((filt & (filtCheck >> 1)) != 0);
            }
        }
        else
        {
            pptr = info->planes[i];
            isFiltered = (unsigned char)((filt & filtCheck) != 0);

        }
        unsigned short bytesRead;
        if (isFiltered) ReadFile(info->handle, (dh+1)/2, (__far unsigned char*)filterBuffer, &bytesRead);
        unsigned long compressedSize;
        __far unsigned char* cpsp = &compressedSize;
        ReadFile(info->handle, 4, cpsp, &bytesRead);
        __far unsigned char* decompressionBuffer = MemAlloc(compressedSize + 4);
        *((__far unsigned long*)(&decompressionBuffer[0])) = compressedSize;
        ReadFile(info->handle, compressedSize, (__far unsigned char*)(decompressionBuffer + 4), &bytesRead);
        unsigned int decSize = LZ4Decompress(pptr, decompressionBuffer);
        MemFree(decompressionBuffer);
        filtCheck <<= 1;
        if (!isFiltered) continue; //Skip defiltering if unnecessary

        //Defilter in-place
        __far unsigned char *pptr1, *pptr2, *pptr3, *pptr4;
        if (info->hasMask)
        {
            if (i == 0) pptr = info->maskPlane;
            else pptr = info->planes[i - 1];
            if (i == 1) pptr1 = info->maskPlane;
            else pptr1 = info->planes[i - 2];
            if (i == 2) pptr2 = info->maskPlane;
            else pptr2 = info->planes[i - 3];
            if (i == 3) pptr3 = info->maskPlane;
            else pptr3 = info->planes[i - 4];
            pptr4 = info->maskPlane;
        }
        else
        {
            pptr = info->planes[i];
            pptr1 = info->planes[i - 1];
            pptr2 = info->planes[i - 2];
            pptr3 = info->planes[i - 3];
        }
        for (int j = 0; j < dh; j++)
        {
            int curFiltSpec = filterBuffer[j >> 1];
            curFiltSpec = (j & 1 ? curFiltSpec >> 4 : curFiltSpec) & 0xF;
            int curRow = j * pw;
            unsigned char carry;
            switch (curFiltSpec)
            {
                case 0x0:
                    break; //Nothing to be done
                case 0x1:
                    carry = 0x00;
                    for (int k = 0; k < pw; k++)
                    {
                        unsigned char in = pptr[k + curRow];
                        in ^= carry;
                        in ^= in >> 1;
                        in ^= in >> 2;
                        in ^= in >> 4;
                        carry = (in & 0x01) << 7;
                        pptr[k + curRow] = in;
                    }
                    break;
                case 0x2:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr[k + curRow - pw];
                    }
                    break;
                case 0x3:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr[k + curRow - pw * h];
                    }
                    break;
                case 0x4:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr1[k + curRow];
                    }
                    break;
                case 0x5:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr2[k + curRow];
                    }
                    break;
                case 0x6:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr3[k + curRow];
                    }
                    break;
                case 0x7:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= pptr4[k + curRow];
                    }
                    break;
                case 0x8:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] = ~pptr[k + curRow];
                    }
                    break;
                case 0x9:
                    carry = 0x00;
                    for (int k = 0; k < pw; k++)
                    {
                        unsigned char in = ~pptr[k + curRow];
                        in ^= carry;
                        in ^= in >> 1;
                        in ^= in >> 2;
                        in ^= in >> 4;
                        carry = (in & 0x01) << 7;
                        pptr[k + curRow] = in;
                    }
                    break;
                case 0xA:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr[k + curRow - pw];
                    }
                    break;
                case 0xB:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr[k + curRow - pw * h];
                    }
                    break;
                case 0xC:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr1[k + curRow];
                    }
                    break;
                case 0xD:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr2[k + curRow];
                    }
                    break;
                case 0xE:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr3[k + curRow];
                    }
                    break;
                case 0xF:
                    for (int k = 0; k < pw; k++)
                    {
                        pptr[k + curRow] ^= ~pptr4[k + curRow];
                    }
                    break;
            }
        }
    }
    if (info->hasMask) //NOT the mask to make it slightly faster to use later on
    {
        __far unsigned char* pptr = info->maskPlane;
        for (int i = 0; i < dh * pw; i++)
        {
            pptr[i] = ~pptr[i];
        }
    }
}
