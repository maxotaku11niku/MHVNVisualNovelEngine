/* MHVNTXAR - Text data archiver for the MHVN Visual Novel Engine
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
 * Main parser and archiver code
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//Initial allocation sizes, and also how many more bytes/entries to add if more memory needs to be allocated for some reason
#define DEFAULT_ALLOCBLOCK_SNB 65536
#define DEFAULT_ALLOCBLOCK_STENB 65536
#define DEFAULT_ALLOCBLOCK_STETB 1048576
#define DEFAULT_ALLOCBLOCK_CLB 65536
#define DEFAULT_ALLOCBLOCK_CNB 65536
#define DEFAULT_ALLOCBLOCK_SEP 256
#define DEFAULT_ALLOCBLOCK_TXA 1048576
#define DEFAULT_ALLOCBLOCK_LINK 1048576

#define EOT ((char)0x04)

typedef struct
{
    char* sceneName;
    char** entryNames;
    char** entryTexts;
    int numEntries;
    int allocSize;
} SceneText;

typedef struct
{
    char* label;
    char* name;
} CharName;

SceneText scenes[65536];
int numScenes = 0;
CharName charNames[65536];
int numChars = 0;

char* sceneNamesBuffer;
char* sceneNamesBufPtr;
size_t sceneNamesBufferAllocSize;
char* sceneTextEntryNamesBuffer;
char* sceneTextEntryNamesBufPtr;
size_t sceneTextEntryNamesBufferAllocSize;
char* sceneTextEntryTextsBuffer;
char* sceneTextEntryTextsBufPtr;
size_t sceneTextEntryTextsBufferAllocSize;
char* charLabelsBuffer;
char* charLabelsBufPtr;
size_t charLabelsBufferAllocSize;
char* charNamesBuffer;
char* charNamesBufPtr;
size_t charNamesBufferAllocSize;
char* txaFileOutputBuffer;
char* txaFileOutputBufPtr;
size_t txaFileOutputBufferAllocSize;
char* linkInfoOutputBuffer;
char* linkInfoOutputBufPtr;
size_t linkInfoOutputBufferAllocSize;

typedef enum
{
    NOTINENTRY,
    INNORMALENTRY,
    INCHARNAMEENTRY
} ParsingState;

const char* invalidAtCommandMessage = "WARNING - %s, line %u: invalid @ command. Command ignored.\n";
const char* missingParamMessage = "WARNING - %s, line %u: %s command missing parameters which should be in brackets (like this). Command ignored.\n";
const char* invalidParamMessage = "WARNING - %s, line %u: %s command has invalid parameters. Command ignored.\n";
const char* declInTextMessage = "WARNING - %s, line %u: attempted to declare new label in the body text of another declaration. Command ignored.\n";
const char* declMissingParamMessage = "ERROR - %s, line %u: %s command missing parameters which should be in brackets (like this). Entry ignored.\n";
const char* missingLabelMessage = "ERROR - %s, line %u: declaration is missing a label. Entry ignored.\n";
const char* missingTextMessage = "ERROR - %s, line %u: declaration is missing body text. Entry ignored.\n";
const char* noSceneMessage = "ERROR - %s, line %u: normal text entry declared without a valid scene declaration, nor with a system reserved label. Entry ignored.\n";

//Quick convenience function
char SingleHexCharToBinary(char ch)
{
    switch (ch)
    {
        case '0': return 0x0;
        case '1': return 0x1;
        case '2': return 0x2;
        case '3': return 0x3;
        case '4': return 0x4;
        case '5': return 0x5;
        case '6': return 0x6;
        case '7': return 0x7;
        case '8': return 0x8;
        case '9': return 0x9;
        case 'a': return 0xA;
        case 'b': return 0xB;
        case 'c': return 0xC;
        case 'd': return 0xD;
        case 'e': return 0xE;
        case 'f': return 0xF;
        default: return -1;
    }
}

int ParseInputFile(const char* contents, const long length, const char* filename)
{
    int curScene = -1;
    unsigned int lineNum = 1;
    ParsingState pState = NOTINENTRY;
    bool isCommandTextInject = false;
    char commandTextInject[16];
    char tempLabelBuffer[256];
    int tlbufPtr = 0;
    int ctInjectPtr = 0;
    char* cnameb = charNamesBufPtr;
    char* stetb = sceneTextEntryTextsBufPtr;
    SceneText* curSceneTexts;

    /*
    Notes for the parsing process:
    Most text that is not in an entry of some kind is treated as a comment (that is, it doesn't get put in the output file in some way)
    Command parsing output is put into a small stack allocated buffer, and passed forward.
    Perhaps I should rearchitect it at a later date.
    */

    for (long i = 0; i < length; i++)
    {
        char curChar = *contents++;
        if (curChar == '@') //Scan for the command character to find text fields and stuff
        {
            curChar = *contents++; i++;
            switch (curChar)
            {
                case '@': //Print single literal '@'
                    if (pState != NOTINENTRY)
                    {
                        isCommandTextInject = true;
                        commandTextInject[0] = '@';
                        commandTextInject[1] = '\0';
                    }
                    break;
                case '[': //Begin normal entry
                    if (pState != NOTINENTRY)
                    {
                        printf(declInTextMessage, filename, lineNum);
                        continue;
                    }
                    if (curScene < 0)
                    {
                        //TODO: Add logic to check for system reserved labels
                        printf(noSceneMessage, filename, lineNum);
                    }
                    curChar = *contents++; i++;
                    if (curChar == '\n' || curChar == '\r')
                    {
                        printf(missingLabelMessage, filename, lineNum);
                        continue;
                    }
                    while (curChar != '\n')
                    {
                        tempLabelBuffer[tlbufPtr] = curChar;
                        curChar = *contents++; i++;
                        tlbufPtr++;
                    }
                    if (contents[-2] == '\r') //LF normalisation
                    {
                        tempLabelBuffer[tlbufPtr - 1] = '\0';
                    }
                    else
                    {
                        tempLabelBuffer[tlbufPtr] = '\0'; //Null terminate to make life easier
                        tlbufPtr++;
                    }
                    memcpy(sceneTextEntryNamesBufPtr, tempLabelBuffer, tlbufPtr);
                    curSceneTexts->entryNames[curSceneTexts->numEntries] = sceneTextEntryNamesBufPtr;
                    sceneTextEntryNamesBufPtr += tlbufPtr;
                    curSceneTexts->entryTexts[curSceneTexts->numEntries] = stetb;
                    pState = INNORMALENTRY;
                    tlbufPtr = 0;
                    continue;
                case ']': //End normal entry
                    if (pState == INNORMALENTRY)
                    {
                        if (tlbufPtr <= 0)
                        {
                            printf(missingTextMessage, filename, lineNum);
                            pState = NOTINENTRY;
                            sceneTextEntryNamesBufPtr = curSceneTexts->entryNames[curSceneTexts->numEntries];
                            continue;
                        }
                        *stetb++ = '\0';
                        curSceneTexts->numEntries++;
                        pState = NOTINENTRY;
                        tlbufPtr = 0;
                    }
                    else if (pState != NOTINENTRY)
                    {
                        printf(declInTextMessage, filename, lineNum);
                        continue;
                    }
                    continue;
                case 's': //Declare scene
                    if (pState != NOTINENTRY)
                    {
                        printf(declInTextMessage, filename, lineNum);
                        continue;
                    }
                    curChar = *contents++; i++;
                    if (curChar != '(')
                    {
                        printf(declMissingParamMessage, filename, lineNum, "@s");
                        continue;
                    }
                    curChar = *contents++; i++;
                    while (curChar != ')')
                    {
                        tempLabelBuffer[tlbufPtr] = curChar;
                        curChar = *contents++; i++;
                        tlbufPtr++;
                    }
                    if (tlbufPtr <= 0)
                    {
                        printf(missingLabelMessage, filename, lineNum);
                        continue;
                    }
                    tempLabelBuffer[tlbufPtr] = '\0'; //Null terminate to make life easier
                    tlbufPtr++;
                    //TODO: Must add logic to find if a scene with a certain name already exists
                    memcpy(sceneNamesBufPtr, tempLabelBuffer, tlbufPtr);
                    curScene = numScenes;
                    curSceneTexts = &scenes[curScene];
                    numScenes++;
                    curSceneTexts->sceneName = sceneNamesBufPtr;
                    curSceneTexts->entryNames = malloc(sizeof(char*) * DEFAULT_ALLOCBLOCK_SEP);
                    curSceneTexts->entryTexts = malloc(sizeof(char*) * DEFAULT_ALLOCBLOCK_SEP);
                    curSceneTexts->numEntries = 0;
                    curSceneTexts->allocSize = DEFAULT_ALLOCBLOCK_SEP;
                    sceneNamesBufPtr += tlbufPtr;
                    tlbufPtr = 0;
                    continue;
                case 'n': //Declare character
                    if (pState != NOTINENTRY)
                    {
                        printf(declInTextMessage, filename, lineNum);
                        continue;
                    }
                    curChar = *contents++; i++;
                    if (curChar != '(')
                    {
                        printf(declMissingParamMessage, filename, lineNum, "@n");
                        continue;
                    }
                    curChar = *contents++; i++;
                    while (curChar != ')')
                    {
                        tempLabelBuffer[tlbufPtr] = curChar;
                        curChar = *contents++; i++;
                        tlbufPtr++;
                    }
                    if (tlbufPtr <= 0)
                    {
                        printf(missingLabelMessage, filename, lineNum);
                        continue;
                    }
                    tempLabelBuffer[tlbufPtr] = '\0'; //Null terminate to make life easier
                    tlbufPtr++;
                    memcpy(charLabelsBufPtr, tempLabelBuffer, tlbufPtr);
                    charNames[numChars].label = charLabelsBufPtr;
                    charLabelsBufPtr += tlbufPtr;
                    charNames[numChars].name = cnameb;
                    pState = INCHARNAMEENTRY;
                    tlbufPtr = 0;
                    continue;
                case 'f': //Set format
                    if (pState != NOTINENTRY)
                    {
                        curChar = *contents++; i++;
                        if (curChar != '(')
                        {
                            printf(missingParamMessage, filename, lineNum, "@f");
                            cheekyBreakf:
                            continue;
                        }
                        curChar = *contents++; i++;
                        bool invalidParam = false;
                        char fmtchar = 0x10;
                        while (curChar != ')')
                        {
                            switch (curChar)
                            {
                            case 'b':
                                fmtchar |= 0x01;
                                break;
                            case 'i':
                                fmtchar |= 0x02;
                                break;
                            case 'u':
                                fmtchar |= 0x04;
                                break;
                            case 's':
                                fmtchar |= 0x08;
                                break;
                            default:
                                printf(invalidParamMessage, filename, lineNum, "@f");
                                goto cheekyBreakf; //ok now this is utterly cursed, but it's faster!!!
                            }
                            curChar = *contents++; i++;
                        }
                        isCommandTextInject = true;
                        commandTextInject[0] = '\x1B';
                        commandTextInject[1] = fmtchar;
                        commandTextInject[2] = '\0';
                    }
                    break;
                case 'm': //Set fade
                    if (pState != NOTINENTRY)
                    {
                        curChar = *contents++; i++;
                        char fmtchar = SingleHexCharToBinary(curChar);
                        if (fmtchar == -1)
                        {
                            printf(invalidParamMessage, filename, lineNum, "@m");
                            continue;
                        }
                        fmtchar |= 0x30;
                        isCommandTextInject = true;
                        commandTextInject[0] = '\x1B';
                        commandTextInject[1] = fmtchar;
                        commandTextInject[2] = '\0';
                    }
                    break;
                case 'c': //Set colour
                    if (pState != NOTINENTRY)
                    {
                        curChar = *contents++; i++;
                        char fmtchar = SingleHexCharToBinary(curChar);
                        if (fmtchar == -1)
                        {
                            printf(invalidParamMessage, filename, lineNum, "@c");
                            continue;
                        }
                        fmtchar |= 0x40;
                        isCommandTextInject = true;
                        commandTextInject[0] = '\x1B';
                        commandTextInject[1] = fmtchar;
                        commandTextInject[2] = '\0';
                    }
                    break;
                case 'p': //Insert custom info
                    if (pState != NOTINENTRY)
                    {
                        curChar = *contents++; i++;
                        char fmtchar = SingleHexCharToBinary(curChar);
                        if (fmtchar == -1)
                        {
                            printf(invalidParamMessage, filename, lineNum, "@p");
                            continue;
                        }
                        fmtchar |= 0x50;
                        isCommandTextInject = true;
                        commandTextInject[0] = '\x1B';
                        commandTextInject[1] = fmtchar;
                        commandTextInject[2] = '\0';
                    }
                    break;
                case 'd': //Wait
                    if (pState == INNORMALENTRY)
                    {
                        curChar = *contents++; i++;
                        char fmtchar = SingleHexCharToBinary(curChar);
                        if (fmtchar == -1)
                        {
                            printf(invalidParamMessage, filename, lineNum, "@d");
                            continue;
                        }
                        fmtchar |= 0x60;
                        isCommandTextInject = true;
                        commandTextInject[0] = '\x1B';
                        commandTextInject[1] = fmtchar;
                        commandTextInject[2] = '\0';
                    }
                    else if (pState != NOTINENTRY)
                    {
                        printf(invalidAtCommandMessage, filename, lineNum);
                    }
                    break;
                case 'w': //Set wait between characters
                    if (pState == INNORMALENTRY)
                    {
                        curChar = *contents++; i++;
                        char fmtchar = SingleHexCharToBinary(curChar);
                        if (fmtchar == -1)
                        {
                            printf(invalidParamMessage, filename, lineNum, "@w");
                            continue;
                        }
                        fmtchar |= 0x70;
                        isCommandTextInject = true;
                        commandTextInject[0] = '\x1B';
                        commandTextInject[1] = fmtchar;
                        commandTextInject[2] = '\0';
                    }
                    else if (pState != NOTINENTRY)
                    {
                        printf(invalidAtCommandMessage, filename, lineNum);
                    }
                    break;
                case 'r': //Reset format parameter
                    if (pState != NOTINENTRY)
                    {
                        curChar = *contents++; i++;
                        if (curChar != '(')
                        {
                            printf(missingParamMessage, filename, lineNum, "@r");
                            cheekyBreakr:
                            continue;
                        }
                        curChar = *contents++; i++;
                        bool invalidParam = false;
                        char fmtchar = 0xF0;
                        while (curChar != ')')
                        {
                            switch (curChar)
                            {
                            case 'f':
                                fmtchar |= 0x01;
                                break;
                            case 'm':
                                fmtchar |= 0x04;
                                break;
                            case 'c':
                                fmtchar |= 0x08;
                                break;
                            default:
                                printf(invalidParamMessage, filename, lineNum, "@f");
                                goto cheekyBreakr; //ok now this is utterly cursed, but it's faster!!!
                            }
                            curChar = *contents++; i++;
                        }
                        isCommandTextInject = true;
                        commandTextInject[0] = '\x1B';
                        commandTextInject[1] = fmtchar;
                        commandTextInject[2] = '\0';
                    }
                    break;
                default:
                    printf(invalidAtCommandMessage, filename, lineNum);
                    break;
            }
        }
        if (pState == INNORMALENTRY)
        {
            if (curChar == '\n')
            {
                lineNum++;
                if (stetb[-1] != '\r') //LF normalisation (MHVN expects CR LF)
                {
                    *stetb++ = '\r';
                    tlbufPtr++;
                }
                *stetb++ = curChar;
                tlbufPtr++;
            }
            else if (curChar == EOT)
            {
                if (tlbufPtr <= 0)
                {
                    printf(missingTextMessage, filename, lineNum);
                    pState = NOTINENTRY;
                    sceneTextEntryNamesBufPtr = curSceneTexts->entryNames[curSceneTexts->numEntries];
                    break;
                }
                *stetb++ = '\0';
                curSceneTexts->numEntries++;
                pState = NOTINENTRY;
                tlbufPtr = 0;
            }
            else if (isCommandTextInject)
            {
                curChar = commandTextInject[ctInjectPtr];
                while (curChar != '\0')
                {
                    if (curChar == '\n' && commandTextInject[ctInjectPtr - 1] != '\r') //LF normalisation (MHVN expects CR LF)
                    {
                        *stetb++ = '\r';
                        tlbufPtr++;
                    }
                    *stetb++ = curChar;
                    ctInjectPtr++;
                    curChar = commandTextInject[ctInjectPtr];
                    tlbufPtr++;
                }
                ctInjectPtr = 0;
                isCommandTextInject = false;
            }
            else
            {
                *stetb++ = curChar;
                tlbufPtr++;
            }
        }
        else if (pState == INCHARNAMEENTRY)
        {
            if (curChar == '\n' || curChar == EOT)
            {
                charnameNewline:
                lineNum++;
                if (tlbufPtr == 0)
                {
                    printf(missingTextMessage, filename, lineNum);
                    pState = NOTINENTRY;
                    charLabelsBufPtr = charNames[numChars].label;
                    continue;
                }
                else if (curChar == '\n' && cnameb[-1] == '\r') //LF normalisation
                {
                    cnameb[-1] = '\0';
                }
                else
                {
                    *cnameb++ = '\0';
                    tlbufPtr++;
                }
                numChars++;
                tlbufPtr = 0;
                pState = NOTINENTRY;
                isCommandTextInject = false;
            }
            else if (isCommandTextInject)
            {
                curChar = commandTextInject[ctInjectPtr];
                while (curChar != '\0')
                {
                    if (curChar == '\n')
                    {
                        ctInjectPtr = 0;
                        goto charnameNewline; //fuck you goto haters
                    }
                    *cnameb++ = curChar;
                    ctInjectPtr++;
                    curChar = commandTextInject[ctInjectPtr];
                    tlbufPtr++;
                }
                ctInjectPtr = 0;
                isCommandTextInject = false;
            }
            else
            {
                *cnameb++ = curChar;
                tlbufPtr++;
            }
        }
        else if (curChar == '\n') //Line feeds need to be accounted for outside of entries as well (for the purpose of informing the user of the proper position of syntax errors)
        {
            lineNum++;
        }
    }
    return 0;
}

