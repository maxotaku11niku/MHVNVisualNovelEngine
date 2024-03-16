//Text drawing engine

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "platform/x86ports.h"
#include "platform/x86strops.h"
#include "platform/x86segments.h"
#include "platform/filehandling.h"
//#include "platform/unrealhwaddr.h"
#include "platform/pc98_crtbios.h"
#include "platform/pc98_gdc.h"
#include "platform/pc98_chargen.h"
#include "platform/pc98_keyboard.h"
#include "platform/pc98_egc.h"
#include "stdbuffer.h"
#include "rootinfo.h"
#include "sceneengine.h"
#include "textengine.h"


TextInfo textInfo;

int ctHandle;
//FILE* ctHandle;
unsigned long charbuf[16];

unsigned long animCharBuf[16 * 16]; //For fade in animation, ring buffer
short charXs[16];
short charYs[16];
char charColours[16];
char charFlags[16];
unsigned char charFade[16];
unsigned short chBufStartNum;
const char* stringToAnimWrite;
const char* curAnimStringPos;
short currentAnimWriteX;
short currentAnimWriteY;
short currentAnimNextWriteX;
short currentAnimNextWriteY;
short currentAnimDefaultFormat;
short currentAnimFormat;
unsigned char animReachedEndOfString;
short animLength;
short waitFrames;
short waitPerChar;

Rect2Int textBoxInnerBounds;
ImageInfo* textBoxImgInfo;
Rect2Int charNameBoxInnerBounds;
ImageInfo* charNameBoxImgInfo;
Rect2Int choiceBoxInnerBounds;
ImageInfo* choiceBoxImgInfo;

unsigned char shadowColours[16];

const unsigned short bayer4x4masks[64] =
{
    0x8888, 0x0000, 0x0000, 0x0000,
    0x8888, 0x0000, 0x2222, 0x0000,
    0x8888, 0x0000, 0xAAAA, 0x0000,
    0xAAAA, 0x0000, 0xAAAA, 0x0000,
    0xAAAA, 0x4444, 0xAAAA, 0x0000,
    0xAAAA, 0x4444, 0xAAAA, 0x1111,
    0xAAAA, 0x4444, 0xAAAA, 0x5555,
    0xAAAA, 0x5555, 0xAAAA, 0x5555,
    0xAAAA, 0xDDDD, 0xAAAA, 0x5555,
    0xAAAA, 0xDDDD, 0xAAAA, 0x7777,
    0xAAAA, 0xDDDD, 0xAAAA, 0xFFFF,
    0xAAAA, 0xFFFF, 0xAAAA, 0xFFFF,
    0xEEEE, 0xFFFF, 0xAAAA, 0xFFFF,
    0xEEEE, 0xFFFF, 0xBBBB, 0xFFFF,
    0xEEEE, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

char* customInfos[16];
char stringBuffer1[512];
char stringBuffer2[512];

void SetShadowColours(const unsigned char* cols)
{
    memcpy(shadowColours, cols, 16);
}

int SetupTextInfo()
{
    unsigned int realReadLen;
    int result = _dos_open(rootInfo.curTextDataPath, 0, &ctHandle);
    //ctHandle = fopen(rootInfo.curTextDataPath, "rb");
    if (result)
    //if (ctHandle == 0)
    {
        WriteString("Error! Could not find text data file!", 172, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result; //Error handler
        //return 1; //Error handler
    }
    __far unsigned char* fb = smallFileBuffer;
    _dos_read(ctHandle, fb, 0x18, &realReadLen);
    //fread(smallFileBuffer, 1, 0x18, ctHandle);
    textInfo.systemTextFilePtr = *((unsigned long*)(smallFileBuffer));
    textInfo.creditsTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x04));
    textInfo.characterNamesFilePtr = *((unsigned long*)(smallFileBuffer + 0x08));
    textInfo.sceneTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x0C));
    textInfo.CGTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x10));
    textInfo.musicTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x14));
    //fclose(ctHandle);
    _dos_close(ctHandle);
    return 0;
}

int LoadCurrentCharacterName(unsigned short charNumber, char* nameBuffer)
{
    unsigned int realReadLen;
    unsigned long curfilepos;
    unsigned short charnamepos;
    int result = _dos_open(rootInfo.curTextDataPath, 0, &ctHandle);
    //ctHandle = fopen(rootInfo.curTextDataPath, "rb");
    if (result)
    //if (ctHandle == 0)
    {
        WriteString("Error! Could not find text data file!", 172, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result; //Error handler
        //return 1; //Error handler
    }
    /**/
    SeekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.characterNamesFilePtr + 2 * charNumber, &curfilepos);
    __far unsigned char* cnp = &charnamepos;
    _dos_read(ctHandle, cnp, 2, &realReadLen);
    SeekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.characterNamesFilePtr + 2 * sceneInfo.numChars + charnamepos, &curfilepos);
    __far unsigned char* nb = nameBuffer;
    _dos_read(ctHandle, nb, 64, &realReadLen);
    _dos_close(ctHandle);
    //*/
    /*/
    fseek(ctHandle, textInfo.characterNamesFilePtr + 2 * charNumber, SEEK_SET);
    fread(&charnamepos, 1, 2, ctHandle);
    fseek(ctHandle, textInfo.characterNamesFilePtr + 2 * sceneInfo.numChars + charnamepos, SEEK_SET);
    fread(nameBuffer, 1, 64, ctHandle);
    fclose(ctHandle);
    //*/
    return 0;
}

