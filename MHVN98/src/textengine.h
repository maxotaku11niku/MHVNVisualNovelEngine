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
 * Text drawing engine
 */

#pragma once

#include "graphics.h"

#define FORMAT_BOLD           0x0001
#define FORMAT_ITALIC         0x0002
#define FORMAT_UNDERLINE      0x0004
#define FORMAT_SHADOW         0x0008
#define FORMAT_FADE(n) (((n) << 8) & 0x0F00)
#define FORMAT_COLOUR(n) (((n) << 12) & 0xF000)
#define FORMAT_FADE_GET(f) (((f) & 0x0F00) >> 8)
#define FORMAT_COLOUR_GET(f) (((f) & 0xF000) >> 12)
#define FORMAT_PART_MAIN   0x000F
#define FORMAT_PART_UNUSED 0x00F0
#define FORMAT_PART_FADE   0x0F00
#define FORMAT_PART_COLOUR 0xF000

typedef struct
{
    unsigned long systemTextFilePtr;
    unsigned long creditsTextFilePtr;
    unsigned long characterNamesFilePtr;
    unsigned long sceneTextFilePtr;
    unsigned long CGTextFilePtr;
    unsigned long musicTextFilePtr;
} TextInfo;

extern TextInfo textInfo;
extern Rect2Int textBoxInnerBounds;
extern ImageInfo* textBoxImgInfo;
extern Rect2Int charNameBoxInnerBounds;
extern ImageInfo* charNameBoxImgInfo;
extern Rect2Int choiceBoxInnerBounds;
extern ImageInfo* choiceBoxImgInfo;

//Set the indices of the shadow colours
void SetShadowColours(const unsigned char* cols);

//Set up text info from the current language
int SetupTextInfo();

//Load the given character's name
int LoadCurrentCharacterName(unsigned short charNumber, char* nameBuffer);

//Load the given scene number's text
int LoadSceneText(unsigned short sceneNumber, __far char* textDataBuffer, unsigned int* textPtrsBuffer);

//Set the custom protagonist info item appropriately
void SetCustomInfo(unsigned short num, char* str);

//Write a string (str) to the screen at position (x, y) with format given by 'format'.
void WriteString(const __far char* str, const short x, const short y, short format, unsigned char autolb);

//Start an animated string write of string 'str', starting from position (x, y) with format given by 'format'
void StartAnimatedStringToWrite(const __far char* str, const short x, const short y, short format);

//Do the next frame of the string write animation
int StringWriteAnimationFrame(unsigned char skip);
