/* MHVNIMGP - Image file packer for the MHVN Visual Novel Engine
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
 * Main image packer code
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include "imagepacker.h"

#define EOT ((char)0x04)

typedef struct
{
    unsigned char* imageData;
    unsigned char* realData;
    int totalDataSize;
    int planeMask;
    char* name;
    int xpos;
    int ypos;
} VariantInfo;

typedef struct
{
    unsigned char* imageData;
    unsigned char* realData;
    int totalDataSize;
    int planeMask;
    char* name;
    int palnum;
    std::vector<VariantInfo>* variants;
} ImageInfo;

typedef struct
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
} ColourRGB;

typedef struct
{
    ColourRGB cols[16];
} Palette;

std::vector<ImageInfo>* images;
std::vector<Palette>* palettes;
char inputFileRoot[512];

#define PARSEMODE_FINDNAME     0
#define PARSEMODE_FINDFILENAME 1

int ParseInputLine(char** line, int mode)
{
    char* linePtr = *line;
    char* wordPtr = linePtr;
    char ch = *linePtr++;
    int parseMode = PARSEMODE_FINDNAME;
    char imageFilename[512];
    ImageInfo* curInf;
    while (ch)
    {
        switch (ch)
        {
            case EOT:
                linePtr--;
                goto parseEnd;
            case ',':
            {
                linePtr[-1] = 0x00;
                ImageInfo iinf;
                iinf.name = wordPtr;
                iinf.palnum = -1;
                iinf.variants = new std::vector<VariantInfo>();
                images->push_back(iinf);
                curInf = &((*images)[images->size()-1]);
                wordPtr = linePtr;
                parseMode = PARSEMODE_FINDFILENAME;
            }
                break;
            case '\r':
            case '\n':
                switch (parseMode)
                {
                    case PARSEMODE_FINDNAME:
                        break;
                    case PARSEMODE_FINDFILENAME:
                    {
                        linePtr[-1] = 0x00;
                        memcpy(imageFilename, inputFileRoot, 512);
                        strcat(imageFilename, wordPtr);
                        FILE* gpiFileHandle = fopen(imageFilename, "rb");
                        if (gpiFileHandle == NULL)
                        {
                            printf("ERROR - Image file %s could not be opened!\n", imageFilename);
                            images->erase(images->cend());
                            break;
                        }
                        fseek(gpiFileHandle, 0, SEEK_END); //Hopefully should set the file pointer to the end of the file
                        const long fileLen = ftell(gpiFileHandle) + 1; //So we can get the file's length!
                        fseek(gpiFileHandle, 0, SEEK_SET);
                        curInf->imageData = (unsigned char*)malloc(fileLen);
                        fread(curInf->imageData, 1, fileLen, gpiFileHandle);
                        fclose(gpiFileHandle);

                        unsigned char* magicNum = curInf->imageData;
                        if (magicNum[0] != 'G' || magicNum[1] != 'P' || magicNum[2] != 'I')
                        {
                            printf("ERROR - Image file %s is not a GPI file!\n", imageFilename);
                            free(curInf->imageData);
                            images->erase(images->cend());
                            break;
                        }
                        int pMask = *((uint16_t*)(curInf->imageData + 0xA));
                        if (pMask & 0x00F0)
                        {
                            printf("ERROR - Image file %s has planes that MHVN98 cannot display! Only planes 0-3 and the mask plane are valid!\n", imageFilename);
                            free(curInf->imageData);
                            images->erase(images->cend());
                            break;
                        }
                        pMask |= ((int)(curInf->imageData[3] & 0x08)) << 13; //cheeky, relies on int being at least 32 bits wide
                        curInf->planeMask = pMask;
                        int numColours = 1;
                        int checkbit = 0x01;
                        for (int i = 0; i < 4; i++)
                        {
                            if (pMask & checkbit)
                            {
                                numColours <<= 1;
                            }
                            checkbit <<= 1;
                        }
                        int palSize = numColours * 3;
                        if (!(pMask & 0x00010000)) palSize /= 2;
                        curInf->realData = curInf->imageData + 0xE + palSize;
                        curInf->totalDataSize = fileLen - 0xE - palSize;
                        if (mode == PACK_MODE_BG)
                        {
                            if (pMask & 0x0100)
                            {
                                printf("WARNING - Image file %s has a mask plane but is intended for background use. Please remove the mask plane.\n", imageFilename);
                            }
                            Palette tpal;
                            ColourRGB* thisPalette = tpal.cols;
                            unsigned char* palPtr = curInf->imageData + 0xE;
                            if (pMask & 0x00010000)
                            {
                                for (int i = 0; i < numColours; i++)
                                {
                                    thisPalette[i].R = *palPtr++;
                                    thisPalette[i].G = *palPtr++;
                                    thisPalette[i].B = *palPtr++;
                                }
                            }
                            else
                            {
                                for (int i = 0; i < numColours; i++)
                                {
                                    unsigned char comp = *palPtr++;
                                    thisPalette[i].R = (comp & 0x0F) * 0x11;
                                    thisPalette[i].G = ((comp & 0xF0) >> 4) * 0x11;
                                    comp = *palPtr++;
                                    thisPalette[i].B = (comp & 0x0F) * 0x11;
                                    i++;
                                    thisPalette[i].R = ((comp & 0xF0) >> 4) * 0x11;
                                    comp = *palPtr++;
                                    thisPalette[i].G = (comp & 0x0F) * 0x11;
                                    thisPalette[i].B = ((comp & 0xF0) >> 4) * 0x11;
                                }
                            }
                            //Relocate colours in the palette
                            switch (pMask & 0x0F)
                            {
                                case 0x0:
                                case 0x1:
                                case 0x3:
                                case 0x7:
                                case 0xF:
                                    break; //No relocation necessary
                                case 0x2:
                                case 0x6:
                                case 0xE:
                                    for (int i = numColours - 1; i > 0; i--)
                                    {
                                        thisPalette[i * 2] = thisPalette[i];
                                    }
                                    break;
                                case 0x4:
                                case 0xC:
                                    for (int i = numColours - 1; i > 0; i--)
                                    {
                                        thisPalette[i * 4] = thisPalette[i];
                                    }
                                    break;
                                case 0x8:
                                    for (int i = numColours - 1; i > 0; i--)
                                    {
                                        thisPalette[i * 8] = thisPalette[i];
                                    }
                                    break;
                                case 0x5:
                                case 0xD:
                                    for (int i = (numColours/2) - 1; i > 0; i--)
                                    {
                                        thisPalette[i * 4] = thisPalette[i * 2];
                                        thisPalette[i * 4 + 1] = thisPalette[i * 2 + 1];
                                    }
                                    break;
                                case 0xA:
                                    for (int i = (numColours/2) - 1; i >= 0; i--)
                                    {
                                        thisPalette[i * 8] = thisPalette[i * 2];
                                        thisPalette[i * 8 + 2] = thisPalette[i * 2 + 1];
                                    }
                                    break;
                                case 0x9:
                                    for (int i = (numColours/2) - 1; i > 0; i--)
                                    {
                                        thisPalette[i * 8] = thisPalette[i * 2];
                                        thisPalette[i * 8 + 1] = thisPalette[i * 2 + 1];
                                    }
                                    break;
                                case 0xB:
                                    for (int i = (numColours/4) - 1; i > 0; i--)
                                    {
                                        thisPalette[i * 8] = thisPalette[i * 4];
                                        thisPalette[i * 8 + 1] = thisPalette[i * 4 + 1];
                                        thisPalette[i * 8 + 2] = thisPalette[i * 4 + 2];
                                        thisPalette[i * 8 + 3] = thisPalette[i * 4 + 3];
                                    }
                                    break;
                            }
                            //Find a palette or add it to the list of palettes
                            int chosenPalNum = -1;
                            for (int i = 0; i < palettes->size(); i++)
                            {
                                bool paletteOk = true;
                                for (int j = 0; j < 16; j++)
                                {
                                    if ((j & pMask) & 0xF)
                                    {
                                        ColourRGB colL = thisPalette[j];
                                        ColourRGB colR = (*palettes)[i].cols[j];
                                        if (colL.R != colR.R || colL.G != colR.G || colL.B != colR.B)
                                        {
                                            paletteOk = false;
                                            break;
                                        }
                                    }
                                }
                                if (paletteOk)
                                {
                                    chosenPalNum = i;
                                    break;
                                }
                            }
                            if (chosenPalNum < 0)
                            {
                                palettes->push_back(tpal);
                                curInf->palnum = palettes->size() - 1;
                            }
                            else curInf->palnum = chosenPalNum;
                        }
                    }
                        break;
                }
                goto parseEnd;
            case 0x20: //space
                wordPtr = linePtr;
                break;
        }
        ch = *linePtr++;
    }
    parseEnd:
    *line = linePtr;
    return 0;
}

int PackImages(char* inputFilename, char* outputFilename, int mode)
{
    char* inputFileContents;
    FILE* inputFileHandle = fopen(inputFilename, "r");
    if (inputFileHandle == NULL)
    {
        printf("CRITICAL ERROR - Input file %s could not be opened!\n", inputFilename);
        return 1;
    }

    fseek(inputFileHandle, 0, SEEK_END); //Hopefully should set the file pointer to the end of the file
    const long fileLen = ftell(inputFileHandle) + 1; //So we can get the file's length!
    fseek(inputFileHandle, 0, SEEK_SET);
    inputFileContents = (char*)malloc(fileLen);
    inputFileContents[fileLen - 1] = EOT; //Put EOT character in buffer so that the parser knows when the file has ended
    fread(inputFileContents, 1, fileLen, inputFileHandle);
    fclose(inputFileHandle);

    //Isolate the relative directory root of the input file
    strcpy(inputFileRoot, inputFilename);
    char* rootPtr = inputFileRoot;
    char* rootEndPtr = inputFileRoot;
    char ch = *rootPtr++;
    while (ch)
    {
        if (ch == '/' || ch == '\\')
        {
            rootEndPtr = rootPtr;
        }
        ch = *rootPtr++;
    }
    *rootEndPtr = 0x00;

    //Open output file just to make sure it can be done beforehand
    FILE* outputFileHandle = fopen(outputFilename, "wb");
    if (outputFileHandle == NULL)
    {
        printf("CRITICAL ERROR - Output file %s could not be opened!\n", outputFilename);
        free(inputFileContents);
        return 1;
    }

    //Get all the required information
    char* cptr = inputFileContents;
    images = new std::vector<ImageInfo>();
    palettes = new std::vector<Palette>();
    while (*cptr != EOT)
    {
        int result = ParseInputLine(&cptr, mode);
    }

    //Write the main data
    unsigned char* tableData;
    int tableSize;
    if (mode == PACK_MODE_BG)
    {
        tableSize = 8 + 24 * palettes->size() + 12 * images->size();
    }
    else if (mode == PACK_MODE_SPR)
    {
        tableSize = 2 + 10 * images->size();
    }
    for (int i = 0; i < images->size(); i++)
    {
        tableSize += 8 * (*images)[i].variants->size();
    }
    tableData = (unsigned char*)malloc(tableSize);
    unsigned char* tPtr = tableData;

    int imageEntryBaseSize;
    if (mode == PACK_MODE_BG)
    {
        *((uint32_t*)(tPtr))       = 8 + 24 * palettes->size();
        *((uint16_t*)(tPtr + 0x4)) = images->size();
        *((uint16_t*)(tPtr + 0x6)) = palettes->size();
        tPtr += 0x8;
        for (int i = 0; i < palettes->size(); i++)
        {
            Palette pal = (*palettes)[i];
            for (int j = 0; j < 16; j++)
            {
                unsigned int oc = pal.cols[j].R;
                oc += 0x08; oc /= 0x11;
                unsigned char comp = oc;
                oc = pal.cols[j].G;
                oc += 0x08; oc /= 0x11;
                comp |= oc << 4;
                *tPtr++ = comp;
                oc = pal.cols[j].B;
                oc += 0x08; oc /= 0x11;
                comp = oc;
                j++;
                oc = pal.cols[j].R;
                oc += 0x08; oc /= 0x11;
                comp |= oc << 4;
                *tPtr++ = comp;
                oc = pal.cols[j].G;
                oc += 0x08; oc /= 0x11;
                comp = oc;
                oc = pal.cols[j].B;
                oc += 0x08; oc /= 0x11;
                comp |= oc << 4;
                *tPtr++ = comp;
            }
        }
        imageEntryBaseSize = 8;
    }
    else if (mode == PACK_MODE_SPR)
    {
        *((uint16_t*)(tPtr)) = images->size();
        tPtr += 0x2;
        imageEntryBaseSize = 6;
    }
    uint32_t* imgPtrPtr = (uint32_t*)tPtr;
    uint32_t imgPtr = 0;
    for (int i = 0; i < images->size(); i++)
    {
        imgPtrPtr[i] = imgPtr;
        imgPtr += imageEntryBaseSize + 8 * (*images)[i].variants->size();
    }
    tPtr += 4 * images->size();
    imgPtr += (uint32_t)(tPtr - tableData);
    for (int i = 0; i < images->size(); i++)
    {
        ImageInfo curImg = (*images)[i];
        *((uint32_t*)(tPtr)) = imgPtr;
        imgPtr += 0xE + curImg.totalDataSize;
        *((uint16_t*)(tPtr + 0x4)) = curImg.variants->size();
        if (mode == PACK_MODE_BG)
        {
            *((uint16_t*)(tPtr + 0x6)) = curImg.palnum;
            tPtr += 0x8;
        }
        else if (mode == PACK_MODE_SPR)
        {
            tPtr += 0x6;
        }
        for (int j = 0; j < curImg.variants->size(); j++)
        {
            VariantInfo curVar = (*(curImg.variants))[j];
            *((uint32_t*)(tPtr)) = imgPtr;
            imgPtr += 0xE + curVar.totalDataSize;
            *((uint16_t*)(tPtr + 0x4)) = curVar.xpos;
            *((uint16_t*)(tPtr + 0x6)) = curVar.ypos;
            tPtr += 0x8;
        }
    }
    uint64_t linkPtr = imgPtr + 8;
    fwrite(&linkPtr, sizeof(uint64_t), 1, outputFileHandle);
    fwrite(tableData, 1, tableSize, outputFileHandle);
    for (int i = 0; i < images->size(); i++)
    {
        ImageInfo curImg = (*images)[i];
        fwrite(curImg.imageData, 1, 0xE, outputFileHandle);
        fwrite(curImg.realData, 1, curImg.totalDataSize, outputFileHandle);
        for (int j = 0; j < curImg.variants->size(); j++)
        {
            VariantInfo curVar = (*(curImg.variants))[j];
            fwrite(curVar.imageData, 1, 0xE, outputFileHandle);
            fwrite(curVar.realData, 1, curVar.totalDataSize, outputFileHandle);
        }
    }

    //Write link info
    uint16_t nImage = images->size();
    fwrite(&nImage, sizeof(uint16_t), 1, outputFileHandle);
    std::vector<unsigned char>* linkInf = new std::vector<unsigned char>(sizeof(uint64_t) * images->size());
    linkInf->reserve(0x00010000);
    uint64_t* imageLinkInfoPtrs = (uint64_t*)linkInf->data();
    uint64_t imageLinkPtr = 0;
    for (int i = 0; i < images->size(); i++)
    {
        char* linkNamePtr = (*images)[i].name;
        char ch = *linkNamePtr++;
        while (ch)
        {
            linkInf->push_back(ch);
            ch = *linkNamePtr++;
        }
        linkInf->push_back(0);
    }
    for (int i = 0; i < images->size(); i++)
    {
        ImageInfo curImg = (*images)[i];
        uint16_t nVar = curImg.variants->size();
        imageLinkInfoPtrs[i] = imageLinkPtr;
        linkInf->push_back((unsigned char)(nVar & 0xFF));
        linkInf->push_back((unsigned char)((nVar >> 8) & 0xFF));
        imageLinkPtr += 2;
        for (int j = 0; j < curImg.variants->size(); j++)
        {
            char* linkNamePtr = (*curImg.variants)[i].name;
            char ch = *linkNamePtr++;
            uint64_t linkNameLen = 0;
            while (ch)
            {
                linkInf->push_back(ch);
                ch = *linkNamePtr++;
                linkNameLen++;
            }
            linkInf->push_back(0);
            imageLinkPtr += linkNameLen + 1;
        }
    }
    fwrite(linkInf->data(), 1, linkInf->size(), outputFileHandle);

    //Clean up
    fclose(outputFileHandle);
    free(tableData);
    for (int i = 0; i < images->size(); i++)
    {
        for (int j = 0; j < (*images)[i].variants->size(); j++)
        {
            free((*((*images)[i].variants))[j].imageData);
        }
        free((*images)[i].imageData);
    }

    delete palettes;
    delete images;
    free(inputFileContents);
    return 0;
}
