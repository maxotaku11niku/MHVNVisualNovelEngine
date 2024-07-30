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

#pragma once

typedef struct
{
    int x;
    int y;
} Vector2Int;

typedef struct
{
    Vector2Int pos;
    Vector2Int size;
} Rect2Int;

#define IMAGE_TYPE_NORMAL 0x00
#define IMAGE_TYPE_9SLICE 0x01
#define IMAGE_MEM_NORMAL  0x00
#define IMAGE_MEM_VRAM    0x02
#define IMAGE_ALIGN_FREE  0x00
#define IMAGE_ALIGN_FIXED 0x04
#define IMAGE_LOADED      0x80
#define IMAGE_DRAWN       0x40
#define IMAGE_DRAWREQ     0x20

typedef struct imginf
{
    Rect2Int boundRect;
    __far unsigned char* mask;
    __far unsigned char* plane0;
    __far unsigned char* plane1;
    __far unsigned char* plane2;
    __far unsigned char* plane3;
    struct imginf* children;
    unsigned short id;
    unsigned char layer;
    unsigned char flags;
} ImageInfo;

void UnloadImage(ImageInfo* img);
ImageInfo* LoadBGImage(unsigned int num);
void DoDrawRequests();
void RedrawEverything();
void LoadStd9SliceBoxIntoVRAM();
ImageInfo* RegisterTextBox(const Rect2Int* rect);
ImageInfo* RegisterCharNameBox(const Rect2Int* rect);
ImageInfo* RegisterChoiceBox(const Rect2Int* rect);
void Draw9SliceBoxInnerRegion(ImageInfo* img);
int InitialiseGraphicsSystem();
void FreeGraphicsSystem();