int ArchiveText(const char* outputFilename, const char** inputFilenames, const int numInputFiles)
{
    unsigned char* inputFileContents;
    int numInvalidFiles = 0;
    FILE* outputFileHandle = fopen(outputFilename, "wb");
    if (outputFileHandle == NULL)
    {
        printf("CRITICAL ERROR - Output file %s could not be opened!", outputFilename);
        return 1;
    }

    charLabelsBufferAllocSize = DEFAULT_ALLOCBLOCK_CLB;
    charLabelsBuffer = malloc(charLabelsBufferAllocSize);
    charLabelsBufPtr = charLabelsBuffer;
    charNamesBufferAllocSize = DEFAULT_ALLOCBLOCK_CNB;
    charNamesBuffer = malloc(charNamesBufferAllocSize);
    charNamesBufPtr = charNamesBuffer;
    sceneNamesBufferAllocSize = DEFAULT_ALLOCBLOCK_SNB;
    sceneNamesBuffer = malloc(sceneNamesBufferAllocSize);
    sceneNamesBufPtr = sceneNamesBuffer;
    sceneTextEntryNamesBufferAllocSize = DEFAULT_ALLOCBLOCK_STENB;
    sceneTextEntryNamesBuffer = malloc(sceneTextEntryNamesBufferAllocSize);
    sceneTextEntryNamesBufPtr = sceneTextEntryNamesBuffer;
    sceneTextEntryTextsBufferAllocSize = DEFAULT_ALLOCBLOCK_STETB;
    sceneTextEntryTextsBuffer = malloc(sceneTextEntryTextsBufferAllocSize);
    sceneTextEntryTextsBufPtr = sceneTextEntryTextsBuffer;

    for (int i = 0; i < numInputFiles; i++)
    {
        //Keep only one file's contents in memory at any given time to manage memory size (for safety I guess)
        FILE* inputFileHandle = fopen(inputFilenames[i], "rb");
        if (inputFileHandle == NULL)
        {
            printf("ERROR - Input file %s is invalid! Skipping over it.", inputFilenames[i]);
            numInvalidFiles++;
            continue;
        }
        fseek(inputFileHandle, 0, SEEK_END); //Hopefully should set the file pointer to the end of the file
        const long fileLen = ftell(inputFileHandle) + 1; //So we can get the file's length!
        fseek(inputFileHandle, 0, SEEK_SET);
        inputFileContents = malloc(fileLen);
        inputFileContents[fileLen - 1] = EOT; //Put EOT character in buffer so that the parser knows when the file has ended
        fread(inputFileContents, 1, fileLen, inputFileHandle);
        fclose(inputFileHandle);

        int result = ParseInputFile(inputFileContents, fileLen, inputFilenames[i]);
        free(inputFileContents);
    }

    if (numInvalidFiles >= numInputFiles)
    {
        printf("CRITICAL ERROR - All input files referenced are invalid!");
        fclose(outputFileHandle);
        remove(outputFilename);
        free(charLabelsBuffer);
        free(charNamesBuffer);
        free(sceneNamesBuffer);
        free(sceneTextEntryNamesBuffer);
        free(sceneTextEntryTextsBuffer);
        return 1;
    }

    //All initialised to point immediately after the header in which they're contained
    uint32_t systextPtr = 0x00000018;
    uint32_t creditsPtr = 0x00000018;
    uint32_t charnamesPtr = 0x00000018;
    uint32_t scenetextPtr = 0x00000018;
    uint32_t cgtextPtr = 0x00000018;
    uint32_t musictextPtr = 0x00000018;

    txaFileOutputBufferAllocSize = DEFAULT_ALLOCBLOCK_TXA;
    txaFileOutputBuffer = malloc(txaFileOutputBufferAllocSize);
    txaFileOutputBufPtr = txaFileOutputBuffer;
    linkInfoOutputBufferAllocSize = DEFAULT_ALLOCBLOCK_LINK;
    linkInfoOutputBuffer = malloc(linkInfoOutputBufferAllocSize);
    linkInfoOutputBufPtr = linkInfoOutputBuffer;

    //Write character names
    charnamesPtr = (intptr_t)txaFileOutputBufPtr - (intptr_t)txaFileOutputBuffer + 0x00000018;
    uint16_t* tempPtr1 = (uint16_t*)txaFileOutputBufPtr;
    txaFileOutputBufPtr += 2 * numChars;
    uint16_t curRealPtr1 = 0;
    for (int i = 0; i < numChars; i++)
    {
        tempPtr1[i] = curRealPtr1;
        char* namePtr = charNames[i].name;
        char curChar = '!';
        while (curChar)
        {
            curChar = *namePtr++;
            *txaFileOutputBufPtr++ = curChar;
            curRealPtr1++;
        }
    }

    //Write scene text
    scenetextPtr = (intptr_t)txaFileOutputBufPtr - (intptr_t)txaFileOutputBuffer + 0x00000018;
    uint32_t* tempPtr2 = (uint32_t*)txaFileOutputBufPtr;
    txaFileOutputBufPtr += 4 * numScenes;
    uint32_t curRealPtr2 = 0;
    for (int i = 0; i < numScenes; i++)
    {
        tempPtr2[i] = curRealPtr2;
        unsigned int curNumEntries = scenes[i].numEntries;
        tempPtr1 = (uint16_t*)txaFileOutputBufPtr;
        *tempPtr1++ = curNumEntries;
        txaFileOutputBufPtr += 2 * curNumEntries + 2;
        curRealPtr1 = 0;
        curRealPtr2 += 2 * curNumEntries + 2;
        for (int j = 0; j < curNumEntries; j++)
        {
            tempPtr1[j] = curRealPtr1;
            char* textPtr = scenes[i].entryTexts[j];
            char curChar = '!';
            while (curChar)
            {
                curChar = *textPtr++;
                *txaFileOutputBufPtr++ = curChar;
                curRealPtr1++;
                curRealPtr2++;
            }
        }
    }

    size_t sizeOfTXA = (intptr_t)txaFileOutputBufPtr - (intptr_t)txaFileOutputBuffer;
    uint64_t linkInfoPtr = 0x00000008 + sizeOfTXA + 0x00000018;

    //Write link info
    tempPtr1 = (uint16_t*)linkInfoOutputBufPtr;
    tempPtr1[0] = (uint16_t)numChars;
    tempPtr1[1] = (uint16_t)numScenes;
    linkInfoOutputBufPtr += 4;
    for (int i = 0; i < numChars; i++)
    {
        char* namePtr = charNames[i].label;
        char curChar = '!';
        while (curChar)
        {
            curChar = *namePtr++;
            *linkInfoOutputBufPtr++ = curChar;
        }
    }
    uint64_t* tempPtr3 = (uint64_t*)linkInfoOutputBufPtr;
    linkInfoOutputBufPtr += 8 * numScenes;
    uint64_t curRealPtr3 = 0;
    for (int i = 0; i < numScenes; i++)
    {
        char* namePtr = scenes[i].sceneName;
        char curChar = '!';
        while (curChar)
        {
            curChar = *namePtr++;
            *linkInfoOutputBufPtr++ = curChar;
        }
    }
    for (int i = 0; i < numScenes; i++)
    {
        tempPtr1 = (uint16_t*)linkInfoOutputBufPtr;
        SceneText* curSceneInf = &scenes[i];
        int nEnt = curSceneInf->numEntries;
        tempPtr1[0] = (uint16_t)nEnt;
        *tempPtr3++ = curRealPtr3;
        linkInfoOutputBufPtr += 2;
        curRealPtr3 += 2;
        for (int j = 0; j < nEnt; j++)
        {
            char* namePtr = curSceneInf->entryNames[j];
            char curChar = '!';
            while (curChar)
            {
                curChar = *namePtr++;
                *linkInfoOutputBufPtr++ = curChar;
                curRealPtr3++;
            }
        }
    }

    size_t sizeOfLinkInfo = (intptr_t)linkInfoOutputBufPtr - (intptr_t)linkInfoOutputBuffer;

    fwrite(&linkInfoPtr, sizeof(uint64_t), 1, outputFileHandle);
    fwrite(&systextPtr, sizeof(uint32_t), 1, outputFileHandle);
    fwrite(&creditsPtr, sizeof(uint32_t), 1, outputFileHandle);
    fwrite(&charnamesPtr, sizeof(uint32_t), 1, outputFileHandle);
    fwrite(&scenetextPtr, sizeof(uint32_t), 1, outputFileHandle);
    fwrite(&cgtextPtr, sizeof(uint32_t), 1, outputFileHandle);
    fwrite(&musictextPtr, sizeof(uint32_t), 1, outputFileHandle);
    fwrite(txaFileOutputBuffer, 1, sizeOfTXA, outputFileHandle);
    fwrite(linkInfoOutputBuffer, 1, sizeOfLinkInfo, outputFileHandle);
    fclose(outputFileHandle);
    for (int i = 0; i < numScenes; i++)
    {
        free(scenes[i].entryNames);
        free(scenes[i].entryTexts);
    }
    free(linkInfoOutputBuffer);
    free(txaFileOutputBuffer);
    free(charLabelsBuffer);
    free(charNamesBuffer);
    free(sceneNamesBuffer);
    free(sceneTextEntryNamesBuffer);
    free(sceneTextEntryTextsBuffer);
    return 0;
}
