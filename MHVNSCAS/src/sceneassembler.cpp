#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unordered_map>
#include <string>

#define EOT ((char)0x04)

#define OP_INSTRUCTION 0x00000000
#define OP_DIRECTIVE 0x00010000
#define OP_JUMPLABEL 0x00020000
#define OP_INVALIDOP -1
#define OP_SCENEUNDEFINED -2

//Initial allocation sizes, and also how many more bytes/entries to add if more memory needs to be allocated for some reason
#define DEFAULT_ALLOCBLOCK_SNB 65536
#define DEFAULT_ALLOCBLOCK_STENB 65536
#define DEFAULT_ALLOCBLOCK_SDB 65536
#define DEFAULT_ALLOCBLOCK_CLB 65536

//This opcode map also accounts for pseudoinstructions by going beyond a byte
const std::unordered_map<std::string, int> mnemonicsToOpcodes =
{
    {std::string("gotoscene"),   0x00},
    {std::string("vnend"),      0x100},
    {std::string("jmp"),         0x01},
    {std::string("jz"),          0x02},
    {std::string("je"),          0x02},
    {std::string("jnz"),         0x03},
    {std::string("jne"),         0x03},
    {std::string("jn"),          0x04},
    {std::string("jl"),          0x04},
    {std::string("jp"),          0x05},
    {std::string("jge"),         0x05},
    {std::string("jle"),         0x06},
    {std::string("jg"),          0x07},
    {std::string("nexttext"),    0x10},
    {std::string("text"),        0x11},
    {std::string("charname"),    0x12},
    {std::string("nocharname"), 0x112},
    {std::string("deltext"),     0x1F},
    {std::string("lut"),         0x20},
    {std::string("multichoice"), 0x21},
    {std::string("ynchoicefl"),  0x22},
    {std::string("ynchoicest"),  0x23},
    {std::string("setvi"),       0x24},
    {std::string("setvv"),       0x25},
    {std::string("cmpvi"),       0x26},
    {std::string("cmpvv"),       0x27},
    {std::string("addvi"),       0x28},
    {std::string("addvv"),       0x29},
    {std::string("subvi"),       0x2A},
    {std::string("subvv"),       0x2B},
    //Directives included as well for ease of programming
    {std::string(".scene"),   0x10000},
    {std::string(".vnentry"), 0x10001},
};

typedef struct
{
    char* label;
    unsigned char* data;
    char** textNames;
    int numTexts;
    int dataLen;
    std::unordered_map<int, std::string>* refSceneLabels;
    std::unordered_map<std::string, int>* foundTextLabels;
} SceneData;

SceneData scenes[65536];
int numScenes;
int currentEntryScene;
char* charLabels[65536];
int numChars;
bool textChain;

std::unordered_map<std::string, int>* foundJumpLabels;
std::unordered_map<int, std::string>* refJumpLabels;
std::unordered_map<std::string, int>* foundSceneLabels;
std::unordered_map<std::string, int>* foundCharLabels;

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

const char* invalidOpMessage = "ERROR - %s, line %u: invalid operation. Line ignored.";
const char* autoOpMessage = "ERROR - %s, line %u: operation is not valid for use in assembly files. Remember that the assembler generates this instruction as part of optimisation. Line ignored";
const char* sceneUndefinedMessage = "ERROR - %s, line %u: attempted to use operations without declaring the scene first. File ignored.";

int GetOpcode(const char* firstWordOfLine)
{
    std::string* wordstr = new std::string(firstWordOfLine);
    int returnstat = OP_INVALIDOP;
    if (wordstr->back() == ':') //Colon at end -> jump label
    {
        returnstat = OP_JUMPLABEL;
    }
    else if (mnemonicsToOpcodes.count(*wordstr))
    {
        returnstat = mnemonicsToOpcodes.at(*wordstr);
    }
    delete wordstr;
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
            if (lenArg)
            {
                *start = wordPtr;
                foundWord = true;
            }
            lenArg = 0;
            wordPtr = curChPtr;
        }
        else lenArg++;
        ch = *curChPtr++;
        if (foundComment || foundWord) break;
    }
    if (foundComment || foundWord) *end = wordPtr - 1;
    else *end = curChPtr - 1;
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
            if (lenArg)
            {
                *start = wordPtr;
                foundWord = true;
                if (ch == ',') foundArgument = true;
            }
            lenArg = 0;
            wordPtr = curChPtr;
        }
        else lenArg++;
        ch = *curChPtr++;
        if (foundComment || foundWord) break;
    }
    if (foundComment || foundWord) *end = wordPtr - 1;
    else *end = curChPtr - 1;
    if (lenArg)
    {
        foundWord = true;
    }
    int returnStat = foundWord ? SCANSTAT_WORD : 0x0;
    returnStat |= foundComment ? SCANSTAT_COMMENT : 0x0;
    returnStat |= foundArgument ? SCANSTAT_ARG : 0x0;
    return returnStat;
}