int LoadSceneText(unsigned short sceneNumber, __far char* textDataBuffer, unsigned int* textPtrsBuffer)
{
    unsigned int realReadLen;
    unsigned long curfilepos;
    unsigned long scenedatpos;
    unsigned short numTexts;
    int result = _dos_open(rootInfo.curTextDataPath, 0, &ctHandle);
    //ctHandle = fopen(rootInfo.curTextDataPath, "rb");
    if (result)
    //if (ctHandle == 0)
    {
        WriteString("Error! Could not find text data file!", 172, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result; //Error handler
        //return 1; //Error handler
    }
    /**/
    SeekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.sceneTextFilePtr + 4 * sceneNumber, &curfilepos);
    __far unsigned char* sdp = &scenedatpos;
    _dos_read(ctHandle, sdp, 4, &realReadLen);
    SeekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.sceneTextFilePtr + 4 * sceneInfo.numScenes, &curfilepos);
    __far unsigned char* nt = &numTexts;
    _dos_read(ctHandle, nt, 2, &realReadLen);
    __far unsigned char* fb = smallFileBuffer;
    _dos_read(ctHandle, fb, 1024, &realReadLen);
    //*/
    /*/
    fseek(ctHandle, textInfo.sceneTextFilePtr + 4 * sceneNumber, SEEK_SET);
    fread(&scenedatpos, 1, 4, ctHandle);
    fseek(ctHandle, textInfo.sceneTextFilePtr + 4 * sceneInfo.numScenes, SEEK_SET);
    fread(&numTexts, 1, 2, ctHandle);
    fread(smallFileBuffer, 1, 1024, ctHandle);
    //*/
    for(int i = 0; i < numTexts; i++)
    {
        textPtrsBuffer[i] = *((unsigned short*)(smallFileBuffer) + i) + (unsigned int)(((unsigned long)textDataBuffer) & 0x0000FFFF);
    }
    /**/
    SeekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.sceneTextFilePtr + 4 * sceneInfo.numScenes + 2 * (numTexts + 1), &curfilepos);
    _dos_read(ctHandle, textDataBuffer, 0x8000, &realReadLen);
    if (realReadLen == 0x8000) _dos_read(ctHandle, textDataBuffer + 0x8000, 0x8000, &realReadLen);
    _dos_close(ctHandle);
    //*/
    /*/
    fseek(ctHandle, textInfo.sceneTextFilePtr + 4 * sceneInfo.numScenes + 2 * (numTexts + 1), SEEK_SET);
    setds((unsigned short)(((unsigned long)textDataBuffer) >> 16));
    fread((unsigned short)(((unsigned long)textDataBuffer) & 0x0000FFFF), 1, 0xFFFF, ctHandle);
    resetdstoss();
    fclose(ctHandle);
    //*/
    return 0;
}

//Char data must be in 'edit-friendly' format
static void BoldenCharLeft(unsigned long* charb, int bits32)
{
    if (!bits32) //Why bother doing work on an empty cell?
    {
        for (unsigned short i = 0; i < 16; i++)
        {
            unsigned short convcharA = ((unsigned short*)charb)[2*i + 1];
            unsigned short convcharB = convcharA << 1;
            convcharB |= convcharA;
            convcharA ^= convcharB;
            convcharB &= ~(convcharA << 1);
            ((unsigned short*)charb)[2*i + 1] = convcharB;
        }
    }
    else
    {
        for (unsigned short i = 0; i < 16; i++)
        {
            unsigned long convcharA = charb[i];
            unsigned long convcharB = convcharA << 1;
            convcharB |= convcharA;
            convcharA ^= convcharB;
            convcharB &= ~(convcharA << 1);
            charb[i] = convcharB;
        }
    }
}

//Char data must be in 'edit-friendly' format
static void BoldenCharRight(unsigned long* charb, int bits32)
{
    if (!bits32) //Why bother doing work on an empty cell?
    {
        for (unsigned short i = 0; i < 16; i++)
        {
            unsigned short convcharA = ((unsigned short*)charb)[2*i + 1];
            unsigned short convcharB = convcharA >> 1;
            convcharB |= convcharA;
            convcharA ^= convcharB;
            convcharB &= ~(convcharA >> 1);
            ((unsigned short*)charb)[2*i + 1] = convcharB;
        }
    }
    else
    {
        for (unsigned short i = 0; i < 16; i++)
        {
            unsigned long convcharA = charb[i];
            unsigned long convcharB = convcharA >> 1;
            convcharB |= convcharA;
            convcharA ^= convcharB;
            convcharB &= ~(convcharA >> 1);
            charb[i] = convcharB;
        }
    }
}

