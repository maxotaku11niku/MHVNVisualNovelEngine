/* MHVNSCAS - Scene assembler for the MHVN Visual Novel Engine
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
 * Main assembler code
 */

extern "C"
{
    #include <lz4hc.h>
}

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unordered_map>
#include <string>
#include <vector>

#define EOT ((char)0x04)

#define OP_INSTRUCTION 0x00000000
#define OP_DIRECTIVE 0x00010000
#define OP_JUMPLABEL 0x00020000
#define OP_INVALIDOP -1
#define OP_SCENEUNDEFINED -2

//Initial allocation sizes, and also how many more bytes/entries to add if more memory needs to be allocated for some reason
#define DEFAULT_ALLOCBLOCK_SNB   65536
#define DEFAULT_ALLOCBLOCK_STENB 65536
#define DEFAULT_ALLOCBLOCK_SDB   65536
#define DEFAULT_ALLOCBLOCK_CLB   65536
#define DEFAULT_ALLOCBLOCK_SEP   256
#define DEFAULT_ALLOCBLOCK_BGLB  65536
#define DEFAULT_ALLOCBLOCK_BGVLB 65536
#define DEFAULT_ALLOCBLOCK_SPLB  65536
#define DEFAULT_ALLOCBLOCK_SPVLB 65536

//This opcode map also accounts for pseudoinstructions by going beyond a byte
const std::unordered_map<std::string, int> mnemonicsToOpcodes =
{
    {std::string("gotoscene"),      0x00},
    {std::string("vnend"),         0x100},
    {std::string("jmp"),            0x01},
    {std::string("jz"),             0x02},
    {std::string("je"),             0x02},
    {std::string("jnz"),            0x03},
    {std::string("jne"),            0x03},
    {std::string("jn"),             0x04},
    {std::string("jl"),             0x04},
    {std::string("jp"),             0x05},
    {std::string("jge"),            0x05},
    {std::string("jle"),            0x06},
    {std::string("jg"),             0x07},
    {std::string("palsetcol"),      0x08},
    {std::string("paladdcol"),      0x09},
    {std::string("palsetlum"),      0x0A},
    {std::string("palsetsat"),      0x0B},
    {std::string("palsethue"),      0x0C},
    {std::string("palcolourise"),   0x0D},
    {std::string("palinvert"),      0x0E},
    {std::string("nowait"),         0x0F},
    {std::string("nexttext"),       0x10},
    {std::string("text"),           0x11},
    {std::string("charname"),       0x12},
    {std::string("nocharname"),    0x112},
    {std::string("deltext"),        0x13},
    {std::string("ynchoice"),       0x14},
    {std::string("choice2"),        0x15},
    {std::string("choice3"),        0x16},
    {std::string("choice4"),        0x17},
    {std::string("bfadein"),        0x18},
    {std::string("bfadeout"),       0x19},
    {std::string("wfadein"),        0x1A},
    {std::string("wfadeout"),       0x1B},
    {std::string("pfadein"),        0x1C},
    {std::string("pfadeout"),       0x1D},
    {std::string("phuerotate"),     0x1E},
    {std::string("shake"),          0x1F},
    {std::string("lut2"),           0x20},
    {std::string("lut3"),           0x21},
    {std::string("lut4"),           0x22},
    {std::string("swapzn"),         0x23},
    {std::string("setvi"),          0x24},
    {std::string("setvv"),          0x25},
    {std::string("csetvi"),         0x26},
    {std::string("csetvv"),         0x27},
    {std::string("cmpvi"),          0x28},
    {std::string("cmpvv"),          0x29},
    {std::string("addvi"),          0x2A},
    {std::string("addvv"),          0x2B},
    {std::string("subvi"),          0x2C},
    {std::string("subvv"),          0x2D},
    {std::string("ldflg"),          0x2E},
    {std::string("stflg"),          0x2F},
    {std::string("setbg"),          0x30},
    {std::string("addbgvar"),       0x31},
    {std::string("subbgvar"),       0x32},
    {std::string("subbgvarall"),   0x132},
    {std::string("setspr0"),        0x34},
    {std::string("unsetspr0"),     0x134},
    {std::string("addspr0var"),     0x35},
    {std::string("subspr0var"),     0x36},
    {std::string("subspr0varall"), 0x136},
    {std::string("setspr1"),        0x38},
    {std::string("unsetspr1"),     0x138},
    {std::string("addspr1var"),     0x39},
    {std::string("subspr1var"),     0x3A},
    {std::string("subspr1varall"), 0x13A},
    {std::string("setspr2"),        0x3C},
    {std::string("unsetspr2"),     0x13C},
    {std::string("addspr2var"),     0x3D},
    {std::string("subspr2var"),     0x3E},
    {std::string("subspr2varall"), 0x13E},
    //Directives included as well for ease of programming
    {std::string(".scene"),     0x10000},
    {std::string(".vnentry"),   0x10001},
    {std::string(".globvar"),   0x10002},
    {std::string(".globflag"),  0x10003},
    {std::string(".localvar"),  0x10004},
    {std::string(".localflag"), 0x10005},
};

const std::unordered_map<std::string, int> defaultVarNames =
{
    {std::string("r0"),  0x0000},
    {std::string("r1"),  0x0001},
    {std::string("r2"),  0x0002},
    {std::string("r3"),  0x0003},
    {std::string("r4"),  0x0004},
    {std::string("r5"),  0x0005},
    {std::string("r6"),  0x0006},
    {std::string("r7"),  0x0007},
    {std::string("f0"),  0x0008},
    {std::string("f1"),  0x0009},
    {std::string("f2"),  0x000A},
    {std::string("f3"),  0x000B},
    {std::string("f4"),  0x000C},
    {std::string("f5"),  0x000D},
    {std::string("f6"),  0x000E},
    {std::string("f7"),  0x000F},
    {std::string("f8"),  0x0010},
    {std::string("f9"),  0x0011},
    {std::string("f10"), 0x0012},
    {std::string("f11"), 0x0013},
    {std::string("f12"), 0x0014},
    {std::string("f13"), 0x0015},
    {std::string("f14"), 0x0016},
    {std::string("f15"), 0x0017},
    {std::string("f16"), 0x0018},
    {std::string("f17"), 0x0019},
    {std::string("f18"), 0x001A},
    {std::string("f19"), 0x001B},
    {std::string("f20"), 0x001C},
    {std::string("f21"), 0x001D},
    {std::string("f22"), 0x001E},
    {std::string("f23"), 0x001F}
};

