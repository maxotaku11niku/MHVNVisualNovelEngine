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

#include "platform/x86strops.h"
#include "platform/filehandling.h"
#include "stdbuffer.h"
#include "textengine.h"
#include "rootinfo.h"

RootInfo rootInfo;

static const char* magicNumber = "MHVN";

int ReadInRootInfo()
{
    MemsetNear(0, &rootInfo, sizeof(RootInfo)); //Zero out the root info structure
    fileptr handle;
    unsigned int realReadLen;
    int result = OpenFile("ROOTINFO.DAT", DOSFILE_OPEN_READ, &handle);
    if (result)
    {
        WriteString("Error! Could not find ROOTINFO.DAT!", 180, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result;
    }
    __far unsigned char* fb = smallFileBuffer;
    ReadFile(handle, 0x7C, fb, &realReadLen);
    for (int i = 0; i < 4; i++)
    {
        if (smallFileBuffer[i] != magicNumber[i])
        {
            WriteString("Error! ROOTINFO.DAT is not valid!", 188, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
            return 0xFF; //Wrong file format error
        }
    }
    //Read in all root info
    rootInfo.VNFlags = *((unsigned short*)(smallFileBuffer + 0x04));
    rootInfo.numstvar_glob = *((unsigned short*)(smallFileBuffer + 0x06));
    rootInfo.numflags_glob = *((unsigned short*)(smallFileBuffer + 0x08));
    rootInfo.numstvar_loc = *((unsigned short*)(smallFileBuffer + 0x0A));
    rootInfo.numflags_loc = *((unsigned short*)(smallFileBuffer + 0x0C));
    rootInfo.defFormatNormal = *((unsigned short*)(smallFileBuffer + 0x0E));
    rootInfo.defFormatCharName = *((unsigned short*)(smallFileBuffer + 0x10));
    rootInfo.defFormatMenuItem = *((unsigned short*)(smallFileBuffer + 0x12));
    rootInfo.defFormatMenuItemSelected = *((unsigned short*)(smallFileBuffer + 0x14));
    for (int i = 0; i < 12; i++)
    {
        rootInfo.sceneDataPath[i] = smallFileBuffer[0x16 + i];
        rootInfo.langDataPath[i] = smallFileBuffer[0x22 + i];
        rootInfo.BGDataPath[i] = smallFileBuffer[0x2E + i];
        rootInfo.spriteDataPath[i] = smallFileBuffer[0x3A + i];
        rootInfo.musicDataPath[i] = smallFileBuffer[0x46 + i];
        rootInfo.sfxDataPath[i] = smallFileBuffer[0x52 + i];
        rootInfo.systemDataPath[i] = smallFileBuffer[0x5E + i];
    }
    return CloseFile(handle);
}

int InitLanguage(unsigned short lang)
{
    fileptr handle;
    unsigned int realReadLen;
    int result = OpenFile(rootInfo.langDataPath, DOSFILE_OPEN_READ, &handle);
    if (result)
    {
        WriteString("Error! Could not find language data file!", 156, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result;
    }
    __far unsigned char* fb = smallFileBuffer;
    ReadFile(handle, sizeof(smallFileBuffer), fb, &realReadLen);
    rootInfo.numLang = *((unsigned short*)(smallFileBuffer));
    rootInfo.curLang = lang;
    char* tdPath = smallFileBuffer + smallFileBuffer[2 + 4 * lang];
    for (unsigned int i = 0; i < 12; i++)
    {
        const char ch = tdPath[i];
        if (ch) rootInfo.curTextDataPath[i] = ch;
        else break;
    }
    return CloseFile(handle);
}

int ChangeLanguage(unsigned short newLang)
{
    if (newLang == rootInfo.curLang) return 0; //Do not go through all this if we're not actually changing the language
    return InitLanguage(newLang);
}