//Char data must be in 'edit-friendly' format
static void ItaliciseChar(unsigned long* charb, int bits32)
{
    if (!bits32) //Why bother doing work on an empty cell?
    {
        for (unsigned short i = 0; i < 16; i++)
        {
            unsigned long convchar = ((unsigned short*)charb)[2*i + 1];
            convchar >>= 7 - (i >> 1);
            ((unsigned short*)charb)[2*i + 1] = convchar;
        }
    }
    else
    {
        for (unsigned short i = 0; i < 16; i++)
        {
            unsigned long convchar = charb[i];
            convchar >>= 7 - (i >> 1);
            charb[i] = convchar;
        }
    }
}

//Char data must be in 'edit-friendly' format
static void UnderlineChar(unsigned long* charb, short underlineLen)
{
    long underLine = 0x80000000;
    if (underlineLen > 32) underlineLen = 32;
    underlineLen--;
    underLine >>= underlineLen;
    charb[14] |= underLine;
}

//Char data must be in 'edit-friendly' format
static void MaskChar(unsigned long* charb, const unsigned short* chosenMask, int bits32)
{
    if (!bits32) //Why bother doing work on an empty cell?
    {
        for (unsigned short i = 0; i < 16; i++)
        {
            unsigned short convchar = ((unsigned short*)charb)[2*i + 1];
            unsigned short mask = chosenMask[i & 0x3];
            convchar &= mask;
            ((unsigned short*)charb)[2*i + 1] = convchar;
        }
    }
    else
    {
        for (unsigned short i = 0; i < 16; i++)
        {
            unsigned long convchar = charb[i];
            unsigned long mask = chosenMask[i & 0x3];
            convchar &= mask | (mask << 16);
            charb[i] = convchar;
        }
    }
}

//Char data must be in 'VRAM-compatible' format
static void DrawChar(const unsigned long* charb, short x, short y, int bits32)
{
    unsigned short* planeptr = (unsigned short*)(y * 80 + ((x >> 3) & 0xFFFE));
    unsigned short xinblock = x & 0x000F;
    egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD | EGC_BITADDRESS_DEST(xinblock));
    setes(GDC_PLANES_SEGMENT);
    if (!bits32) //Why bother doing work on an empty cell?
    {
        egc_bitlen(16);
        if (xinblock) //Unaligned
        {
            __asm volatile ( //Unrolled a bit to reduce the overhead of jumping
                "lea 0x500(%%di), %%bx\n"
                ".loop%=: movsw\n\t"
                "stosw\n\t"
                "addw $76, %w1\n\t"
                "movsw\n\t"
                "stosw\n\t"
                "addw $76, %w1\n\t"
                "movsw\n\t"
                "stosw\n\t"
                "addw $76, %w1\n\t"
                "movsw\n\t"
                "stosw\n\t"
                "addw $76, %w1\n\t"
                "cmpw %%bx, %w1\n\t"
                "jne .loop%=\n\t"
            : "+S" (charb), "+D" (planeptr) : : "%bx", "memory");
            /*/
            for (unsigned short j = 0; j < 16; j++)
            {
                unsigned short charL = ((unsigned short*)charb)[2*j];
                planeptr[40 * j] = charL;
                planeptr[40 * j + 1] = charL; //Dummy write to empty the EGC's shift buffer
            }
            //*/
        }
        else //Aligned
        {
            __asm volatile ( //Unrolled a bit to reduce the overhead of jumping
                "lea 0x500(%%di), %%bx\n"
                ".loop%=: movsw\n\t"
                "addw $78, %w1\n\t"
                "movsw\n\t"
                "addw $78, %w1\n\t"
                "movsw\n\t"
                "addw $78, %w1\n\t"
                "movsw\n\t"
                "addw $78, %w1\n\t"
                "cmpw %%bx, %w1\n\t"
                "jne .loop%=\n\t"
            : "+S" (charb), "+D" (planeptr) : : "%bx", "memory");
            /*/
            for (unsigned short j = 0; j < 16; j++)
            {
                planeptr[40 * j] = ((unsigned short*)charb)[2*j];
            }
            //*/
        }
    }
    else
    {
        egc_bitlen(32);
        if (xinblock) //Unaligned
        {
            __asm volatile ( //Unrolled a bit to reduce the overhead of jumping
                "lea 0x500(%%di), %%bx\n"
                ".loop%=: movsw\n\t"
                "movsw\n\t"
                "stosw\n\t"
                "addw $74, %w1\n\t"
                "movsw\n\t"
                "movsw\n\t"
                "stosw\n\t"
                "addw $74, %w1\n\t"
                "movsw\n\t"
                "movsw\n\t"
                "stosw\n\t"
                "addw $74, %w1\n\t"
                "movsw\n\t"
                "movsw\n\t"
                "stosw\n\t"
                "addw $74, %w1\n\t"
                "cmpw %%bx, %w1\n\t"
                "jne .loop%=\n\t"
            : "+S" (charb), "+D" (planeptr) : : "%bx", "memory");
            /*/
            for (unsigned short j = 0; j < 16; j++)
            {
                unsigned long charL = charb[j];
                *((unsigned long*)(&planeptr[40 * j])) = charL;
                planeptr[40 * j + 2] = (unsigned short)charL; //Dummy write to empty the EGC's shift buffer
            }
            //*/
        }
        else //Aligned
        {
            __asm volatile ( //Unrolled a bit to reduce the overhead of jumping
                "lea 0x500(%%di), %%bx\n"
                ".loop%=: movsw\n\t"
                "movsw\n\t"
                "addw $76, %w1\n\t"
                "movsw\n\t"
                "movsw\n\t"
                "addw $76, %w1\n\t"
                "movsw\n\t"
                "movsw\n\t"
                "addw $76, %w1\n\t"
                "movsw\n\t"
                "movsw\n\t"
                "addw $76, %w1\n\t"
                "cmpw %%bx, %w1\n\t"
                "jne .loop%=\n\t"
            : "+S" (charb), "+D" (planeptr) : : "%bx", "memory");
            /*/
            for (unsigned short j = 0; j < 16; j++)
            {
                *((unsigned long*)(&planeptr[40 * j])) = charb[j];
            }
            //*/
        }
    }
}