typedef struct
{
    char* label;
    unsigned char* data;
    char** textNames;
    int numTexts;
    int dataLen;
    std::unordered_map<int, std::string>* refSceneLabels;
    std::unordered_map<int, std::string>* refVarLabels;
    std::unordered_map<std::string, int>* foundTextLabels;
} SceneData;

typedef struct
{
    char* label;
    char** variantNames;
    int numVariants;
    std::unordered_map<std::string, int>* foundVariantLabels;
} ImageData;

SceneData scenes[65536];
int numScenes;
int currentEntryScene;
char* charLabels[65536];
int numChars;
bool textChain;
char* globVarLabels[224];
char* globFlagLabels[768];
char* localVarLabels[512];
char* localFlagLabels[2560];
int numGlobVars;
int numGlobFlags;
int numLocalVars;
int numLocalFlags;
ImageData bg[65536];
int numBg;
ImageData spr[65536];
int numSpr;

std::unordered_map<std::string, int>* foundJumpLabels;
std::vector<std::pair<int, std::string>>* refJumpLabels;
std::unordered_map<std::string, int>* foundSceneLabels;
std::unordered_map<std::string, int>* foundCharLabels;
std::unordered_map<std::string, int>* foundVarLabels;
std::unordered_map<std::string, int>* foundBgLabels;
std::unordered_map<std::string, int>* foundSprLabels;

char* sceneNamesBuffer;
char* sceneNamesBufPtr;
size_t sceneNamesBufferAllocSize;
char* sceneTextEntryNamesBuffer;
char* sceneTextEntryNamesBufPtr;
size_t sceneTextEntryNamesBufferAllocSize;
unsigned char* sceneDataBuffer;
unsigned char* sceneDataBufPtr;
size_t sceneDataBufferAllocSize;
char* charLabelsBuffer;
char* charLabelsBufPtr;
size_t charLabelsBufferAllocSize;
char* bgLabelsBuffer;
char* bgLabelsBufPtr;
size_t bgLabelsBufferAllocSize;
char* bgVariantLabelsBuffer;
char* bgVariantLabelsBufPtr;
size_t bgVariantLabelsBufferAllocSize;
char* sprLabelsBuffer;
char* sprLabelsBufPtr;
size_t sprLabelsBufferAllocSize;
char* sprVariantLabelsBuffer;
char* sprVariantLabelsBufPtr;
size_t sprVariantLabelsBufferAllocSize;

const char* invalidOpMessage = "ERROR - %s, line %u: invalid operation. Line ignored.";
const char* autoOpMessage = "ERROR - %s, line %u: operation is not valid for use in assembly files. Remember that the assembler generates this instruction as part of optimisation. Line ignored";
const char* sceneUndefinedMessage = "ERROR - %s, line %u: attempted to use operations without declaring the scene first. File ignored.";

int GetOpcode(const char* firstWordOfLine)
{
    std::string wordstr = std::string(firstWordOfLine);
    int returnstat = OP_INVALIDOP;
    if (wordstr.back() == ':') //Colon at end -> jump label
    {
        returnstat = OP_JUMPLABEL;
    }
    else if (mnemonicsToOpcodes.count(wordstr))
    {
        returnstat = mnemonicsToOpcodes.at(wordstr);
    }
    return returnstat;
}

#define SCANSTAT_WORD 0x1
#define SCANSTAT_ARG 0x2
#define SCANSTAT_COMMENT 0x4

int ScanForWord(const char** start, const char** end)
{
    const char* curChPtr = *start;
    const char* wordPtr = curChPtr;
    char ch = *curChPtr++;
    int lenArg = 0;
    bool foundComment = false;
    bool foundWord = false;
    while(ch)
    {
        //Ignore whitespace and also consider whitespace to end a mnemonic. ';' begins comments.
        if (ch == ' ' || ch == '\t' || ch == ';')
        {
            if (ch == ';') foundComment = true;
            if (foundWord)
            {
                break;
            }
            wordPtr = curChPtr;
        }
        else
        {
            foundWord = true;
            lenArg++;
        }
        ch = *curChPtr++;
        if (foundComment) break;
    }
    *start = wordPtr;
    *end = curChPtr - 1;
    if (lenArg)
    {
        foundWord = true;
    }
    int returnStat = foundWord ? SCANSTAT_WORD : 0x0;
    returnStat |= foundComment ? SCANSTAT_COMMENT: 0x0;
    return returnStat;
}

int ScanForArgument(const char** start, const char** end)
{
    const char* curChPtr = *start;
    const char* wordPtr = curChPtr;
    char ch = *curChPtr++;
    int lenArg = 0;
    bool foundComment = false;
    bool foundWord = false;
    bool foundArgument = false;
    while(ch)
    {
        //Ignore whitespace and also consider whitespace to end a mnemonic. ',' separates arguments, ';' begins comments.
        if (ch == ',' || ch == ' ' || ch == '\t' || ch == ';')
        {
            if (ch == ';') foundComment = true;
            if (foundWord)
            {
                if (ch == ',') foundArgument = true;
                break;
            }
            wordPtr = curChPtr;
        }
        else
        {
            foundWord = true;
            lenArg++;
        }
        ch = *curChPtr++;
        if (foundComment) break;
    }
    *start = wordPtr;
    *end = curChPtr - 1;
    if (lenArg)
    {
        foundWord = true;
    }
    int returnStat = foundWord ? SCANSTAT_WORD : 0x0;
    returnStat |= foundComment ? SCANSTAT_COMMENT : 0x0;
    returnStat |= foundArgument ? SCANSTAT_ARG : 0x0;

    return returnStat;
}

