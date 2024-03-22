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
 * Root info data structure and loader
 */

#pragma once

typedef struct
{
    unsigned short VNFlags;
    unsigned short numstvar_glob;
    unsigned short numflags_glob;
    unsigned short numstvar_loc;
    unsigned short numflags_loc;
    unsigned short defFormatNormal;
    unsigned short defFormatCharName;
    unsigned short defFormatMenuItem;
    unsigned short defFormatMenuItemSelected;
    unsigned short numLang;
    unsigned short curLang;
    char sceneDataPath[13];
    char langDataPath[13];
    char curTextDataPath[13];
    char BGDataPath[13];
    char spriteDataPath[13];
    char musicDataPath[13];
    char sfxDataPath[13];
    char systemDataPath[13];
} RootInfo;

extern RootInfo rootInfo;

//Read in the root info from ROOTINFO.DAT
int ReadInRootInfo();
//Initialise the language setting
int InitLanguage(unsigned short lang);
//Change the current language
int ChangeLanguage(unsigned short newLang);