//Char data must be in 'VRAM-compatible' format
static void DrawCharMask(const unsigned long* charb, short x, short y, const unsigned short* chosenMask, int bits32)
{
    unsigned short* planeptr = (unsigned short*)(y * 80 + ((x >> 3) & 0xFFFE));
    unsigned short xinblock = x & 0x000F;
    egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD | EGC_BITADDRESS_DEST(xinblock));
    setes(GDC_PLANES_SEGMENT);
    if (!bits32) //Why bother doing work on an empty cell?
    {
        egc_bitlen(16);
        if (xinblock) //Unaligned
        {
            __asm volatile ( //Unrolled so that we only need to load each mask once
                "xor %%cx, %%cx\n\t"
                "push %%bp\n\t"
                "movw %w2, %%bp\n\t"
                ".loop%=: movw %%bp, %w2\n\t"
                "addw %%cx, %w2\n\t"
                "movw (%w2), %%dx\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "stosw\n\t"
                "addw $6, %0\n\t"
                "addw $316, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "stosw\n\t"
                "addw $6, %0\n\t"
                "addw $316, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "stosw\n\t"
                "addw $6, %0\n\t"
                "addw $316, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "stosw\n\t"
                "subw $24, %0\n\t"
                "subw $884, %1\n\t"
                "addw $2, %%cx\n\t"
                "cmpw $8, %%cx\n\t"
                "jne .loop%=\n\t"
                "pop %%bp"
            : "+S" (charb), "+D" (planeptr) : "b" (chosenMask) : "%ax", "%dx", "%cx", "memory");
            /*/
            for (unsigned short j = 0; j < 16; j++)
            {
                unsigned short mask = chosenMask[j & 0x3];
                unsigned short charL = ((unsigned short*)charb)[2*j];
                charL &= mask;
                planeptr[40 * j] = charL;
                planeptr[40 * j + 1] = charL; //Dummy write to empty the EGC's shift buffer
            }
            //*/
        }
        else //Aligned
        {
            __asm volatile ( //Unrolled so that we only need to load each mask once
                "xor %%cx, %%cx\n\t"
                "push %%bp\n\t"
                "movw %w2, %%bp\n\t"
                ".loop%=: movw %%bp, %w2\n\t"
                "addw %%cx, %w2\n\t"
                "movw (%w2), %%dx\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "addw $6, %0\n\t"
                "addw $318, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "addw $6, %0\n\t"
                "addw $318, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "addw $6, %0\n\t"
                "addw $318, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "subw $24, %0\n\t"
                "subw $882, %1\n\t"
                "addw $2, %%cx\n\t"
                "cmpw $8, %%cx\n\t"
                "jne .loop%=\n\t"
                "pop %%bp"
            : "+S" (charb), "+D" (planeptr) : "b" (chosenMask) : "%ax", "%dx", "%cx", "memory");
            /*/
            for (unsigned short j = 0; j < 16; j++)
            {
                unsigned short mask = chosenMask[j & 0x3];
                unsigned short charL = ((unsigned short*)charb)[2*j];
                planeptr[40 * j] = charL & mask;
            }
            //*/
        }
    }
    else
    {
        egc_bitlen(32);
        if (xinblock) //Unaligned
        {
            __asm volatile ( //Unrolled so that we only need to load each mask once
                "xor %%cx, %%cx\n\t"
                "push %%bp\n\t"
                "movw %w2, %%bp\n\t"
                ".loop%=: movw %%bp, %w2\n\t"
                "addw %%cx, %w2\n\t"
                "movw (%w2), %%dx\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "stosw\n\t"
                "addw $12, %0\n\t"
                "addw $314, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "stosw\n\t"
                "addw $12, %0\n\t"
                "addw $314, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "stosw\n\t"
                "addw $12, %0\n\t"
                "addw $314, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "stosw\n\t"
                "subw $48, %0\n\t"
                "subw $886, %1\n\t"
                "addw $2, %%cx\n\t"
                "cmpw $8, %%cx\n\t"
                "jne .loop%=\n\t"
                "pop %%bp"
            : "+S" (charb), "+D" (planeptr) : "b" (chosenMask) : "%ax", "%dx", "%cx", "memory");
            /*/
            for (unsigned short j = 0; j < 16; j++)
            {
                unsigned long mask = chosenMask[j & 0x3];
                mask |= (mask << 16);
                unsigned long charL = charb[j];
                charL &= mask;
                *((unsigned long*)(&planeptr[40 * j])) = charL;
                planeptr[40 * j + 2] = (unsigned short)charL; //Dummy write to empty the EGC's shift buffer
            }
            //*/
        }
        else //Aligned
        {
            __asm volatile ( //Unrolled so that we only need to load each mask once
                "xor %%cx, %%cx\n\t"
                "push %%bp\n\t"
                "movw %w2, %%bp\n\t"
                ".loop%=: movw %%bp, %w2\n\t"
                "addw %%cx, %w2\n\t"
                "movw (%w2), %%dx\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "addw $12, %0\n\t"
                "addw $316, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "addw $12, %0\n\t"
                "addw $316, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "addw $12, %0\n\t"
                "addw $316, %1\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "lodsw\n\t"
                "andw %%dx, %%ax\n\t"
                "stosw\n\t"
                "subw $48, %0\n\t"
                "subw $884, %1\n\t"
                "addw $2, %%cx\n\t"
                "cmpw $8, %%cx\n\t"
                "jne .loop%=\n\t"
                "pop %%bp"
            : "+S" (charb), "+D" (planeptr) : "b" (chosenMask) : "%ax", "%dx", "%cx", "memory");
            /*/
            for (unsigned short j = 0; j < 16; j++)
            {
                unsigned long mask = chosenMask[j & 0x3];
                mask |= (mask << 16);
                unsigned long charL = charb[j];
                *((unsigned long*)(&planeptr[40 * j])) = charL & mask;
            }
            //*/
        }
    }
}

