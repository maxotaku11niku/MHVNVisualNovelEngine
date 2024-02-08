#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "x86strops.h"
//#include "filehandling.h"
#include "stdbuffer.h"
#include "textengine.h"
#include "rootinfo.h"

RootInfo rootInfo;

static const char* magicNumber = "MHVN";

int ReadInRootInfo()
{
    //MemsetSeg(0, &rootInfo, sizeof(RootInfo));
    memset(&rootInfo, 0, sizeof(RootInfo)); //Zero out the root info structure
    int handle;
    unsigned int realReadLen;
    //FILE* handle = fopen("ROOTINFO.DAT", "rb");
    int result = _dos_open("ROOTINFO.DAT", 0, &handle);
    if (result)
    //if (handle == 0)
    {
        WriteString("Error! Could not find ROOTINFO.DAT!", 180, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
        return result; //Error handler
        //return 1; //Error handler
    }
    __far unsigned char* fb = smallFileBuffer;
    _dos_read(handle, fb, 0x7C, &realReadLen);
    //fread(smallFileBuffer, 1, 0x7C, handle);
    for (int i = 0; i < 4; i++)
    {
        if (smallFileBuffer[i] != magicNumber[i])
        {
            WriteString("Error! ROOTINFO.DAT is not valid!", 188, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
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
    //fclose(handle);
    _dos_close(handle);
    return 0;
}

int InitLanguage(unsigned short lang)
{
    int handle;
    unsigned int realReadLen;
    int result = _dos_open(rootInfo.langDataPath, 0, &handle);
    //FILE* handle = fopen(rootInfo.langDataPath, "rb");
    if (result)
    //if (handle == 0)
    {
        WriteString("Error! Could not find language data file!", 156, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
        return result; //Error handler
        //return 1; //Error handler
    }
    __far unsigned char* fb = smallFileBuffer;
    _dos_read(handle, fb, 1024, &realReadLen);
    //fread(smallFileBuffer, 1, 1024, handle);
    rootInfo.numLang = *((unsigned short*)(smallFileBuffer));
    rootInfo.curLang = lang;
    char* tdPath = smallFileBuffer + smallFileBuffer[2 + 4 * lang];
    for (unsigned int i = 0; i < 12; i++)
    {
        const char ch = tdPath[i];
        if (ch) rootInfo.curTextDataPath[i] = ch;
        else break;
    }
    //fclose(handle);
    _dos_close(handle);
    return 0;
}

int ChangeLanguage(unsigned short newLang)
{
    if (newLang == rootInfo.curLang) return 0; //Do not go through all this if we're not actually changing the language
    int handle;
    unsigned int realReadLen;
    int result = _dos_open(rootInfo.langDataPath, 0, &handle);
    //FILE* handle = fopen(rootInfo.langDataPath, "rb");
    if (result)
    //if (handle == 0)
    {
        WriteString("Error! Could not find language data file!", 156, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
        return result; //Error handler
        //return 1; //Error handler
    }
    __far unsigned char* fb = smallFileBuffer;
    _dos_read(handle, fb, 1024, &realReadLen);
    //fread(smallFileBuffer, 1, 1024, handle);
    rootInfo.curLang = newLang;
    char* tdPath = smallFileBuffer + smallFileBuffer[2 + 4 * newLang];
    for (unsigned int i = 0; i < 12; i++)
    {
        const char ch = tdPath[i];
        if (ch) rootInfo.curTextDataPath[i] = ch;
        else break;
    }
    //fclose(handle);
    _dos_close(handle);
    return 0;
}
