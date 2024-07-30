/* MHVNLINK - Data archive linker for the MHVN Visual Novel Engine
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
 * Main linker code
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unordered_map>
#include <string>

#define GLOBAL_VAR_BASE  0x0020
#define GLOBAL_FLAG_BASE 0x0100
#define LOCAL_VAR_BASE   0x0400
#define LOCAL_FLAG_BASE  0x0600

typedef struct
{
    int numTextsInScene;
    std::unordered_map<std::string, int>* sceneDataTextsMap;
} SceneData;

typedef struct
{
    int numVariantsInImage;
    std::unordered_map<std::string, int>* imageDataVariantsMap;
} ImageData;

SceneData scenes[65536];
ImageData bgs[65536];
ImageData sprs[65536];
std::unordered_map<std::string, int>* sceneDataSceneNamesMap;
std::unordered_map<std::string, int>* sceneDataCharNamesMap;
std::unordered_map<std::string, int>* sceneDataVarNamesMap;
std::unordered_map<std::string, int>* sceneDataBgNamesMap;
std::unordered_map<std::string, int>* sceneDataSprNamesMap;

int numChars;
int numScenes;
int numGlobVars;
int numGlobFlags;
int numLocalVars;
int numLocalFlags;
int numBg;
int numSpr;

uint16_t Ptr16TempArray[65536];
uint32_t Ptr32TempArray[65536];
uint64_t Ptr64TempArray[65536];
int sceneIndexArray[65536];
int bgIndexArray[65536];
int sprIndexArray[66536];

unsigned char* sceneObjectData;

const char* GetFilenameNoDir(const char* fn)
{
    const char* chptr = fn;
    const char* retptr = fn;
    char ch = *chptr++;
    while(ch) //Isolate file name from directory
    {
        if (ch == '/' || ch == '\\')
        {
            retptr = chptr;
        }
        ch = *chptr++;
    }
    return retptr;
}

int MakeOutputFilename(const char* src, char* dst, const char* ext)
{
    int len = 0;
    for (int i = 0; i < 8; i++)
    {
        const char ch = src[i];
        if (ch == '.' || !ch)
        {
            break;
        }
        *dst++ = ch;
        len++;
    }
    if (ext != NULL)
    {
        *dst++ = '.';
        len++;
        for (int i = 0; i < 3; i++)
        {
            const char ch = ext[i];
            if (!ch)
            {
                break;
            }
            *dst++ = ch;
            len++;
        }
    }
    *dst = 0;
    return len;
}

long GetFileLength(FILE* f)
{
    fseek(f, 0, SEEK_END); //Hopefully should set the file pointer to the end of the file
    long fileLen = ftell(f) + 1; //So we can get the file's length!
    fseek(f, 0, SEEK_SET);
    return fileLen;
}

int LinkTextData(const char* tDataFilename, char* outputFilename)
{
    //Read in text data object
    FILE* curTextObjectFilePtr = fopen(tDataFilename, "rb");
    if (curTextObjectFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Text data object file %s could not be opened!\n", tDataFilename);
        return 1;
    }
    const long curTextObjectFileLen = GetFileLength(curTextObjectFilePtr);
    unsigned char* textObjectData = (unsigned char*)malloc(curTextObjectFileLen);
    fread(textObjectData, 1, curTextObjectFileLen, curTextObjectFilePtr);
    fclose(curTextObjectFilePtr);
    uint64_t textLinkInfoPtr = *((uint64_t*)(textObjectData));
    unsigned char* textObjectLinkInfoPtr = textObjectData + textLinkInfoPtr;

    //Link text data based on scene data
    int numCharsInText = *((uint16_t*)textObjectLinkInfoPtr);
    int numScenesInText = *((uint16_t*)(&textObjectLinkInfoPtr[0x2]));
    uint16_t* charNamePtrs = (uint16_t*)(&textObjectData[8 + *((uint32_t*)(&textObjectData[8 + 0x8]))]);
    uint32_t* sceneTextPtrs = (uint32_t*)(&textObjectData[8 + *((uint32_t*)(&textObjectData[8 + 0xC]))]);
    textObjectLinkInfoPtr += 4;
    if (numScenesInText < numScenes)
    {
        printf("CRITICAL ERROR - Some scenes have not been given text in %s!\n", tDataFilename);
        return 1;
    }

    //Fix character name pointers
    if (numCharsInText < numChars)
    {
        printf("CRITICAL ERROR - Some characters have not been given names in %s!\n", tDataFilename);
        return 1;
    }
    for (int i = 0; i < numChars; i++)
    {
        uint16_t cptr = charNamePtrs[i];
        std::string str = std::string((char*)textObjectLinkInfoPtr);
        int newIndex = (*sceneDataCharNamesMap)[str];
        Ptr16TempArray[newIndex] = cptr;
        textObjectLinkInfoPtr += str.size()+ 1;
    }
    memcpy(charNamePtrs, Ptr16TempArray, sizeof(uint16_t) * numChars);
    if (numCharsInText > numChars)
    {
        printf("WARNING - Some names in %s correspond to unused characters!\n", tDataFilename);
        for (int i = numChars; i < numCharsInText; i++) //Skip over unused character names
        {
            while (1)
            {
                if (*textObjectLinkInfoPtr++ == 0)
                {
                    break;
                }
            }
        }
    }

    //Fix scene data pointers
    uint64_t* sceneTextLinkPtrs = (uint64_t*)textObjectLinkInfoPtr;
    textObjectLinkInfoPtr += 8 * numScenesInText;
    for (int i = 0; i < numScenes; i++)
    {
        uint32_t sptr = sceneTextPtrs[i];
        std::string str = std::string((char*)textObjectLinkInfoPtr);
        int newIndex = (*sceneDataSceneNamesMap)[str];
        sceneIndexArray[i] = newIndex;
        Ptr32TempArray[newIndex] = sptr;
        textObjectLinkInfoPtr += str.size()+ 1;
    }
    memcpy(sceneTextPtrs, Ptr32TempArray, sizeof(uint32_t) * numScenes);
    if (numScenesInText > numScenes)
    {
        printf("WARNING - Some scenes in %s correspond to unused scenes!\n", tDataFilename);
        for (int i = numScenes; i < numScenesInText; i++) //Skip over unused scenes
        {
            while (1)
            {
                if (*textObjectLinkInfoPtr++ == 0)
                {
                    break;
                }
            }
        }
    }
    unsigned char* sceneTextsLinkInfoBasePtr = textObjectLinkInfoPtr;

    //Fix scene texts
    for (int i = 0; i < numScenes; i++)
    {
        textObjectLinkInfoPtr = sceneTextsLinkInfoBasePtr + sceneTextLinkPtrs[i];
        int numTextsInSceneInText = *((uint16_t*)textObjectLinkInfoPtr);
        textObjectLinkInfoPtr += 2;
        int realIndex = sceneIndexArray[i];
        int numTextsInScene = scenes[realIndex].numTextsInScene;
        if (numTextsInSceneInText < numTextsInScene)
        {
            printf("CRITICAL ERROR - Some texts in scene %u in %s have not been written!\n", i, tDataFilename);
            return 1;
        }
        std::unordered_map<std::string, int>* textsMap = scenes[realIndex].sceneDataTextsMap;
        uint16_t* curSceneTextPtrs = ((uint16_t*)(((unsigned char*)(sceneTextPtrs + numScenesInText)) + sceneTextPtrs[i])) + 1;
        for (int j = 0; j < numTextsInScene; j++)
        {
            uint16_t tptr = curSceneTextPtrs[j];
            std::string str = std::string((char*)textObjectLinkInfoPtr);
            int newIndex = (*textsMap)[str];
            Ptr16TempArray[newIndex] = tptr;
            textObjectLinkInfoPtr += str.size()+ 1;
        }
        memcpy(curSceneTextPtrs, Ptr16TempArray, sizeof(uint16_t) * numTextsInScene);
        if (numTextsInSceneInText > numTextsInScene)
        {
            printf("WARNING - Some texts in scene %u in %s correspond to unused texts!\n", i, tDataFilename);
        }
    }


    //Write out linked text data
    FILE* textDataOutputFilePtr = fopen(outputFilename, "wb");
    if (textDataOutputFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Text data output file %s could not be opened!\n", outputFilename);
        free(textObjectData);
        return 1;
    }
    fwrite(textObjectData + 8, 1, textLinkInfoPtr - 8, textDataOutputFilePtr);
    fclose(textDataOutputFilePtr);

    free(textObjectData);
    return 0;
}

int LinkVN(const char* dir, const char* masterDesc, const char* fontData, const char* sceneData, const char** textData, int numTextDatas, const char* bgData, const char* spriteData, const char* musicData, const char* sfxData, const char* sysData)
{
    //Read in master descriptor
    FILE* masterDescFilePtr = fopen(masterDesc, "rb");
    if (masterDescFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Master descriptor file %s could not be opened!\n", masterDesc);
        return 1;
    }
    const long masterDescFileLen = GetFileLength(masterDescFilePtr);
    unsigned char* masterDescData = (unsigned char*)malloc(masterDescFileLen+1);
    fread(masterDescData, 1, masterDescFileLen, masterDescFilePtr);
    fclose(masterDescFilePtr);
    masterDescData[masterDescFileLen] = 0; //Add null terminator just in case
    int VNFlags = 0;
    int normFormat = 0xF000;
    int charFormat = 0xF000;
    int menuNotSelFormat = 0xF000;
    int menuSelFormat = 0x0000;
    char* pch = (char*)masterDescData;
    char* cmpstr = pch;
    char ch = *pch++;
    int selectedOption = -1;
    bool equalsFound = false;
    bool checkingOption = false;
    int* selFormat;
    while (ch)
    {
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') //Whitespace and new line
        {
            if (checkingOption)
            {
                *(pch-1) = 0;
                if (equalsFound)
                {
                    switch (selectedOption)
                    {
                        case 0: //a default text format
                            if (!strcmp(cmpstr, "bold")) *selFormat |= 0x0001;
                            else if (!strcmp(cmpstr, "italic")) *selFormat |= 0x0002;
                            else if (!strcmp(cmpstr, "underline")) *selFormat |= 0x0004;
                            else if (!strcmp(cmpstr, "shadow")) *selFormat |= 0x0008;
                            else //Ingenious, but unstable
                            {
                                int amtnum = strtol(pch-2, NULL, 16);
                                *(pch-2) = 0;
                                //Give americans a break
                                if (!strcmp(cmpstr, "colour") || !strcmp(cmpstr, "color")) *selFormat = (*selFormat & 0x0FFF) | ((amtnum & 0xF) << 12);
                                else if (!strcmp(cmpstr, "mask")) *selFormat = (*selFormat & 0xF0FF) | ((amtnum & 0xF) << 8);
                            }
                            break;
                        case 1: //cg_gallery
                            if (!strcmp(cmpstr, "yes")) VNFlags |= 0x0001;
                            break;
                        case 2: //music_room
                            if (!strcmp(cmpstr, "yes")) VNFlags |= 0x0002;
                            break;
                        case 3: //custom_info
                            if (!strcmp(cmpstr, "yes")) VNFlags |= 0x0004;
                            break;
                    }
                    checkingOption = false;
                }
                else if (!strcmp(cmpstr, "format_norm"))
                {
                    selectedOption = 0;
                    selFormat = &normFormat;
                }
                else if (!strcmp(cmpstr, "format_char"))
                {
                    selectedOption = 0;
                    selFormat = &charFormat;
                }
                else if (!strcmp(cmpstr, "format_menu_notsel"))
                {
                    selectedOption = 0;
                    selFormat = &menuNotSelFormat;
                }
                else if (!strcmp(cmpstr, "format_menu_sel"))
                {
                    selectedOption = 0;
                    selFormat = &menuSelFormat;
                }
                else if (!strcmp(cmpstr, "cg_gallery"))
                {
                    selectedOption = 1;
                }
                else if (!strcmp(cmpstr, "music_room"))
                {
                    selectedOption = 2;
                }
                else if (!strcmp(cmpstr, "custom_info"))
                {
                    selectedOption = 3;
                }
            }
            if (ch == '\n' || ch == '\r') //New line
            {
                selectedOption = -1;
                equalsFound = false;
                checkingOption = false;
            }
            cmpstr = pch;
        }
        else if (ch == '=')
        {
            if (selectedOption < 0)
            {
                puts("ERROR - '=' found in master descriptor with no valid option! Line ignored.");
                while (ch)
                {
                    if (ch == '\r' || ch == '\n')
                    {
                        break;
                    }
                    ch = *pch++;
                }
                if (!ch) break;
                pch--;
            }
            else
            {
                equalsFound = true;
                checkingOption = false;
                cmpstr = pch;
            }
        }
        else //Any other character
        {
            checkingOption = true;
        }
        ch = *pch++;
    }

    //Read in scene data object
    FILE* sceneObjectFilePtr = fopen(sceneData, "rb");
    if (sceneObjectFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Scene data object file %s could not be opened!\n", sceneData);
        return 1;
    }
    const long sceneObjectFileLen = GetFileLength(sceneObjectFilePtr);
    sceneObjectData = (unsigned char*)malloc(sceneObjectFileLen);
    fread(sceneObjectData, 1, sceneObjectFileLen, sceneObjectFilePtr);
    fclose(sceneObjectFilePtr);

    //Write out scene data
    char fullDir[512];
    strcpy(fullDir, dir);
    strcat(fullDir, "/");
    char sceneDataFilename[512];
    char sceneDataFilenameNoDir[16];
    memset(sceneDataFilename, 0, 512);
    strcpy(sceneDataFilename, fullDir);
    const char* sceneFilenameNoDirPtr = GetFilenameNoDir(sceneData);
    MakeOutputFilename(sceneFilenameNoDirPtr, sceneDataFilenameNoDir, "DAT");
    strcat(sceneDataFilename, sceneDataFilenameNoDir);
    FILE* sceneDataOutputFilePtr = fopen(sceneDataFilename, "wb");
    if (sceneDataOutputFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Scene data output file %s could not be opened!\n", sceneDataFilename);
        free(sceneObjectData);
        return 1;
    }
    uint64_t sceneLinkInfoPtr = *((uint64_t*)(sceneObjectData));
    fwrite(sceneObjectData + 8, 1, sceneLinkInfoPtr - 8, sceneDataOutputFilePtr);
    fclose(sceneDataOutputFilePtr);

    //Get scene linking info
    unsigned char* sceneObjectLinkInfoPtr = sceneObjectData + sceneLinkInfoPtr;
    numChars = *((uint16_t*)sceneObjectLinkInfoPtr);
    numScenes = *((uint16_t*)(&sceneObjectLinkInfoPtr[0x2]));
    numGlobVars = *((uint16_t*)(&sceneObjectLinkInfoPtr[0x4]));
    numGlobFlags = *((uint16_t*)(&sceneObjectLinkInfoPtr[0x6]));
    numLocalVars = *((uint16_t*)(&sceneObjectLinkInfoPtr[0x8]));
    numLocalFlags = *((uint16_t*)(&sceneObjectLinkInfoPtr[0xA]));
    numBg = *((uint16_t*)(&sceneObjectLinkInfoPtr[0xC]));
    numSpr = *((uint16_t*)(&sceneObjectLinkInfoPtr[0xE]));
    sceneObjectLinkInfoPtr += 0x10;
    sceneDataSceneNamesMap = new std::unordered_map<std::string, int>();
    sceneDataCharNamesMap = new std::unordered_map<std::string, int>();
    sceneDataVarNamesMap = new std::unordered_map<std::string, int>();
    sceneDataBgNamesMap = new std::unordered_map<std::string, int>();
    sceneDataSprNamesMap = new std::unordered_map<std::string, int>();
    //Character names
    for (int i = 0; i < numChars; i++)
    {
        std::string str = std::string((char*)sceneObjectLinkInfoPtr);
        (*sceneDataCharNamesMap)[str] = i;
        sceneObjectLinkInfoPtr += str.size() + 1;
    }
    //Variable names
    for (int i = 0; i < numGlobVars; i++)
    {
        std::string str = std::string((char*)sceneObjectLinkInfoPtr);
        (*sceneDataVarNamesMap)[str] = i + GLOBAL_VAR_BASE;
        sceneObjectLinkInfoPtr += str.size() + 1;
    }
    for (int i = 0; i < numGlobFlags; i++)
    {
        std::string str = std::string((char*)sceneObjectLinkInfoPtr);
        (*sceneDataVarNamesMap)[str] = i + GLOBAL_FLAG_BASE;
        sceneObjectLinkInfoPtr += str.size() + 1;
    }
    for (int i = 0; i < numLocalVars; i++)
    {
        std::string str = std::string((char*)sceneObjectLinkInfoPtr);
        (*sceneDataVarNamesMap)[str] = i + LOCAL_VAR_BASE;
        sceneObjectLinkInfoPtr += str.size() + 1;
    }
    for (int i = 0; i < numLocalFlags; i++)
    {
        std::string str = std::string((char*)sceneObjectLinkInfoPtr);
        (*sceneDataVarNamesMap)[str] = i + LOCAL_FLAG_BASE;
        sceneObjectLinkInfoPtr += str.size() + 1;
    }
    //Scene names
    for (int i = 0; i < numScenes; i++)
    {
        std::string str = std::string((char*)sceneObjectLinkInfoPtr);
        (*sceneDataSceneNamesMap)[str] = i;
        sceneObjectLinkInfoPtr += str.size() + 1;
    }
    //Background names
    for (int i = 0; i < numBg; i++)
    {
        std::string str = std::string((char*)sceneObjectLinkInfoPtr);
        (*sceneDataBgNamesMap)[str] = i;
        sceneObjectLinkInfoPtr += str.size() + 1;
    }
    //Sprite names
    for (int i = 0; i < numSpr; i++)
    {
        std::string str = std::string((char*)sceneObjectLinkInfoPtr);
        (*sceneDataSprNamesMap)[str] = i;
        sceneObjectLinkInfoPtr += str.size() + 1;
    }
    //Individual scenes
    for (int i = 0; i < numScenes; i++)
    {
        int numTexts = *((uint16_t*)sceneObjectLinkInfoPtr);
        sceneObjectLinkInfoPtr += 2;
        std::unordered_map<std::string, int>* scdTextsMap = new std::unordered_map<std::string, int>();
        for (int j = 0; j < numTexts; j++)
        {
            std::string str = std::string((char*)sceneObjectLinkInfoPtr);
            (*scdTextsMap)[str] = j;
            sceneObjectLinkInfoPtr += str.size() + 1;
        }
        scenes[i].sceneDataTextsMap = scdTextsMap;
        scenes[i].numTextsInScene = numTexts;
    }
    //Individual backgrounds
    for (int i = 0; i < numBg; i++)
    {
        int numVariants = *((uint16_t*)sceneObjectLinkInfoPtr);
        sceneObjectLinkInfoPtr += 2;
        std::unordered_map<std::string, int>* bgVariantsMap = new std::unordered_map<std::string, int>();
        for (int j = 0; j < numVariants; j++)
        {
            std::string str = std::string((char*)sceneObjectLinkInfoPtr);
            (*bgVariantsMap)[str] = j;
            sceneObjectLinkInfoPtr += str.size() + 1;
        }
        bgs[i].imageDataVariantsMap = bgVariantsMap;
        bgs[i].numVariantsInImage= numVariants;
    }
    //Individual sprites
    for (int i = 0; i < numSpr; i++)
    {
        int numVariants = *((uint16_t*)sceneObjectLinkInfoPtr);
        sceneObjectLinkInfoPtr += 2;
        std::unordered_map<std::string, int>* sprVariantsMap = new std::unordered_map<std::string, int>();
        for (int j = 0; j < numVariants; j++)
        {
            std::string str = std::string((char*)sceneObjectLinkInfoPtr);
            (*sprVariantsMap)[str] = j;
            sceneObjectLinkInfoPtr += str.size() + 1;
        }
        sprs[i].imageDataVariantsMap = sprVariantsMap;
        sprs[i].numVariantsInImage= numVariants;
    }

    //Write language metadata file
    char langDataFilename[512];
    memset(langDataFilename, 0, 512);
    strcpy(langDataFilename, fullDir);
    strcat(langDataFilename, "LANGUAGE.DAT");
    FILE* langDataFilePtr = fopen(langDataFilename, "wb");
    if (langDataFilePtr == NULL)
    {
        puts("CRITICAL ERROR - Language metadata file could not be opened!\n");
        for (int i = 0; i < numScenes; i++)
        {
            delete scenes[i].sceneDataTextsMap;
        }
        delete sceneDataSceneNamesMap;
        delete sceneDataCharNamesMap;
        delete sceneDataVarNamesMap;
        free(sceneObjectData);
        return 1;
    }
    fwrite(&numTextDatas, sizeof(uint16_t), 1, langDataFilePtr);
    uint32_t* textFilenamePtrs = (uint32_t*)malloc(sizeof(uint32_t) * numTextDatas);
    char* textOutputFilenamesNoDir = (char*)malloc(65536);
    uint32_t textFilenamePtrCounter = 0;
    for (int i = 0; i < numTextDatas; i++)
    {
        const char* toFilenameNoDirPtr = GetFilenameNoDir(textData[i]);
        int fnLen = MakeOutputFilename(toFilenameNoDirPtr, &textOutputFilenamesNoDir[textFilenamePtrCounter], "TXA");
        textFilenamePtrs[i] = textFilenamePtrCounter + 2 + 4 * numTextDatas;
        textFilenamePtrCounter += fnLen + 1;
    }
    fwrite(textFilenamePtrs, sizeof(uint32_t), numTextDatas, langDataFilePtr);
    fwrite(textOutputFilenamesNoDir, 1, textFilenamePtrCounter, langDataFilePtr);
    fclose(langDataFilePtr);

    //Link text archive files with scene data
    for (int i = 0; i < numTextDatas; i++)
    {
        textFilenamePtrs[i] -= 2 + 4 * numTextDatas;
        char textDataOutputFilename[512];
        memset(textDataOutputFilename, 0, 512);
        strcpy(textDataOutputFilename, fullDir);
        strcat(textDataOutputFilename, &textOutputFilenamesNoDir[textFilenamePtrs[i]]);
        int result = LinkTextData(textData[i], textDataOutputFilename);
        if (result)
        {
            free(textFilenamePtrs);
            free(textOutputFilenamesNoDir);
            for (int i = 0; i < numScenes; i++)
            {
                delete scenes[i].sceneDataTextsMap;
            }
            for (int i = 0; i < numBg; i++)
            {
                delete bgs[i].imageDataVariantsMap;
            }
            for (int i = 0; i < numSpr; i++)
            {
                delete sprs[i].imageDataVariantsMap;
            }
            delete sceneDataSceneNamesMap;
            delete sceneDataCharNamesMap;
            delete sceneDataVarNamesMap;
            delete sceneDataBgNamesMap;
            delete sceneDataSprNamesMap;
            free(sceneObjectData);
            return result;
        }
    }
    free(textFilenamePtrs);
    free(textOutputFilenamesNoDir);

    //Link background archive file with scene data
    //Read in background data object
    FILE* bgObjectFilePtr = fopen(bgData, "rb");
    if (bgObjectFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Background data object file %s could not be opened!\n", bgData);
        for (int i = 0; i < numScenes; i++)
        {
            delete scenes[i].sceneDataTextsMap;
        }
        for (int i = 0; i < numBg; i++)
        {
            delete bgs[i].imageDataVariantsMap;
        }
        for (int i = 0; i < numSpr; i++)
        {
            delete sprs[i].imageDataVariantsMap;
        }
        delete sceneDataSceneNamesMap;
        delete sceneDataCharNamesMap;
        delete sceneDataVarNamesMap;
        delete sceneDataBgNamesMap;
        delete sceneDataSprNamesMap;
        free(sceneObjectData);
        return 1;
    }
    const long bgObjectFileLen = GetFileLength(bgObjectFilePtr);
    unsigned char* bgObjectData = (unsigned char*)malloc(bgObjectFileLen);
    fread(bgObjectData, 1, bgObjectFileLen, bgObjectFilePtr);
    fclose(bgObjectFilePtr);
    uint64_t bgLinkInfoPtr = *((uint64_t*)(bgObjectData));
    unsigned char* bgObjectLinkInfoPtr = bgObjectData + bgLinkInfoPtr;

    //Link background data based on scene data
    int numBgsInArchive = *((uint16_t*)bgObjectLinkInfoPtr);
    uint32_t* bgPtrs = (uint32_t*)(&bgObjectData[8 + *((uint32_t*)(&bgObjectData[8]))]);
    bgObjectLinkInfoPtr += 2;

    //Fix background name pointers
    uint64_t* bgVariantLinkPtrs = (uint64_t*)bgObjectLinkInfoPtr;
    bgObjectLinkInfoPtr += 8 * numBgsInArchive;
    if (numBgsInArchive < numBg)
    {
        printf("CRITICAL ERROR - Some backgrounds have not been given in %s!\n", bgData);
        for (int i = 0; i < numScenes; i++)
        {
            delete scenes[i].sceneDataTextsMap;
        }
        for (int i = 0; i < numBg; i++)
        {
            delete bgs[i].imageDataVariantsMap;
        }
        for (int i = 0; i < numSpr; i++)
        {
            delete sprs[i].imageDataVariantsMap;
        }
        delete sceneDataSceneNamesMap;
        delete sceneDataCharNamesMap;
        delete sceneDataVarNamesMap;
        delete sceneDataBgNamesMap;
        delete sceneDataSprNamesMap;
        free(bgObjectData);
        free(sceneObjectData);
        return 1;
    }
    for (int i = 0; i < numBg; i++)
    {
        uint32_t bptr = bgPtrs[i];
        std::string str = std::string((char*)bgObjectLinkInfoPtr);
        int newIndex = (*sceneDataBgNamesMap)[str];
        bgIndexArray[i] = newIndex;
        Ptr32TempArray[newIndex] = bptr;
        bgObjectLinkInfoPtr += str.size()+ 1;
    }
    memcpy(bgPtrs, Ptr32TempArray, sizeof(uint32_t) * numBg);
    if (numBgsInArchive > numBg)
    {
        printf("WARNING - Some names in %s correspond to unused backgrounds!\n", bgData);
        for (int i = numBg; i < numBgsInArchive; i++) //Skip over unused background names
        {
            while (1)
            {
                if (*bgObjectLinkInfoPtr++ == 0)
                {
                    break;
                }
            }
        }
    }
    unsigned char* bgVariantsLinkInfoBasePtr = bgObjectLinkInfoPtr;

    //Fix background variants
    for (int i = 0; i < numBg; i++)
    {
        bgObjectLinkInfoPtr = bgVariantsLinkInfoBasePtr + bgVariantLinkPtrs[i];
        int numVariantsInBgInBg = *((uint16_t*)bgObjectLinkInfoPtr);
        bgObjectLinkInfoPtr += 2;
        int realIndex = bgIndexArray[i];
        int numVariantsInBg = bgs[realIndex].numVariantsInImage;
        if (numVariantsInBgInBg < numVariantsInBg)
        {
            printf("CRITICAL ERROR - Some variants in background %u in %s have not been written!\n", i, bgData);
            for (int i = 0; i < numScenes; i++)
            {
                delete scenes[i].sceneDataTextsMap;
            }
            for (int i = 0; i < numBg; i++)
            {
                delete bgs[i].imageDataVariantsMap;
            }
            for (int i = 0; i < numSpr; i++)
            {
                delete sprs[i].imageDataVariantsMap;
            }
            delete sceneDataSceneNamesMap;
            delete sceneDataCharNamesMap;
            delete sceneDataVarNamesMap;
            delete sceneDataBgNamesMap;
            delete sceneDataSprNamesMap;
            free(bgObjectData);
            free(sceneObjectData);
            return 1;
        }
        std::unordered_map<std::string, int>* variantsMap = bgs[realIndex].imageDataVariantsMap;
        uint64_t* curBgVariants = ((uint64_t*)(((unsigned char*)(bgPtrs + numBgsInArchive)) + bgPtrs[i] + 8));
        for (int j = 0; j < numVariantsInBg; j++)
        {
            uint64_t vptr = curBgVariants[j];
            std::string str = std::string((char*)bgObjectLinkInfoPtr);
            int newIndex = (*variantsMap)[str];
            Ptr64TempArray[newIndex] = vptr;
            bgObjectLinkInfoPtr += str.size()+ 1;
        }
        memcpy(curBgVariants, Ptr64TempArray, sizeof(uint64_t) * numVariantsInBg);
        if (numVariantsInBgInBg > numVariantsInBg)
        {
            printf("WARNING - Some variants in background %u in %s correspond to unused variants!\n", i, bgData);
        }
    }


    //Write out linked background data
    char bgDataFilename[512];
    char bgDataFilenameNoDir[16];
    memset(bgDataFilename, 0, 512);
    strcpy(bgDataFilename, fullDir);
    const char* bgFilenameNoDirPtr = GetFilenameNoDir(bgData);
    MakeOutputFilename(bgFilenameNoDirPtr, bgDataFilenameNoDir, "DAT");
    strcat(bgDataFilename, bgDataFilenameNoDir);
    FILE* bgDataOutputFilePtr = fopen(bgDataFilename, "wb");
    if (bgDataOutputFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Background data output file %s could not be opened!\n", bgDataFilename);
        for (int i = 0; i < numScenes; i++)
        {
            delete scenes[i].sceneDataTextsMap;
        }
        for (int i = 0; i < numBg; i++)
        {
            delete bgs[i].imageDataVariantsMap;
        }
        for (int i = 0; i < numSpr; i++)
        {
            delete sprs[i].imageDataVariantsMap;
        }
        delete sceneDataSceneNamesMap;
        delete sceneDataCharNamesMap;
        delete sceneDataVarNamesMap;
        delete sceneDataBgNamesMap;
        delete sceneDataSprNamesMap;
        free(bgObjectData);
        free(sceneObjectData);
        return 1;
    }
    fwrite(bgObjectData + 8, 1, bgLinkInfoPtr - 8, bgDataOutputFilePtr);
    fclose(bgDataOutputFilePtr);
    free(bgObjectData);

    //Link sprite archive file with scene data
    //Read in sprite data object
    FILE* sprObjectFilePtr = fopen(spriteData, "rb");
    if (sprObjectFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Sprite data object file %s could not be opened!\n", spriteData);
        for (int i = 0; i < numScenes; i++)
        {
            delete scenes[i].sceneDataTextsMap;
        }
        for (int i = 0; i < numBg; i++)
        {
            delete bgs[i].imageDataVariantsMap;
        }
        for (int i = 0; i < numSpr; i++)
        {
            delete sprs[i].imageDataVariantsMap;
        }
        delete sceneDataSceneNamesMap;
        delete sceneDataCharNamesMap;
        delete sceneDataVarNamesMap;
        delete sceneDataBgNamesMap;
        delete sceneDataSprNamesMap;
        free(sceneObjectData);
        return 1;
    }
    const long sprObjectFileLen = GetFileLength(sprObjectFilePtr);
    unsigned char* sprObjectData = (unsigned char*)malloc(sprObjectFileLen);
    fread(sprObjectData, 1, sprObjectFileLen, sprObjectFilePtr);
    fclose(sprObjectFilePtr);
    uint64_t sprLinkInfoPtr = *((uint64_t*)(sprObjectData));
    unsigned char* sprObjectLinkInfoPtr = sprObjectData + sprLinkInfoPtr;

    //Link sprite data based on scene data
    int numSprsInArchive = *((uint16_t*)sprObjectLinkInfoPtr);
    uint32_t* sprPtrs = (uint32_t*)(&sprObjectData[8 + 2]);
    sprObjectLinkInfoPtr += 2;

    //Fix sprite name pointers
    uint64_t* sprVariantLinkPtrs = (uint64_t*)sprObjectLinkInfoPtr;
    sprObjectLinkInfoPtr += 8 * numSprsInArchive;
    if (numSprsInArchive < numSpr)
    {
        printf("CRITICAL ERROR - Some sprites have not been given in %s!\n", spriteData);
        for (int i = 0; i < numScenes; i++)
        {
            delete scenes[i].sceneDataTextsMap;
        }
        for (int i = 0; i < numBg; i++)
        {
            delete bgs[i].imageDataVariantsMap;
        }
        for (int i = 0; i < numSpr; i++)
        {
            delete sprs[i].imageDataVariantsMap;
        }
        delete sceneDataSceneNamesMap;
        delete sceneDataCharNamesMap;
        delete sceneDataVarNamesMap;
        delete sceneDataBgNamesMap;
        delete sceneDataSprNamesMap;
        free(sprObjectData);
        free(sceneObjectData);
        return 1;
    }
    for (int i = 0; i < numSpr; i++)
    {
        uint32_t sptr = sprPtrs[i];
        std::string str = std::string((char*)sprObjectLinkInfoPtr);
        int newIndex = (*sceneDataSprNamesMap)[str];
        sprIndexArray[i] = newIndex;
        Ptr32TempArray[newIndex] = sptr;
        sprObjectLinkInfoPtr += str.size()+ 1;
    }
    memcpy(sprPtrs, Ptr32TempArray, sizeof(uint32_t) * numSpr);
    if (numSprsInArchive > numSpr)
    {
        printf("WARNING - Some names in %s correspond to unused sprites!\n", spriteData);
        for (int i = numSpr; i < numSprsInArchive; i++) //Skip over unused sprite names
        {
            while (1)
            {
                if (*sprObjectLinkInfoPtr++ == 0)
                {
                    break;
                }
            }
        }
    }
    unsigned char* sprVariantsLinkInfoBasePtr = sprObjectLinkInfoPtr;

    //Fix sprite variants
    for (int i = 0; i < numSpr; i++)
    {
        sprObjectLinkInfoPtr = sprVariantsLinkInfoBasePtr + sprVariantLinkPtrs[i];
        int numVariantsInSprInSpr = *((uint16_t*)sprObjectLinkInfoPtr);
        sprObjectLinkInfoPtr += 2;
        int realIndex = sprIndexArray[i];
        int numVariantsInSpr = sprs[realIndex].numVariantsInImage;
        if (numVariantsInSprInSpr < numVariantsInSpr)
        {
            printf("CRITICAL ERROR - Some variants in sprite %u in %s have not been written!\n", i, spriteData);
            for (int i = 0; i < numScenes; i++)
            {
                delete scenes[i].sceneDataTextsMap;
            }
            for (int i = 0; i < numBg; i++)
            {
                delete bgs[i].imageDataVariantsMap;
            }
            for (int i = 0; i < numSpr; i++)
            {
                delete sprs[i].imageDataVariantsMap;
            }
            delete sceneDataSceneNamesMap;
            delete sceneDataCharNamesMap;
            delete sceneDataVarNamesMap;
            delete sceneDataBgNamesMap;
            delete sceneDataSprNamesMap;
            free(sprObjectData);
            free(sceneObjectData);
            return 1;
        }
        std::unordered_map<std::string, int>* variantsMap = sprs[realIndex].imageDataVariantsMap;
        uint64_t* curSprVariants = ((uint64_t*)(((unsigned char*)(sprPtrs + numSprsInArchive)) + sprPtrs[i] + 6));
        for (int j = 0; j < numVariantsInSpr; j++)
        {
            uint64_t vptr = curSprVariants[j];
            std::string str = std::string((char*)sprObjectLinkInfoPtr);
            int newIndex = (*variantsMap)[str];
            Ptr64TempArray[newIndex] = vptr;
            sprObjectLinkInfoPtr += str.size()+ 1;
        }
        memcpy(curSprVariants, Ptr64TempArray, sizeof(uint64_t) * numVariantsInSpr);
        if (numVariantsInSprInSpr > numVariantsInSpr)
        {
            printf("WARNING - Some variants in sprite %u in %s correspond to unused variants!\n", i, spriteData);
        }
    }


    //Write out linked sprite data
    char spriteDataFilename[512];
    char spriteDataFilenameNoDir[16];
    memset(spriteDataFilename, 0, 512);
    strcpy(spriteDataFilename, fullDir);
    const char* spriteFilenameNoDirPtr = GetFilenameNoDir(spriteData);
    MakeOutputFilename(spriteFilenameNoDirPtr, spriteDataFilenameNoDir, "DAT");
    strcat(spriteDataFilename, spriteDataFilenameNoDir);
    FILE* sprDataOutputFilePtr = fopen(spriteDataFilename, "wb");
    if (sprDataOutputFilePtr == NULL)
    {
        printf("CRITICAL ERROR - Sprite data output file %s could not be opened!\n", spriteDataFilename);
        for (int i = 0; i < numScenes; i++)
        {
            delete scenes[i].sceneDataTextsMap;
        }
        for (int i = 0; i < numBg; i++)
        {
            delete bgs[i].imageDataVariantsMap;
        }
        for (int i = 0; i < numSpr; i++)
        {
            delete sprs[i].imageDataVariantsMap;
        }
        delete sceneDataSceneNamesMap;
        delete sceneDataCharNamesMap;
        delete sceneDataVarNamesMap;
        delete sceneDataBgNamesMap;
        delete sceneDataSprNamesMap;
        free(sprObjectData);
        free(sceneObjectData);
        return 1;
    }
    fwrite(sprObjectData + 8, 1, sprLinkInfoPtr - 8, sprDataOutputFilePtr);
    fclose(sprDataOutputFilePtr);
    free(sprObjectData);

    //Linking done
    for (int i = 0; i < numScenes; i++)
    {
        delete scenes[i].sceneDataTextsMap;
    }
    for (int i = 0; i < numBg; i++)
    {
        delete bgs[i].imageDataVariantsMap;
    }
    for (int i = 0; i < numSpr; i++)
    {
        delete sprs[i].imageDataVariantsMap;
    }
    delete sceneDataSceneNamesMap;
    delete sceneDataCharNamesMap;
    delete sceneDataVarNamesMap;
    delete sceneDataBgNamesMap;
    delete sceneDataSprNamesMap;
    free(sceneObjectData);

    //Write root info
    char rootinfoFilename[512];
    strcpy(rootinfoFilename, fullDir);
    strcat(rootinfoFilename, "ROOTINFO.DAT");
    unsigned char* rootinfo = (unsigned char*)calloc(0x76, 1);
    rootinfo[0] = 'M'; rootinfo[1] = 'H'; rootinfo[2] = 'V'; rootinfo[3] = 'N'; //Magic number
    *((uint16_t*)(&rootinfo[0x04])) = VNFlags; //VNflags
    *((uint16_t*)(&rootinfo[0x06])) = (uint16_t)numGlobVars; //numstvar_glob
    *((uint16_t*)(&rootinfo[0x08])) = (uint16_t)numGlobFlags; //numflags_glob
    *((uint16_t*)(&rootinfo[0x0A])) = (uint16_t)numLocalVars; //numstvar_loc
    *((uint16_t*)(&rootinfo[0x0C])) = (uint16_t)numLocalFlags; //numflags_loc
    *((uint16_t*)(&rootinfo[0x0E])) = normFormat; //format_norm
    *((uint16_t*)(&rootinfo[0x10])) = charFormat; //format_char
    *((uint16_t*)(&rootinfo[0x12])) = menuNotSelFormat; //format_menu_n
    *((uint16_t*)(&rootinfo[0x14])) = menuSelFormat; //format_menu_S

    char fontDataFilename[512];
    char fontDataFilenameNoDir[16];
    memset(fontDataFilename, 0, 512);
    strcpy(fontDataFilename, fullDir);
    const char* fontFilenameNoDirPtr = GetFilenameNoDir(fontData);
    const char* fontFilenameNoDirExtPtr = fontFilenameNoDirPtr;
    const char* fontFilenameNoDirExt = NULL;
    char dch = *fontFilenameNoDirExtPtr++;
    while (dch)
    {
        if (dch == '.')
        {
            fontFilenameNoDirExt = fontFilenameNoDirExtPtr;
        }
        dch = *fontFilenameNoDirExtPtr++;
    }
    MakeOutputFilename(fontFilenameNoDirPtr, fontDataFilenameNoDir, fontFilenameNoDirExt);
    strcat(fontDataFilename, fontDataFilenameNoDir);
    char* filenameptr = (char*)(&rootinfo[0x16]);
    pch = fontDataFilenameNoDir;
    ch = *pch++;
    while(ch)
    {
        *filenameptr++ = ch;
        ch = *pch++;
    }

    filenameptr = (char*)(&rootinfo[0x22]);
    pch = sceneDataFilenameNoDir;
    ch = *pch++;
    while(ch)
    {
        *filenameptr++ = ch;
        ch = *pch++;
    }

    strcpy((char*)(&rootinfo[0x2E]), "LANGUAGE.DAT"); //fixed by design

    filenameptr = (char*)(&rootinfo[0x3A]);
    pch = bgDataFilenameNoDir;
    ch = *pch++;
    while(ch)
    {
        *filenameptr++ = ch;
        ch = *pch++;
    }

    filenameptr = (char*)(&rootinfo[0x46]);
    pch = spriteDataFilenameNoDir;
    ch = *pch++;
    while(ch)
    {
        *filenameptr++ = ch;
        ch = *pch++;
    }

    char musicDataFilename[512];
    char musicDataFilenameNoDir[16];
    memset(musicDataFilename, 0, 512);
    strcpy(musicDataFilename, fullDir);
    const char* musicFilenameNoDirPtr = GetFilenameNoDir(musicData);
    MakeOutputFilename(musicFilenameNoDirPtr, musicDataFilenameNoDir, "DAT");
    strcat(musicDataFilename, musicDataFilenameNoDir);
    filenameptr = (char*)(&rootinfo[0x52]);
    pch = musicDataFilenameNoDir;
    ch = *pch++;
    while(ch)
    {
        *filenameptr++ = ch;
        ch = *pch++;
    }

    char sfxDataFilename[512];
    char sfxDataFilenameNoDir[16];
    memset(sfxDataFilename, 0, 512);
    strcpy(sfxDataFilename, fullDir);
    const char* sfxFilenameNoDirPtr = GetFilenameNoDir(sfxData);
    MakeOutputFilename(sfxFilenameNoDirPtr, sfxDataFilenameNoDir, "DAT");
    strcat(sfxDataFilename, sfxDataFilenameNoDir);
    filenameptr = (char*)(&rootinfo[0x5E]);
    pch = sfxDataFilenameNoDir;
    ch = *pch++;
    while(ch)
    {
        *filenameptr++ = ch;
        ch = *pch++;
    }

    char sysDataFilename[512];
    char sysDataFilenameNoDir[16];
    memset(sysDataFilename, 0, 512);
    strcpy(sysDataFilename, fullDir);
    const char* sysFilenameNoDirPtr = GetFilenameNoDir(sysData);
    MakeOutputFilename(sysFilenameNoDirPtr, sysDataFilenameNoDir, "DAT");
    strcat(sysDataFilename, sysDataFilenameNoDir);
    filenameptr = (char*)(&rootinfo[0x6A]);
    pch = sysDataFilenameNoDir;
    ch = *pch++;
    while(ch)
    {
        *filenameptr++ = ch;
        ch = *pch++;
    }

    FILE* rootinfofile = fopen(rootinfoFilename, "wb");
    if (rootinfofile == NULL)
    {
        puts("CRITICAL ERROR - Root info file could not be opened!\n");
        free(rootinfo);
        return 1;
    }
    fwrite(rootinfo, 1, 0x76, rootinfofile);
    fclose(rootinfofile);

    free(rootinfo);
    return 0;
}