void SetCustomInfo(unsigned short num, char* str)
{
    customInfos[num] = str;
}

static const char* PreprocessString(const __far char* str, unsigned char autolb, short lx, short rx, short ty, short by)
{
    //Inject variable strings
    char ch = *str++;
    char* pch = stringBuffer1;
    while (ch) //stop when the parent string's null terminator is reached
    {
        if (ch == 0x1B)
        {
            ch = *str++;
            if ((ch & 0xF0) == 0x50)
            {
                char* ich = customInfos[ch & 0x0F];
                ch = *ich++;
                while (ch) //inject entire string (like sprintf would do)
                {
                    *pch++ = ch;
                    ch = *ich++;
                }
            }
            else //escape command is not an information inject -> put literal characters
            {
                *pch++ = 0x1B;
                *pch++ = ch;
            }
        }
        else *pch++ = ch; //no escape -> put literal character
        ch = *str++;
    }
    *pch = 0; //null terminate, as usual
    if (!autolb) return stringBuffer1;
    //Insert automatic line breaks
    pch = stringBuffer2;
    char* sspch = stringBuffer1;
    char* bpp = 0;
    ch = *sspch++;
    short curX = lx;
    short curY = ty;
    while (ch)
    {
        if (ch < 0x21) //control characters
        {
            switch (ch)
            {
                case 0x09: //Tab
                    bpp = pch;
                    short delX = curX - lx + 32;
                    delX &= 0xFFE0; //Tab stops every 4 halfwidth characters
                    curX = lx + delX;
                    break;
                case 0x0A: //LF
                    curY += 16;
                    break;
                case 0x0D: //CR
                    curX = lx;
                    break;
                case 0x20: //Space
                    bpp = pch;
                    curX += 8;
                    break;
            }
        }
        else if (ch <= 0x7F || (((unsigned char)ch) > 0x9F && ((unsigned char)ch) < 0xE0)) //Single byte
        {
            if (ch == '-') bpp = pch + 1;
            curX += 8;
        }
        else //double byte
        {
            unsigned short twobytecode = ch << 8;
            ch = *sspch++;
            if (!ch)
            {
                break; //Standard null termination
            }
            twobytecode |= ch;
            twobytecode = SjisToInternalCode(twobytecode);
            curX += (twobytecode >= 0x0900 && twobytecode < 0x0C00) ? 8 : 16; //The characters in this range are logically halfwidth
        }
        if (curY >= by) //Do not allow text to overflow the box
        {
            break;
        }
        else if (curX >= rx)
        {
            if (!bpp) bpp = pch; //Emergency newline if the break point was not set earlier
            else
            {
                sspch -= pch - bpp;
            }
            *bpp++ = 0x0D; *bpp++ = 0x0A; //Insert newline in the most appropriate position if the string is about to overflow the box
            pch = bpp;
            bpp = 0;
            curX = lx;
            curY += 16;
        }
        else *pch++ = ch;
        ch = *sspch++;
    }
    *pch = 0; //null terminate, as usual
    return stringBuffer2;
}