int GetStateVarNum(const char* name)
{
    int output = 0;
    std::string namestr = std::string(name);
    if (foundVarLabels->count(namestr))
    {
        output = foundVarLabels->at(namestr);
    }
    else output = -1; //State variable not found
    return output;
}

typedef enum
{
    NOOPERANDS, //Operation takes no operands
    SCENEREF, //Operation takes a single scene reference
    BGREF, //Operation takes a single background reference
    BGVARREF, //Operation takes a single background variant reference
    SPRREF, //Operation takes a single sprite reference
    SPRVARREF, //Operation takes a single sprite variant reference
    TEXTREF, //Operation takes a single text reference
    CHARACTERREF, //Operation takes a single character reference
    JUMP, //Operation takes a single jump label
    UINT8, //Operation takes a single 8-bit unsigned integer
    INT8, //Operation takes a single 8-bit signed integer
    UINT5_3, //Operation takes 3 5-bit unsigned integers
    INT5_3, //Operation takes 3 5-bit signed integers
    UINT3_UINT5, //Operation takes a 3-bit unsigned integer and a 5-bit unsigned integer
    UINT6_UINT5_UINT5, //Operation takes a 6-bit unsigned integer and two 5-bit unsigned integers
    STATEVAR_SINGLE, //Operation takes a single state variable reference
    STATEVAR_DOUBLE, //Operation takes two state variable references
    STATEVAR_SINGLE_IMMEDIATE, //Operation takes a single state variable reference and an immediate
    MULTICHOICE, //Operation is a multiple choice instruction
    LUT //Operation is a LUT instruction
} OperandType;