typedef enum
{
    NOOPERANDS, //Operation takes no operands
    SCENEREF, //Operation takes a single scene reference
    TEXTREF, //Operation takes a single text reference
    CHARACTERREF, //Operation takes a single character reference
    JUMP, //Operation takes a single jump label
    STATEVAR_SINGLE, //Operation takes a single state variable reference
    STATEVAR_DOUBLE, //Operation takes two state variable references
    STATEVAR_SINGLE_IMMEDIATE, //Operation takes a single state variable reference and an immediate
    MULTICHOICE, //Operation is the 'multichoice' instruction
    LUT //Operation is the 'lut' instruction
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
            type = OperandType::NOOPERANDS;
            break;
        case 0x01: //jmp
            numArg = 1;
            type = OperandType::JUMP;
            break;
        case 0x02: //jz, je
            numArg = 1;
            type = OperandType::JUMP;
            break;
        case 0x03: //jnz, jne
            numArg = 1;
            type = OperandType::JUMP;
            break;
        case 0x04: //jn, jl
            numArg = 1;
            type = OperandType::JUMP;
            break;
        case 0x05: //jp, jge
            numArg = 1;
            type = OperandType::JUMP;
            break;
        case 0x06: //jle
            numArg = 1;
            type = OperandType::JUMP;
            break;
        case 0x07: //jg
            numArg = 1;
            type = OperandType::JUMP;
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
            type = OperandType::NOOPERANDS;
            break;
        case 0x1F: //deltext
            numArg = 0;
            break;
        case 0x20: //lut
            numArg = 2;
            type = OperandType::LUT;
            break;
        case 0x21: //multichoice
            numArg = 9;
            type = OperandType::MULTICHOICE;
            break;
        case 0x22: //ynchoicefl
            numArg = 0;
            break;
        case 0x23: //ynchoicest
            numArg = 1;
            type = OperandType::STATEVAR_SINGLE;
            break;
        case 0x24: //setvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x25: //setvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        case 0x26: //cmpvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x27: //cmpvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        case 0x28: //addvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x29: //addvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        case 0x2A: //subvi
            numArg = 2;
            type = OperandType::STATEVAR_SINGLE_IMMEDIATE;
            break;
        case 0x2B: //subvv
            numArg = 2;
            type = OperandType::STATEVAR_DOUBLE;
            break;
        
        case 0x10000: //.scene
            numArg = 1;
            type = OperandType::SCENEREF;
            break;
        case 0x10001: //.vnentry
            numArg = 0;
            if (scene >= 0) currentEntryScene = scene;
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

    //Tried to use operation other than .scene without declaring a scene first -> error, stop parsing file
    if (scene < 0 && opcode != 0x10000)
    {
        return OP_SCENEUNDEFINED;
    }

    //Invalid opcode -> error, ignore line
    if (opcode == OP_INVALIDOP || opcode == 0x10)
    {
        return opcode;
    }

    wordptr = wordEndPtr + 1;

    int nT = curScDat->numTexts;
    int arg0;
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
                        numChars++;
                    }
                    numBytesInInstruction += 2;
                    instructionBytes[1] = (unsigned char)(arg0 & 0x00FF);
                    instructionBytes[2] = (unsigned char)((arg0 & 0xFF00) >> 8);
                    break;
                case OperandType::JUMP: //Resolve jumps after parsing all lines (we'll just put a dummy value in for now)
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    (*refJumpLabels)[locCounter] = std::string(wordptr);
                    numBytesInInstruction += 2;
                    instructionBytes[1] = 0x00;
                    instructionBytes[2] = 0x00;
                    break;
                case OperandType::STATEVAR_SINGLE:
                    scanStat = ScanForWord((const char**)&wordptr, (const char**)&wordEndPtr);
                    *wordEndPtr = '\0';
                    numBytesInInstruction += 2;
                    instructionBytes[1] = 0x00;
                    instructionBytes[2] = 0x00;
                    break;
                case OperandType::STATEVAR_DOUBLE:
                    break;
                case OperandType::STATEVAR_SINGLE_IMMEDIATE:
                    break;
                case OperandType::MULTICHOICE:
                    break;
                case OperandType::LUT:
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
            }
            break;
        case OP_JUMPLABEL: //Registers a new jump label
            wordEndPtr[-1] = '\0';
            (*foundJumpLabels)[std::string(wordptr)] = locCounter;
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
    refJumpLabels = new std::unordered_map<int, std::string>();
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
                    return result;
                default: return result;
            }
            lineptr = contents + 1;
            if (curChar == '\n') lineNum++;
        }
        contents++;
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

    numScenes = 0;
    numChars = 0;
    currentEntryScene = 0;
    foundSceneLabels = new std::unordered_map<std::string, int>();
    foundCharLabels = new std::unordered_map<std::string, int>();

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

    if (numInvalidFiles >= numInputFiles)
    {
        printf("CRITICAL ERROR - All input files referenced are invalid!");
        fclose(outputFileHandle);
        remove(outputFilename);
        free(sceneNamesBuffer);
        free(sceneTextEntryNamesBuffer);
        free(sceneDataBuffer);
        free(charLabelsBuffer);
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
    }

    //Write scene data to file
    size_t sizeOfSceneDat = 4 + 4 * numScenes;
    uint32_t* scenePtrs = (uint32_t*)malloc(numScenes * sizeof(uint32_t));
    uint64_t curScenePtr = 0;
    for (int i = 0; i < numScenes; i++)
    {
        const int curSceneLen = scenes[i].dataLen;
        sizeOfSceneDat += curSceneLen;
        scenePtrs[i] = (uint32_t)curScenePtr;
        curScenePtr += curSceneLen;
    }
    uint64_t linkInfoPtr = 0x00000008 + sizeOfSceneDat;
    fwrite(&linkInfoPtr, sizeof(uint64_t), 1, outputFileHandle);
    fwrite(&numScenes, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(&numChars, sizeof(uint16_t), 1, outputFileHandle);
    fwrite(scenePtrs, sizeof(uint32_t), numScenes, outputFileHandle);
    for (int i = 0; i < numScenes; i++)
    {
        fwrite(scenes[i].data, 1, scenes[i].dataLen, outputFileHandle);
    }

    //TODO: link info write

    free(scenePtrs);
    free(sceneNamesBuffer);
    free(sceneTextEntryNamesBuffer);
    free(sceneDataBuffer);
    free(charLabelsBuffer);
    return 0;
}