void StartAnimatedStringToWrite(const __far char* str, const short x, const short y, short format)
{
    const char* pstr = PreprocessString(str, 1, x, x + textBoxInnerBounds.size.x, y, y + textBoxInnerBounds.size.y);
    stringToAnimWrite = pstr;
    curAnimStringPos = pstr;
    currentAnimWriteX = x;
    currentAnimWriteY = y;
    currentAnimNextWriteX = x;
    currentAnimNextWriteY = y;
    currentAnimDefaultFormat = format;
    currentAnimFormat = format;
    chBufStartNum = 0;
    animReachedEndOfString = 0;
    animLength = 16;
    waitFrames = 0;
    waitPerChar = 0;
    memset(animCharBuf, 0, 1024);
    memset(charFade, 0xFFFF, 16);
}

int StringWriteAnimationFrame(unsigned char skip)
{
    if (skip) //If a skip is requested, just write the entire string out with its intended formatting, and return the end of animation signal
    {
        animReachedEndOfString = 1;
        animLength = 0;
        WriteString(stringToAnimWrite, currentAnimWriteX, currentAnimWriteY, currentAnimDefaultFormat, 0);
        return 1;
    }
    unsigned char ch;
    unsigned short twobytecode;
    const char* str = curAnimStringPos;
    short defFormat = currentAnimDefaultFormat;
    short format = currentAnimFormat;
    short x = currentAnimWriteX;
    short y = currentAnimWriteY;
    short curX = currentAnimNextWriteX;
    short curY = currentAnimNextWriteY;
    char nullTerm = animReachedEndOfString;
    unsigned long* nextCharBuf = animCharBuf + 16 * chBufStartNum;
    //Input new characters into the buffer
    if (waitFrames <= 0)
    {
        while (!nullTerm)
        {
            ch = *str++;
            if (ch)
            {
                if (ch < 0x20) //control characters
                {
                    switch (ch)
                    {
                        case 0x09: //Tab
                        {
                            short delX = curX - x + 32;
                            delX &= 0xFFE0; //Tab stops every 4 halfwidth characters
                            curX = x + delX;
                            waitFrames = waitPerChar;
                            break;
                        }
                        case 0x0A: //LF
                            curY += 16;
                            break;
                        case 0x0D: //CR
                            curX = x;
                            waitFrames = waitPerChar;
                            break;
                        case 0x1B: //ESC, used for formatting escape sequences
                            ch = *str++;
                            switch ((ch & 0xF0) >> 4)
                            {
                                case 0x00: //intentional nop
                                    if (ch & 0x0F) break;
                                    else
                                    {
                                        nullTerm = 1; //null terminate
                                        break;
                                    }
                                case 0x01: //set formatting flags, section 0
                                    format = (format & (~FORMAT_PART_MAIN)) | (ch & 0x0F);
                                    break;
                                case 0x02: //set formatting flags, section 1
                                    format = (format & (~FORMAT_PART_UNUSED)) | ((ch & 0x0F) << 4);
                                    break;
                                case 0x03: //set formatting flags, section 2
                                    format = (format & (~FORMAT_PART_FADE)) | ((ch & 0x0F) << 8);
                                    break;
                                case 0x04: //set formatting flags, text colour
                                    format = (format & (~FORMAT_PART_COLOUR)) | ((ch & 0x0F) << 12);
                                    break;
                                case 0x05: //custom protagonist information inject (this is handled by a preprocessor, so do nothing here)
                                    break;
                                case 0x06: //wait for some time before continuing
                                    waitFrames += 10 * ((ch & 0x0F) + 1);
                                    goto LdrawT;
                                case 0x07: //set wait between characters
                                    waitPerChar = ch & 0x0F;
                                    break;
                                case 0x08: //unassigned
                                case 0x09: //unassigned
                                case 0x0A: //unassigned
                                case 0x0B: //unassigned
                                case 0x0C: //unassigned
                                case 0x0D: //unassigned
                                case 0x0E: //unassigned
                                    break;
                                case 0x0F: //reset sections
                                    format = (format & (~FORMAT_PART_MAIN)) | ((ch & 0x1 ? defFormat : format) & FORMAT_PART_MAIN);
                                    format = (format & (~FORMAT_PART_UNUSED)) | ((ch & 0x2 ? defFormat : format) & FORMAT_PART_UNUSED);
                                    format = (format & (~FORMAT_PART_FADE)) | ((ch & 0x4 ? defFormat : format) & FORMAT_PART_FADE);
                                    format = (format & (~FORMAT_PART_COLOUR)) | ((ch & 0x8 ? defFormat : format) & FORMAT_PART_COLOUR);
                                    break;
                            }
                            break;
                    }
                }
                else if (ch <= 0x7F || (ch > 0x9F && ch < 0xE0)) //Single byte
                {
                    twobytecode = ch << 8;
                    if (format & FORMAT_SHADOW) charFlags[chBufStartNum] = 1;
                    else charFlags[chBufStartNum] = 0;
                    GetCharacterDataEditFriendly(twobytecode, nextCharBuf);
                    BoldenCharLeft(nextCharBuf, 0);
                    if (format & FORMAT_ITALIC) ItaliciseChar(nextCharBuf, 0);
                    if (format & FORMAT_BOLD) BoldenCharRight(nextCharBuf, 0);
                    if (format & FORMAT_UNDERLINE) UnderlineChar(nextCharBuf, 8);
                    if (format & FORMAT_PART_FADE) MaskChar(nextCharBuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)], 0);
                    SwapCharDataFormats(nextCharBuf, 0);
                    charColours[chBufStartNum] = FORMAT_COLOUR_GET(format);
                    charXs[chBufStartNum] = curX;
                    charYs[chBufStartNum] = curY;
                    charFade[chBufStartNum] = 0;
                    curX += 8;
                    waitFrames = waitPerChar;
                    break;
                }
                else //double byte
                {
                    twobytecode = ch << 8;
                    ch = *str++;
                    if (!ch)
                    {
                        nullTerm = 1;
                        break; //Standard null termination
                    }
                    twobytecode |= ch;
                    twobytecode = SjisToInternalCode(twobytecode);
                    short actualWidth = (twobytecode >= 0x0900 && twobytecode < 0x0C00) ? 8 : 16; //The characters in this range are logically halfwidth
                    if (format & FORMAT_SHADOW) charFlags[chBufStartNum] = 1;
                    else charFlags[chBufStartNum] = 0;
                    GetCharacterDataEditFriendly(twobytecode, nextCharBuf);
                    BoldenCharLeft(nextCharBuf, 0);
                    char is32Pixels = 0;
                    if (format & FORMAT_ITALIC) { ItaliciseChar(nextCharBuf, 1); is32Pixels = 0x02; }
                    if (format & FORMAT_BOLD) { BoldenCharRight(nextCharBuf, 1); is32Pixels = 0x02; }
                    if (format & FORMAT_UNDERLINE) UnderlineChar(nextCharBuf, actualWidth);
                    if (format & FORMAT_PART_FADE) MaskChar(nextCharBuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)], is32Pixels);
                    SwapCharDataFormats(nextCharBuf, is32Pixels);
                    charFlags[chBufStartNum] |= is32Pixels;
                    charColours[chBufStartNum] = FORMAT_COLOUR_GET(format);
                    charXs[chBufStartNum] = curX;
                    charYs[chBufStartNum] = curY;
                    charFade[chBufStartNum] = 0;
                    curX += actualWidth;
                    waitFrames = waitPerChar;
                    break;
                }
            }
            else
            {
                nullTerm = 1;
                break;
            }
        }
    }
    else
    {
        waitFrames--;
    }
    LdrawT:
    curAnimStringPos = str;
    currentAnimFormat = format;
    currentAnimNextWriteX = curX;
    currentAnimNextWriteY = curY;
    animReachedEndOfString = nullTerm;
    if (nullTerm)
    {
        chBufStartNum--;
        chBufStartNum &= 0xF;
        animLength--;
    }
    for (unsigned short i = 0; i < animLength; i++)
    {
        unsigned short chBufNum = (chBufStartNum - i) & 0xF;
        unsigned short fadeStart = charFade[chBufNum] * 4;
        if (fadeStart < 64)
        {
            charFade[chBufNum]++;
            const unsigned short curcol = charColours[chBufNum];
            if (charFlags[chBufNum] & 0x01)
            {
                egc_fgcolour(shadowColours[curcol]);
                DrawCharMask(&animCharBuf[16 * chBufNum], charXs[chBufNum] + 1, charYs[chBufNum] + 1, bayer4x4masks + fadeStart, charFlags[chBufNum] & 0x02);
            }
            egc_fgcolour(curcol);
            DrawCharMask(&animCharBuf[16 * chBufNum], charXs[chBufNum], charYs[chBufNum], bayer4x4masks + fadeStart, charFlags[chBufNum] & 0x02);
        }
    }
    chBufStartNum++;
    chBufStartNum &= 0xF;
    if (animLength <= 0) return 1;
    else return 0;
}