int ParseLine(char* line, int* curScene)
{
    char* wordptr = line;
    char* wordEndPtr;
    int locCounter = 0;
    int scene = *curScene;
    SceneData* curScDat;
    if (scene >= 0)
    {
        curScDat = &scenes[scene];
        locCounter = curScDat->dataLen;
    }
    int numArg = 0;
    int opcode = OP_INVALIDOP;
    OperandType type = OperandType::NOOPERANDS;
    unsigned char instructionBytes[32];
    int numBytesInInstruction = 0;
    bool requiresSceneDef = true;
    bool foundOperation = false;
    bool foundComment = false;
    //Find mnemonic on this line
    int scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
    char* jumpLabelPtr = wordptr;
    *wordEndPtr = '\0';
    opcode = GetOpcode(wordptr);
    foundOperation = scanStat & SCANSTAT_WORD;
    foundComment = scanStat & SCANSTAT_COMMENT;
    instructionBytes[0] = (unsigned char)(opcode & 0xFF);
    numBytesInInstruction++;
    switch (opcode)
    {
        case 0x00: //gotoscene
            numArg = 1;
            type = OperandType::SCENEREF;
            break;
        case 0x100: //vnend
            instructionBytes[1] = 0xFF;
            instructionBytes[2] = 0xFF;
            numBytesInInstruction += 2;
            numArg = 0;
            break;
        case 0x01: //jmp
        case 0x02: //jz, je
        case 0x03: //jnz, jne
        case 0x04: //jn, jl
        case 0x05: //jp, jge
        case 0x06: //jle
        case 0x07: //jg
            numArg = 1;
            type = OperandType::JUMP;
            break;
        case 0x08: //palsetcol
            numArg = 3;
            type = OperandType::UINT5_3;
            break;
        case 0x09: //paladdcol
            numArg = 3;
            type = OperandType::INT5_3;
            break;
        case 0x0A: //palsetlum
            numArg = 1;
            type = OperandType::INT8;
            break;
        case 0x0B: //palsetsat
            numArg = 1;
            type = OperandType::UINT8;
            break;
        case 0x0C: //palsethue
            numArg = 1;
            type = OperandType::INT8;
            break;
        case 0x0D: //palcolourise
            numArg = 3;
            type = OperandType::UINT5_3;
            break;
        case 0x0E: //palinvert
            numArg = 0;
            break;
        case 0x0F: //nowait
            numArg = 0;
            break;
        case 0x10: //nexttext
            break;
        case 0x11: //text
            numArg = 1;
            type = OperandType::TEXTREF;
            break;
        case 0x12: //charname
            numArg = 1;
            type = OperandType::CHARACTERREF;
            break;
        case 0x112: //nocharname
            instructionBytes[1] = 0xFF;
            instructionBytes[2] = 0xFF;
            numBytesInInstruction += 2;
            numArg = 0;
            break;
        case 0x13: //deltext
            numArg = 0;
            break;
        case 0x14: //ynchoice
            numArg = 0;
            break;
        case 0x15: //choice2
            numArg = 3;
            type = OperandType::MULTICHOICE;
            break;
        case 0x16: //choice3
            numArg = 4;
            type = OperandType::MULTICHOICE;
            break;
        case 0x17: //choice4
            numArg = 5;
            type = OperandType::MULTICHOICE;
            break;
        case 0x18: //bfadein
        case 0x19: //bfadeout
        case 0x1A: //wfadein
        case 0x1B: //wfadeout
        case 0x1C: //pfadein
        case 0x1D: //pfadeout
            numArg = 2;
            type = OperandType::UINT3_UINT5;
            break;
        case 0x1E: //phuerotate
            numArg = 1;
            type = OperandType::UINT8;
            break;
        case 0x1F: //shake
            numArg = 3;
            type = OperandType::UINT6_UINT5_UINT5;
            break;
        case 0x20: //lut2
            numArg = 3;
            type = OperandType::LUT;
            break;
        case 0x21: //lut3
            numArg = 4;
            type = OperandType::LUT;
            break;
        case 0x22: //lut4
            numArg = 5;
            type = OperandType::LUT;
            break;
        case 0x23: //swapzn
            numArg = 0;
            break;
        case 0x24: //setvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x25: //setvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        case 0x26: //csetvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x27: //csetvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        case 0x28: //cmpvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x29: //cmpvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        case 0x2A: //addvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x2B: //addvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        case 0x2C: //subvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x2D: //subvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        case 0x2E: //ldflg
            numArg = 1;
            type = OperandType::STATEVAR_SINGLE;
            break;
        case 0x2F: //stflg
            numArg = 1;
            type = OperandType::STATEVAR_SINGLE;
            break;
        case 0x30: //setbg
            numArg = 1;
            type = OperandType::BGREF;
            break;
        case 0x31: //addbgvar
        case 0x32: //subbgvar
            numArg = 2;
            type = OperandType::BGVARREF;
            break;
        case 0x132: //subbgvarall
            instructionBytes[1] = 0xFF;
            instructionBytes[2] = 0xFF;
            numBytesInInstruction += 2;
            numArg = 0;
            break;
        case 0x34: //setspr0
        case 0x38: //setspr1
        case 0x3C: //setspr2
            numArg = 1;
            type = OperandType::SPRREF;
            break;
        case 0x134: //unsetspr0
        case 0x138: //unsetspr1
        case 0x13C: //unsetspr2
            instructionBytes[1] = 0xFF;
            instructionBytes[2] = 0xFF;
            numBytesInInstruction += 2;
            numArg = 0;
            break;
        case 0x35: //addspr0var
        case 0x36: //subspr0var
        case 0x39: //addspr1var
        case 0x3A: //subspr1var
        case 0x3D: //addspr2var
        case 0x3E: //subspr2var
            numArg = 2;
            type = OperandType::SPRVARREF;
            break;
        case 0x136: //subspr0varall
        case 0x13A: //subspr1varall
        case 0x13E: //subspr2varall
            instructionBytes[1] = 0xFF;
            instructionBytes[2] = 0xFF;
            numBytesInInstruction += 2;
            numArg = 0;
            break;
        
        case 0x10000: //.scene
            numArg = 1;
            type = OperandType::SCENEREF;
            requiresSceneDef = false;
            break;
        case 0x10001: //.vnentry
            numArg = 0;
            if (scene >= 0) currentEntryScene = scene;
            break;
        case 0x10002: //.globvar
            numArg = 1;
            type = OperandType::STATEVAR_SINGLE;
            requiresSceneDef = false;
            break;
        case 0x10003: //.globflag
            numArg = 1;
            type = OperandType::STATEVAR_SINGLE;
            requiresSceneDef = false;
            break;
        case 0x10004: //.localvar
            numArg = 1;
            type = OperandType::STATEVAR_SINGLE;
            requiresSceneDef = false;
            break;
        case 0x10005: //.localflag
            numArg = 1;
            type = OperandType::STATEVAR_SINGLE;
            requiresSceneDef = false;
            break;
        case OP_JUMPLABEL:
            numArg = 1;
            break;
        default:
        case OP_INVALIDOP:
            break;
    }

    //No operation found -> empty line -> ignore it without raising an error
    if (!foundOperation) return 0;

    //Tried to use an operation that requires a scene without declaring a scene first -> error, stop parsing file
    if (scene < 0 && requiresSceneDef)
    {
        return OP_SCENEUNDEFINED;
    }

    //Invalid opcode -> error, ignore line
    if (opcode == OP_INVALIDOP || opcode == 0x10)
    {
        return opcode;
    }

    wordptr = wordEndPtr + 1;

    int nT;
    if (scene >= 0)
    {
        nT = curScDat->numTexts;
    }
    int arg0;
    int arg1;
    int arg2;
    char* pcch;
    char cch;
    switch (opcode & 0xFFFF0000)
    {
        case OP_INSTRUCTION: //Actual instruction that is being stored as scene bytecode
            locCounter++;
            switch (type)
            {
                case OperandType::NOOPERANDS: //No operands means nothing else to do
                    break;
                case OperandType::SCENEREF: //Resolve scene references after parsing all scene files (we'll just put a reference to the current scene in for now)
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(scene & 0x00FF);
                    instructionBytes[2] = (unsigned char)((scene & 0xFF00) >> 8);
                    break;
                case OperandType::BGREF: //BG references are assigned here
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    if (foundBgLabels->count(std::string(wordptr))) //Background name already exists
                    {
                        arg0 = (*foundBgLabels)[std::string(wordptr)];
                    }
                    else //Background name doesn't already exist
                    {
                        arg0 = numBg;
                        (*foundBgLabels)[std::string(wordptr)] = arg0;
                        bg[numBg].label = bgLabelsBufPtr;
                        bg[numBg].numVariants = 0;
                        bg[numBg].foundVariantLabels = new std::unordered_map<std::string, int>();
                        pcch = wordptr;
                        cch = 0xFF;
                        while (cch)
                        {
                            cch = *pcch++;
                            *bgLabelsBufPtr++ = cch;
                        }
                        numBg++;
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    break;
                case OperandType::BGVARREF: //BG variant references are assigned here (put a stub for now, TODO)
                    numBytesInInstruction += 2;
                    instructionBytes[1] = 0x00;
                    instructionBytes[2] = 0x00;
                    break;
                case OperandType::SPRREF: //Sprite references are assigned here
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    if (foundSprLabels->count(std::string(wordptr))) //Sprite name already exists
                    {
                        arg0 = (*foundSprLabels)[std::string(wordptr)];
                    }
                    else //Sprite name doesn't already exist
                    {
                        arg0 = numSpr;
                        (*foundSprLabels)[std::string(wordptr)] = arg0;
                        spr[numSpr].label = sprLabelsBufPtr;
                        spr[numSpr].numVariants = 0;
                        spr[numSpr].foundVariantLabels = new std::unordered_map<std::string, int>();
                        pcch = wordptr;
                        cch = 0xFF;
                        while (cch)
                        {
                            cch = *pcch++;
                            *sprLabelsBufPtr++ = cch;
                        }
                        numSpr++;
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    break;
                case OperandType::SPRVARREF: //Sprite variant references are assigned here (put a stub for now, TODO)
                    numBytesInInstruction += 2;
                    instructionBytes[1] = 0x00;
                    instructionBytes[2] = 0x00;
                    break;
                case OperandType::TEXTREF: //Text references are assigned here
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    if (textChain) //Consecutive 'text' instructions with no intervening jump labels are converted into 'nexttext' instructions which are more space efficient
                    {
                        instructionBytes[0] = 0x10;
                    }
                    else //If we can't chain, just use the normal 'text' instruction encoding
                    {
                        numBytesInInstruction += 2;
                        instructionBytes[1] = (unsigned char)(nT & 0x00FF);
                        instructionBytes[2] = (unsigned char)((nT & 0xFF00) >> 8);
                    }
                    curScDat->textNames[nT] = sceneTextEntryNamesBufPtr;
                    pcch = wordptr;
                    cch = 0xFF;
                    while (cch)
                    {
                        cch = *pcch++;
                        *sceneTextEntryNamesBufPtr++ = cch;
                    }
                    curScDat->numTexts++;
                    textChain = true;
                    break;
                case OperandType::CHARACTERREF: //Character references are assigned here
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    if (foundCharLabels->count(std::string(wordptr))) //Character name already exists
                    {
                        arg0 = (*foundCharLabels)[std::string(wordptr)];
                    }
                    else //Character name doesn't already exist
                    {
                        arg0 = numChars;
                        (*foundCharLabels)[std::string(wordptr)] = arg0;
                        charLabels[numChars] = charLabelsBufPtr;
                        pcch = wordptr;
                        cch = 0xFF;
                        while (cch)
                        {
                            cch = *pcch++;
                            *charLabelsBufPtr++ = cch;
                        }
                        numChars++;
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    break;
                case OperandType::JUMP: //Resolve jumps after parsing all lines (we'll just put a dummy value in for now)
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    refJumpLabels->push_back(std::pair<int, std::string>(locCounter, std::string(wordptr)));
                    numBytesInInstruction += 2;
                    instructionBytes[1] = 0x00;
                    instructionBytes[2] = 0x00;
                    break;
                case OperandType::UINT8:
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = atoi(wordptr);
                    if (arg0 < 0) arg0 = 0;
                    else if (arg0 > 0xFF) arg0 = 0xFF;
                    numBytesInInstruction += 1;
                    instructionBytes[1] = (unsigned char)arg0;
                    break;
                case OperandType::INT8:
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = atoi(wordptr);
                    if (arg0 < -0x80) arg0 = -0x80;
                    else if (arg0 > 0x7F) arg0 = 0x7F;
                    numBytesInInstruction += 1;
                    instructionBytes[1] = (char)arg0;
                    break;
                case OperandType::UINT5_3:
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = atoi(wordptr);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg1 = atoi(wordptr);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg2 = atoi(wordptr);
                    if (arg0 < 0) arg0 = 0;
                    else if (arg0 > 0x1F) arg0 = 0x1F;
                    if (arg1 < 0) arg1 = 0;
                    else if (arg1 > 0x1F) arg1 = 0x1F;
                    if (arg2 < 0) arg2 = 0;
                    else if (arg2 > 0x1F) arg2 = 0x1F;
                    arg0 |= (arg1 << 5) | (arg2 << 10);
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0xFF);
                    instructionBytes[2] = (unsigned char)((arg0 >> 8) & 0xFF);
                    break;
                case OperandType::INT5_3:
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = atoi(wordptr);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg1 = atoi(wordptr);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg2 = atoi(wordptr);
                    if (arg0 < -0x10) arg0 = -0x10;
                    else if (arg0 > 0x0F) arg0 = 0x0F;
                    if (arg1 < -0x10) arg1 = -0x10;
                    else if (arg1 > 0x0F) arg1 = 0x0F;
                    if (arg2 < -0x10) arg2 = -0x10;
                    else if (arg2 > 0x0F) arg2 = 0x0F;
                    arg0 &= 0x1F; arg1 &= 0x1F; arg2 &= 0x1F;
                    arg0 |= (arg1 << 5) | (arg2 << 10);
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0xFF);
                    instructionBytes[2] = (unsigned char)((arg0 >> 8) & 0xFF);
                    break;
                case OperandType::UINT3_UINT5:
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = atoi(wordptr);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg1 = atoi(wordptr);
                    if (arg0 < 0) arg0 = 0;
                    else if (arg0 > 0x07) arg0 = 0x07;
                    if (arg1 < 0) arg1 = 0;
                    else if (arg1 > 0x1F) arg1 = 0x1F;
                    arg0 |= (arg1 << 3);
                    numBytesInInstruction += 1;
                    instructionBytes[1] = (unsigned char)arg0;
                    break;
                case OperandType::UINT6_UINT5_UINT5:
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = atoi(wordptr);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg1 = atoi(wordptr);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg2 = atoi(wordptr);
                    if (arg0 < 0) arg0 = 0;
                    else if (arg0 > 0x3F) arg0 = 0x3F;
                    if (arg1 < 0) arg1 = 0;
                    else if (arg1 > 0x1F) arg1 = 0x1F;
                    if (arg2 < 0) arg2 = 0;
                    else if (arg2 > 0x1F) arg2 = 0x1F;
                    arg0 |= (arg1 << 6) | (arg2 << 11);
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0xFF);
                    instructionBytes[2] = (unsigned char)((arg0 >> 8) & 0xFF);
                    break;
                case OperandType::STATEVAR_SINGLE:
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = GetStateVarNum(wordptr);
                    if (arg0 < 0)
                    {
                        arg0 = 0;
                        (*(curScDat->refVarLabels))[locCounter] = std::string(wordptr);
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    break;
                case OperandType::STATEVAR_DOUBLE:
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = GetStateVarNum(wordptr);
                    if (arg0 < 0)
                    {
                        arg0 = 0;
                        (*(curScDat->refVarLabels))[locCounter] = std::string(wordptr);
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = GetStateVarNum(wordptr);
                    if (arg0 < 0)
                    {
                        arg0 = 0;
                        (*(curScDat->refVarLabels))[locCounter + 2] = std::string(wordptr);
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[3] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[4] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    break;
                case OperandType::STATEVAR_SINGLE_IMMEDIATE:
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = GetStateVarNum(wordptr);
                    if (arg0 < 0)
                    {
                        arg0 = 0;
                        (*(curScDat->refVarLabels))[locCounter] = std::string(wordptr);
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = atoi(wordptr);
                    numBytesInInstruction += 2;
                    instructionBytes[3] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[4] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    break;
                case OperandType::MULTICHOICE: //TODO: make this able to rearrange text pointers should that be necessary
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = GetStateVarNum(wordptr);
                    if (arg0 < 0)
                    {
                        arg0 = 0;
                        (*(curScDat->refVarLabels))[locCounter] = std::string(wordptr);
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    numBytesInInstruction += 2;
                    instructionBytes[3] = (unsigned char)(nT & 0x00FF);
                    instructionBytes[4] = (unsigned char)((nT & 0xFF00) >> 8);
                    curScDat->textNames[nT] = sceneTextEntryNamesBufPtr;
                    pcch = wordptr;
                    cch = 0xFF;
                    while (cch)
                    {
                        cch = *pcch++;
                        *sceneTextEntryNamesBufPtr++ = cch;
                    }
                    curScDat->numTexts++;
                    nT++;
                    for (int i = 2; i < numArg-1; i++)
                    {
                        wordptr = wordEndPtr + 1;
                        scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                        *wordEndPtr = '\0';
                        curScDat->textNames[nT] = sceneTextEntryNamesBufPtr;
                        pcch = wordptr;
                        cch = 0xFF;
                        while (cch)
                        {
                            cch = *pcch++;
                            *sceneTextEntryNamesBufPtr++ = cch;
                        }
                        curScDat->numTexts++;
                        nT++;
                    }
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    curScDat->textNames[nT] = sceneTextEntryNamesBufPtr;
                    pcch = wordptr;
                    cch = 0xFF;
                    while (cch)
                    {
                        cch = *pcch++;
                        *sceneTextEntryNamesBufPtr++ = cch;
                    }
                    curScDat->numTexts++;
                    textChain = false; //Since choice labels are taken from the scene's pool of texts, chaining is interrupted
                    break;
                case OperandType::LUT:
                    scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = GetStateVarNum(wordptr);
                    if (arg0 < 0)
                    {
                        arg0 = 0;
                        (*(curScDat->refVarLabels))[locCounter] = std::string(wordptr);
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    for (int i = 1; i < numArg-1; i++)
                    {
                        wordptr = wordEndPtr + 1;
                        scanStat = ScanForArgument((const char**)&wordptr, (const char**)&wordEndPtr);
                        *wordEndPtr = '\0';
                        arg0 = atoi(wordptr);
                        numBytesInInstruction += 2;
                        instructionBytes[1 + 2*i] = (unsigned char)(arg0 & 0x00FF);
                        instructionBytes[2 + 2*i] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    }
                    wordptr = wordEndPtr + 1;
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    arg0 = atoi(wordptr);
                    numBytesInInstruction += 2;
                    instructionBytes[1 + 2*(numArg-1)] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2 + 2*(numArg-1)] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    break;
            }

            for (int i = 0; i < numBytesInInstruction; i++)
            {
                *sceneDataBufPtr++ = instructionBytes[i];
            }

            locCounter += numBytesInInstruction - 1;
            scenes[scene].dataLen = locCounter;
            break;
        case OP_DIRECTIVE: //High level directive that is not directly stored
            switch (type)
            {
                case OperandType::NOOPERANDS: //No operands means nothing else to do
                    break;
                case OperandType::SCENEREF:
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    if (opcode == 0x10000) //.scene
                    {
                        curScDat = &scenes[numScenes];
                        curScDat->numTexts = 0;
                        curScDat->dataLen = 0;
                        curScDat->label = sceneNamesBufPtr;
                        curScDat->data = sceneDataBufPtr;
                        curScDat->refSceneLabels = new std::unordered_map<int, std::string>();
                        curScDat->foundTextLabels = new std::unordered_map<std::string, int>();
                        curScDat->refVarLabels = new std::unordered_map<int, std::string>();
                        curScDat->textNames = (char**)malloc(sizeof(char*) * DEFAULT_ALLOCBLOCK_SEP);
                        char ch = *wordptr++;
                        while (ch)
                        {
                            *sceneNamesBufPtr++ = ch;
                            ch = *wordptr++;
                        }
                        *sceneNamesBufPtr++ = '\0';
                        scene = numScenes;
                        numScenes++;
                    }
                    break;
                case OperandType::STATEVAR_SINGLE:
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    break;
            }
            break;
        case OP_JUMPLABEL: //Registers a new jump label
            wordEndPtr[-1] = '\0';
            (*foundJumpLabels)[std::string(jumpLabelPtr)] = locCounter;
            textChain = false; //Any valid jump label is a place where we would need to start counting the text number again
            break;
    }

    *curScene = scene;
    return 0;
}

int ParseInputFile(char* contents, const long length, const char* filename)
{
    char* lineptr = contents;
    int result = 0;
    int curScene = -1;
    int lineNum = 1;
    foundJumpLabels = new std::unordered_map<std::string, int>();
    refJumpLabels = new std::vector<std::pair<int, std::string>>();
    textChain = false;

    for (long i = 0; i < length; i++)
    {
        char curChar = *contents;
        //Line feed and carriage return define separate lines
        if (curChar == '\n' || curChar == '\r' || curChar == EOT)
        {
            *contents = '\0';
            result = ParseLine(lineptr, &curScene);
            switch (result)
            {
                case 0: break;
                case 0x10:
                    printf(autoOpMessage, filename, lineNum);
                    break;
                case OP_INVALIDOP:
                    printf(invalidOpMessage, filename, lineNum);
                    break;
                case OP_SCENEUNDEFINED:
                    printf(sceneUndefinedMessage, filename, lineNum);
                    delete foundJumpLabels;
                    delete refJumpLabels;
                    return result;
                default:
                    delete foundJumpLabels;
                    delete refJumpLabels;
                    return result;
            }
            lineptr = contents + 1;
            if (curChar == '\n') lineNum++;
        }
        contents++;
    }

    unsigned char* curSceneData;
    if (curScene >= 0)
    {
        curSceneData = scenes[curScene].data;
        for (int i = 0; i < refJumpLabels->size(); i++)
        {
            std::pair<int, std::string> refpair = (*refJumpLabels)[i];
            *((int16_t*)(&curSceneData[refpair.first])) = (int16_t)((*foundJumpLabels)[refpair.second] - refpair.first - 2);
        }
    }

    delete foundJumpLabels;
    delete refJumpLabels;
    return result;
}

int AssembleScenes(const char* outputFilename, const char** inputFilenames, const int numInputFiles)
{
    unsigned char* inputFileContents;
    int numInvalidFiles = 0;
    FILE* outputFileHandle = fopen(outputFilename, "wb");
    if (outputFileHandle == NULL)
    {
        printf("CRITICAL ERROR - Output file %s could not be opened!", outputFilename);
        return 1;
    }

    sceneNamesBufferAllocSize = DEFAULT_ALLOCBLOCK_SNB;
    sceneNamesBuffer = (char*)malloc(sceneNamesBufferAllocSize);
    sceneNamesBufPtr = sceneNamesBuffer;
    sceneTextEntryNamesBufferAllocSize = DEFAULT_ALLOCBLOCK_STENB;
    sceneTextEntryNamesBuffer = (char*)malloc(sceneTextEntryNamesBufferAllocSize);
    sceneTextEntryNamesBufPtr = sceneTextEntryNamesBuffer;
    sceneDataBufferAllocSize = DEFAULT_ALLOCBLOCK_SDB;
    sceneDataBuffer = (unsigned char*)malloc(sceneDataBufferAllocSize);
    sceneDataBufPtr = sceneDataBuffer;
    charLabelsBufferAllocSize = DEFAULT_ALLOCBLOCK_CLB;
    charLabelsBuffer = (char*)malloc(charLabelsBufferAllocSize);
    charLabelsBufPtr = charLabelsBuffer;
    bgLabelsBufferAllocSize = DEFAULT_ALLOCBLOCK_BGLB;
    bgLabelsBuffer = (char*)malloc(bgLabelsBufferAllocSize);
    bgLabelsBufPtr = bgLabelsBuffer;
    bgVariantLabelsBufferAllocSize = DEFAULT_ALLOCBLOCK_BGVLB;
    bgVariantLabelsBuffer = (char*)malloc(bgVariantLabelsBufferAllocSize);
    bgVariantLabelsBufPtr = bgVariantLabelsBuffer;
    sprLabelsBufferAllocSize = DEFAULT_ALLOCBLOCK_SPLB;
    sprLabelsBuffer = (char*)malloc(sprLabelsBufferAllocSize);
    sprLabelsBufPtr = sprLabelsBuffer;
    sprVariantLabelsBufferAllocSize = DEFAULT_ALLOCBLOCK_SPVLB;
    sprVariantLabelsBuffer = (char*)malloc(sprVariantLabelsBufferAllocSize);
    sprVariantLabelsBufPtr = sprVariantLabelsBuffer;

    numScenes = 0;
    numChars = 0;
    numBg = 0;
    numSpr = 0;
    currentEntryScene = 0;
    foundSceneLabels = new std::unordered_map<std::string, int>();
    foundCharLabels = new std::unordered_map<std::string, int>();
    foundVarLabels = new std::unordered_map<std::string, int>(defaultVarNames);
    foundBgLabels = new std::unordered_map<std::string, int>();
    foundSprLabels = new std::unordered_map<std::string, int>();

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
        inputFileContents = (unsigned char*)malloc(fileLen);
        inputFileContents[fileLen - 1] = EOT; //Put EOT character in buffer so that the parser knows when the file has ended
        fread(inputFileContents, 1, fileLen, inputFileHandle);
        fclose(inputFileHandle);

        int result = ParseInputFile((char*)inputFileContents, fileLen, inputFilenames[i]);
        free(inputFileContents);
    }

    delete foundSceneLabels; //These were only needed to quickly check if a certain label already existed
    delete foundCharLabels;
    delete foundBgLabels;
    delete foundSprLabels;

    if (numInvalidFiles >= numInputFiles)
    {
        printf("CRITICAL ERROR - All input files referenced are invalid!");
        fclose(outputFileHandle);
        remove(outputFilename);
        free(sceneNamesBuffer);
        free(sceneTextEntryNamesBuffer);
        free(sceneDataBuffer);
        free(charLabelsBuffer);
        free(bgLabelsBuffer);
        free(bgVariantLabelsBuffer);
        free(sprLabelsBuffer);
        free(sprVariantLabelsBuffer);
        delete foundVarLabels;
        return 1;
    }

    //Swap entry scene with scene 0 since MHVN98 assumes the first scene is always scene 0
    if (currentEntryScene != 0)
    {
        SceneData dat1 = scenes[currentEntryScene];
        SceneData dat2 = scenes[0];
        scenes[0] = dat1;
        scenes[currentEntryScene] = dat2;
    }

    //TODO: Scene reference fixing

    for (int i = 0; i < numScenes; i++)
    {
        delete scenes[i].refSceneLabels;
        delete scenes[i].foundTextLabels;
        delete scenes[i].refVarLabels;
    }
    for (int i = 0; i < numBg; i++)
    {
        delete bg[i].foundVariantLabels;
    }
    for (int i = 0; i < numSpr; i++)
    {
        delete spr[i].foundVariantLabels;
    }

    //Write scene data to file
    size_t sizeOfSceneDat = 4 + 4 * numScenes;
    uint32_t* scenePtrs = (uint32_t*)malloc(numScenes * sizeof(uint32_t));
    uint64_t curScenePtr = 0;
    uint32_t* compLens = (uint32_t*)malloc(numScenes * sizeof(uint32_t));
    char** compData = (char**)malloc(numScenes * sizeof(char*));
    for (int i = 0; i < numScenes; i++)
    {
        compData[i] = (char*)malloc(16384);
        uint32_t compressedSizeScene = LZ4_compress_HC((char*)(scenes[i].data), compData[i] + 4, scenes[i].dataLen, 16384, LZ4HC_CLEVEL_MAX);
        *((uint32_t*)compData[i]) = compressedSizeScene;
        compLens[i] = compressedSizeScene + 4;
        sizeOfSceneDat += compressedSizeScene + 4;
        scenePtrs[i] = (uint32_t)curScenePtr;
        curScenePtr += compressedSizeScene + 4;
    }
    uint64_t linkInfoPtr = 0x00000008 + sizeOfSceneDat;
    fwrite(&linkInfoPtr, sizeof(uint64_t), 1, outputFileHandle);
    fwrite(&numScenes, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numChars, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(scenePtrs, sizeof(uint32_t), numScenes, outputFileHandle);
    for (int i = 0; i < numScenes; i++)
    {
        fwrite(compData[i], 1, compLens[i], outputFileHandle);
    }
    //Write link info (may need some cleanup)
    fwrite(&numChars, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numScenes, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numGlobVars, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numGlobFlags, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numLocalVars, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numLocalFlags, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numBg, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numSpr, sizeof(uint16_t), 1, outputFileHandle);
    unsigned char zero = 0;
    for (int i = 0; i < numChars; i++)
    {
        fputs(charLabels[i], outputFileHandle);
        fwrite(&zero, 1, 1, outputFileHandle);
    }
    for (int i = 0; i < numGlobVars; i++)
    {
        fputs(globVarLabels[i], outputFileHandle);
        fwrite(&zero, 1, 1, outputFileHandle);
    }
    for (int i = 0; i < numGlobFlags; i++)
    {
        fputs(globFlagLabels[i], outputFileHandle);
        fwrite(&zero, 1, 1, outputFileHandle);
    }
    for (int i = 0; i < numLocalVars; i++)
    {
        fputs(localVarLabels[i], outputFileHandle);
        fwrite(&zero, 1, 1, outputFileHandle);
    }
    for (int i = 0; i < numLocalFlags; i++)
    {
        fputs(localFlagLabels[i], outputFileHandle);
        fwrite(&zero, 1, 1, outputFileHandle);
    }
    for (int i = 0; i < numScenes; i++)
    {
        fputs(scenes[i].label, outputFileHandle);
        fwrite(&zero, 1, 1, outputFileHandle);
    }
    for (int i = 0; i < numBg; i++)
    {
        fputs(bg[i].label, outputFileHandle);
        fwrite(&zero, 1, 1, outputFileHandle);
    }
    for (int i = 0; i < numSpr; i++)
    {
        fputs(spr[i].label, outputFileHandle);
        fwrite(&zero, 1, 1, outputFileHandle);
    }

    std::vector<unsigned char>* linkInfoOutputBuffer = new std::vector<unsigned char>();
    linkInfoOutputBuffer->reserve(0x100000);

    for (int i = 0; i < numScenes; i++)
    {
        uint16_t nT = scenes[i].numTexts;
        linkInfoOutputBuffer->push_back(nT & 0xFF);
        linkInfoOutputBuffer->push_back((nT >> 8) & 0xFF);
        const char* const* tlptr = scenes[i].textNames;
        for (int j = 0; j < nT; j++)
        {
            const char* chptr = tlptr[j];
            char ch = 0xFF;
            while (ch)
            {
                ch = *chptr++;
                linkInfoOutputBuffer->push_back(ch);
            }
        }
    }
    for (int i = 0; i < numBg; i++)
    {
        uint16_t nV = bg[i].numVariants;
        linkInfoOutputBuffer->push_back(nV & 0xFF);
        linkInfoOutputBuffer->push_back((nV >> 8) & 0xFF);
        const char* const* bgvptr = bg[i].variantNames;
        for (int j = 0; j < nV; j++)
        {
            const char* chptr = bgvptr[j];
            char ch = 0xFF;
            while (ch)
            {
                ch = *chptr++;
                linkInfoOutputBuffer->push_back(ch);
            }
        }
    }
    for (int i = 0; i < numSpr; i++)
    {
        uint16_t nV = spr[i].numVariants;
        linkInfoOutputBuffer->push_back(nV & 0xFF);
        linkInfoOutputBuffer->push_back((nV >> 8) & 0xFF);
        const char* const* sprvptr = spr[i].variantNames;
        for (int j = 0; j < nV; j++)
        {
            const char* chptr = sprvptr[j];
            char ch = 0xFF;
            while (ch)
            {
                ch = *chptr++;
                linkInfoOutputBuffer->push_back(ch);
            }
        }
    }

    fwrite(linkInfoOutputBuffer->data(), sizeof(unsigned char), linkInfoOutputBuffer->size(), outputFileHandle);


    fclose(outputFileHandle);
    delete linkInfoOutputBuffer;
    free(scenePtrs);
    free(sceneNamesBuffer);
    free(sceneTextEntryNamesBuffer);
    free(sceneDataBuffer);
    free(charLabelsBuffer);
    free(bgLabelsBuffer);
    free(bgVariantLabelsBuffer);
    free(sprLabelsBuffer);
    free(sprVariantLabelsBuffer);
    delete foundVarLabels;
    return 0;
}