void WriteStringInternal(const char* str, const short x, const short y, short format)
{
    unsigned char ch = 0xFF;
    short twobytecode;
    short curX = x;
    short curY = y;
    const short defFormat = format;
    /*
        CCCC FFFF 000T SUIB
        B -> bold flag
        I -> italic flag
        U -> underline flag
        S -> shadow flag
        T -> halfwidth font (0 -> JIS-X-0201 default, 1 -> NEC specific)
        F -> fade value (0 -> no fade, F -> almost complete fade)
        C -> text colour (shadow colour is automatically picked)
    */
    while (1)
    {
        ch = *str++;
        if (ch)
        {
            if (ch <= 0x20) //control characters
            {
                switch (ch)
                {
                    case 0x09: //Tab
                    {
                        short delX = curX - x + 32;
                        delX &= 0xFFE0; //Tab stops every 4 halfwidth characters
                        curX = x + delX;
                        break;
                    }
                    case 0x0A: //LF
                        curY += 16;
                        break;
                    case 0x0D: //CR
                        curX = x;
                        break;
                    case 0x1B: //ESC, used for formatting escape sequences
                        ch = *str++;
                        switch ((ch & 0xF0) >> 4)
                        {
                            case 0x00: //intentional nop
                                if (ch & 0x0F) break;
                                else return; //null terminate
                            case 0x01: //set formatting flags, section 0
                                format = (format & (~FORMAT_PART_MAIN)) | (ch & 0x0F);
                                break;
                            case 0x02: //set formatting flags, section 1
                                format = (format & (~FORMAT_PART_UNUSED)) | ((ch & 0x0F) << 4);
                                break;
                            case 0x03: //set formatting flags, section 2
                                format = (format & (~FORMAT_PART_FADE)) | ((ch & 0x0F) << 8);
                                break;
                            case 0x04: //set formatting flags, text colour
                                format = (format & (~FORMAT_PART_COLOUR)) | ((ch & 0x0F) << 12);
                                break;
                            case 0x05: //custom protagonist information inject (this is handled by a preprocessor, so do nothing here)
                            case 0x06: //wait for some time before continuing (pointless here, so do nothing)
                            case 0x07: //set wait between characters (pointless here, so do nothing)
                            case 0x08: //unassigned
                            case 0x09: //unassigned
                            case 0x0A: //unassigned
                            case 0x0B: //unassigned
                            case 0x0C: //unassigned
                            case 0x0D: //unassigned
                            case 0x0E: //unassigned
                                break;
                            case 0x0F: //reset sections
                                format = (format & (~FORMAT_PART_MAIN)) | ((ch & 0x1 ? defFormat : format) & FORMAT_PART_MAIN);
                                format = (format & (~FORMAT_PART_UNUSED)) | ((ch & 0x2 ? defFormat : format) & FORMAT_PART_UNUSED);
                                format = (format & (~FORMAT_PART_FADE)) | ((ch & 0x4 ? defFormat : format) & FORMAT_PART_FADE);
                                format = (format & (~FORMAT_PART_COLOUR)) | ((ch & 0x8 ? defFormat : format) & FORMAT_PART_COLOUR);
                                break;
                        }
                        break;
                    case 0x20: //space
                        memset(charbuf, 0, 64);
                        if (format & FORMAT_UNDERLINE)
                        {
                            UnderlineChar(charbuf, 8);
                            if (format & FORMAT_PART_FADE) MaskChar(charbuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)], 0);
                            SwapCharDataFormats(charbuf, 0);
                            if (format & FORMAT_SHADOW)
                            {
                                egc_fgcolour(shadowColours[FORMAT_COLOUR_GET(format)]);
                                DrawChar(charbuf, curX + 1, curY + 1, 0);
                            }
                            egc_fgcolour(FORMAT_COLOUR_GET(format));
                            DrawChar(charbuf, curX, curY, 0);
                        }
                        curX += 8;
                        break;
                }
            }
            else if (ch <= 0x7F || (ch > 0x9F && ch < 0xE0)) //Single byte
            {
                twobytecode = ch << 8;
                GetCharacterDataEditFriendly(twobytecode, charbuf);
                BoldenCharLeft(charbuf, 0);
                if (format & FORMAT_ITALIC) ItaliciseChar(charbuf, 0);
                if (format & FORMAT_BOLD) BoldenCharRight(charbuf, 0);
                if (format & FORMAT_UNDERLINE) UnderlineChar(charbuf, 8);
                if (format & FORMAT_PART_FADE) MaskChar(charbuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)], 0);
                SwapCharDataFormats(charbuf, 0);
                if (format & FORMAT_SHADOW)
                {
                    egc_fgcolour(shadowColours[FORMAT_COLOUR_GET(format)]);
                    DrawChar(charbuf, curX + 1, curY + 1, 0);
                }
                egc_fgcolour(FORMAT_COLOUR_GET(format));
                DrawChar(charbuf, curX, curY, 0);
                curX += 8;
            }
            else //double byte
            {
                twobytecode = ch << 8;
                ch = *str++;
                if (!ch) return; //Standard null termination
                twobytecode |= ch;
                twobytecode = SjisToInternalCode(twobytecode);
                short actualWidth = (twobytecode >= 0x0900 && twobytecode < 0x0C00) ? 8 : 16; //The characters in this range are logically halfwidth
                GetCharacterDataEditFriendly(twobytecode, charbuf);
                BoldenCharLeft(charbuf, 0);
                char is32Pixels = 0;
                if (format & FORMAT_ITALIC) { ItaliciseChar(charbuf, 1); is32Pixels = 1; }
                if (format & FORMAT_BOLD) { BoldenCharRight(charbuf, 1); is32Pixels = 1; }
                if (format & FORMAT_UNDERLINE) UnderlineChar(charbuf, actualWidth);
                if (format & FORMAT_PART_FADE) MaskChar(charbuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)], is32Pixels);
                SwapCharDataFormats(charbuf, is32Pixels);
                if (format & FORMAT_SHADOW)
                {
                    egc_fgcolour(shadowColours[FORMAT_COLOUR_GET(format)]);
                    DrawChar(charbuf, curX + 1, curY + 1, is32Pixels);
                }
                egc_fgcolour(FORMAT_COLOUR_GET(format));
                DrawChar(charbuf, curX, curY, is32Pixels);
                curX += actualWidth;
            }
        }
        else break; //Standard null termination
    }
}

void WriteString(const __far char* str, const short x, const short y, short format, unsigned char autolb)
{
    const char* pstr = PreprocessString(str, autolb, x, x + textBoxInnerBounds.size.x, y, y + textBoxInnerBounds.size.y);
    WriteStringInternal(pstr, x, y, format);
}
