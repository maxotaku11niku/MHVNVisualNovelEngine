#include <stdio.h>
#include <stdlib.h>
#include <libi86/malloc.h>
#include <dos.h>
//#include "platform/x86strops.h"
//#include "platform/memalloc.h"
#include "platform/filehandling.h"
#include "platform/pc98_egc.h"
#include "stdbuffer.h"
#include "rootinfo.h"
#include "sceneengine.h"
#include "textengine.h"

#define VMFLAG_Z 0x01
#define VMFLAG_N 0x02
#define VMFLAG_TEXTINBOX 0x40
#define VMFLAG_PROCESS 0x80

#define SVAR_BASE 0x0000
#define SFLG_BASE 0x0020
#define GVAR_BASE 0x0080
#define GFLG_BASE 0x0100
#define LVAR_BASE 0x0400
#define LFLG_BASE 0x0600
#define VARSPACE_TOP 0x1000

SceneInfo sceneInfo;
unsigned char curSceneData[1024];
unsigned short curSceneDataPC;
unsigned char vmFlags;
int returnStatus;
unsigned short curCharNum;
unsigned short nextTextNum;
char curCharName[64];
unsigned int curTextArray[256];
__far char* sceneTextBuffer = 0;
int sdHandle;
//FILE* sdHandle;

short scratchVars[32];
short globalVars[128];
short localVars[512];
//Flags are bitpacked for space-efficiency reasons
unsigned char scratchFlags[12];
unsigned char globalFlags[96];
unsigned char localFlags[320];

int LoadNewScene(unsigned short sceneNum)
{
    if (sceneNum == sceneInfo.curScene) return 0; //Don't change scene if scene number isn't going to change
    unsigned int realReadLen;
    unsigned long curfilepos;
    unsigned long scenedatpos;
    int result = _dos_open(rootInfo.sceneDataPath, 0, &sdHandle);
    //sdHandle = fopen(rootInfo.sceneDataPath, "rb");
    if (result)
    //if (sdHandle == 0)
    {
        WriteString("Error! Could not find scene data file!", 168, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
        return result; //Error handler
        //return 1; //Error handler
    }
    /**/
    SeekFile(sdHandle, FILE_SEEK_ABSOLUTE, 4 + 4 * sceneNum, &curfilepos);
    __far unsigned char* sdp = &scenedatpos;
    _dos_read(sdHandle, sdp, 4, &realReadLen);
    SeekFile(sdHandle, FILE_SEEK_ABSOLUTE, 4 + 4 * sceneInfo.numScenes + scenedatpos, &curfilepos);
    __far unsigned char* csd = curSceneData;
    _dos_read(sdHandle, csd, 1024, &realReadLen);
    _dos_close(sdHandle);
    //*/
    /*/
    fseek(sdHandle, 4 + 4 * sceneNum, SEEK_SET);
    fread(&scenedatpos, 1, 4, sdHandle);
    fseek(sdHandle, 4 + 4 * sceneInfo.numScenes + scenedatpos, SEEK_SET);
    fread(curSceneData, 1, 1024, sdHandle);
    fclose(sdHandle);
    //*/
    result = LoadSceneText(sceneNum, sceneTextBuffer, curTextArray);
    if (result) return result;
    curSceneDataPC = 0;
    returnStatus = 0;
    curCharNum = 0xFFFF;
    vmFlags = VMFLAG_PROCESS;
    return 0;
}

int SetupSceneEngine()
{
    unsigned int realReadLen;
    int result = _dos_open(rootInfo.sceneDataPath, 0, &sdHandle);
    //sdHandle = fopen(rootInfo.sceneDataPath, "rb");
    if (result)
    //if (sdHandle == 0)
    {
        WriteString("Error! Could not find scene data file!", 168, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
        return result; //Error handler
        //return 1; //Error handler
    }
    __far unsigned char* fb = smallFileBuffer;
    _dos_read(sdHandle, fb, 4, &realReadLen);
    //fread(smallFileBuffer, 1, 4, sdHandle);
    sceneInfo.numScenes = *((unsigned short*)(smallFileBuffer));
    sceneInfo.numChars = *((unsigned short*)(smallFileBuffer + 0x02));
    sceneTextBuffer = _fmalloc(0xFFFF);
    sceneInfo.curScene = 0xFFFF;
    _dos_close(sdHandle);
    //fclose(sdHandle);
    return LoadNewScene(0);
}

int FreeSceneEngine()
{
    if (sceneTextBuffer == 0) return 0;
    else _ffree(sceneTextBuffer);
    return 0;
}

short* GetVariableRef(unsigned short addr)
{
    if (addr < SFLG_BASE) //Scratch variables
    {
        return scratchVars + addr;
    }
    else if (addr < GVAR_BASE) //Scratch flags (invalid)
    {
        return (short*)0;
    }
    else if (addr < GFLG_BASE) //Global variables
    {
        return globalVars + addr - GVAR_BASE;
    }
    else if (addr < LVAR_BASE) //Global flags (invalid)
    {
        return (short*)0;
    }
    else if (addr < LFLG_BASE) //Local variables
    {
        return localVars + addr - LVAR_BASE;
    }
    else //Local flags (invalid) and out-of-range addresses
    {
        return (short*)0;
    }
}

unsigned char GetFlag(unsigned short addr)
{
    unsigned short val = 0;
    if (addr < SFLG_BASE) //Scratch variables
    {
        val = scratchVars[addr];
    }
    else if (addr < GVAR_BASE) //Scratch flags
    {
        addr -= SFLG_BASE;
        val  = scratchFlags[addr >> 3];
        val &= (0x01 << (addr & 0x7));
    }
    else if (addr < GFLG_BASE) //Global variables
    {
        val = globalVars[addr - GVAR_BASE];
    }
    else if (addr < LVAR_BASE) //Global flags
    {
        addr -= GFLG_BASE;
        val  = globalFlags[addr >> 3];
        val &= (0x01 << (addr & 0x7));
    }
    else if (addr < LFLG_BASE) //Local variables
    {
        val = localVars[addr - LVAR_BASE];
    }
    else if (addr < VARSPACE_TOP) //Local flags
    {
        addr -= LFLG_BASE;
        val  = localFlags[addr >> 3];
        val &= (0x01 << (addr & 0x7));
    }
    else return 0; //Out-of-range addresses
    return val != 0;
}

void SetFlag(unsigned short addr, unsigned char val)
{
    unsigned char* flgptr;
    unsigned char flgval;
    unsigned char flgsel;
    if (addr < SFLG_BASE) //Scratch variables (invalid)
    {
        return;
    }
    else if (addr < GVAR_BASE) //Scratch flags
    {
        addr -= SFLG_BASE;
        flgptr = scratchFlags + (addr >> 3);
    }
    else if (addr < GFLG_BASE) //Global variables (invalid)
    {
        return;
    }
    else if (addr < LVAR_BASE) //Global flags
    {
        addr -= GFLG_BASE;
        flgptr = globalFlags + (addr >> 3);
    }
    else if (addr < LFLG_BASE) //Local variables (invalid)
    {
        return;
    }
    else if (addr < VARSPACE_TOP) //Local flags
    {
        addr -= LFLG_BASE;
        flgptr = localFlags + (addr >> 3);
    }
    else return; //Out-of-range addresses
    flgval = *flgptr;
    flgsel = (0x01 << (addr & 0x7));
    val = val != 0;
    val <<= (addr & 0x7);
    flgval &= ~flgsel;
    flgval |= val;
    *flgptr = flgval;
}

void ControlProcess(unsigned char process)
{
    if (process) vmFlags |= VMFLAG_PROCESS;
    else vmFlags &= ~VMFLAG_PROCESS;
}

void ClearTextBox()
{
    ClearLinesEGC(textBoxtY, textBoxbY - textBoxtY + 1);
    egc_patdatandreadmode(EGC_PATTERNSOURCE_FGCOLOUR);
    egc_rwmode(EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP((EGC_ROP_SRC & EGC_ROP_PAT) | ((~EGC_ROP_SRC) & EGC_ROP_DST)));
    egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
    egc_bitlen(32);
}

void ClearCharacterName()
{
    ClearLinesEGC(60, 17);
    egc_patdatandreadmode(EGC_PATTERNSOURCE_FGCOLOUR);
    egc_rwmode(EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP((EGC_ROP_SRC & EGC_ROP_PAT) | ((~EGC_ROP_SRC) & EGC_ROP_DST)));
    egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
    egc_bitlen(32);
}

int SceneDataProcess()
{
    unsigned char curOpcode;
    int result = 0;
    short* varptr1;
    short* varptr2;
    while (vmFlags & VMFLAG_PROCESS)
    {
        returnStatus = 0;
        curOpcode = curSceneData[curSceneDataPC++];
        switch (curOpcode)
        {
        case 0x00: //gotoscene
        {
            unsigned short sNum = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (sNum == 0xFFFF) //scene number FFFF is a proxy for the end of the whole VN
            {
                vmFlags &= ~VMFLAG_PROCESS;
                returnStatus |= SCENE_STATUS_FINALEND;
                break;
            }
            result = LoadNewScene(sNum);
            if (result)
            {
                returnStatus = SCENE_STATUS_ERROR | result;
                vmFlags &= ~VMFLAG_PROCESS;
                break;
            }
            curSceneDataPC += 2;
            break;
        }
        case 0x01: //jmp
            curSceneDataPC += 2;
            vmJump:
            curSceneDataPC += *((unsigned short*)(curSceneData + curSceneDataPC - 2));
            break;
        case 0x02: //jz, je
            curSceneDataPC += 2;
            if (vmFlags & VMFLAG_Z) goto vmJump;
            else break;
        case 0x03: //jnz, jne
            curSceneDataPC += 2;
            if (vmFlags & VMFLAG_Z) break;
            else goto vmJump;
        case 0x04: //jn, jl
            curSceneDataPC += 2;
            if (vmFlags & VMFLAG_N) goto vmJump;
            else break;
        case 0x05: //jp, jge
            curSceneDataPC += 2;
            if (vmFlags & VMFLAG_N) break;
            else goto vmJump;
        case 0x06: //jle
            curSceneDataPC += 2;
            if (vmFlags & (VMFLAG_Z | VMFLAG_N)) goto vmJump;
            else break;
        case 0x07: //jg
            curSceneDataPC += 2;
            if (vmFlags & (VMFLAG_Z | VMFLAG_N)) break;
            else goto vmJump;
        case 0x11: //text
            if (vmFlags & VMFLAG_TEXTINBOX)
            {
                curSceneDataPC--; //compensation
                goto delText;
            }
            nextTextNum = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
        case 0x10: //nexttext
            if (vmFlags & VMFLAG_TEXTINBOX)
            {
                curSceneDataPC--; //compensation
                goto delText;
            }
            StartAnimatedStringToWrite(sceneTextBuffer + curTextArray[nextTextNum], textBoxlX, textBoxtY, rootInfo.defFormatNormal);
            nextTextNum++;
            vmFlags |= VMFLAG_TEXTINBOX;
            vmFlags &= ~VMFLAG_PROCESS;
            returnStatus |= SCENE_STATUS_RENDERTEXT;
            break;
        case 0x12: //charname
        {
            unsigned short charNum = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (charNum != curCharNum)
            {
                ClearCharacterName();
                if (charNum != 0xFFFF)
                {
                    result = LoadCurrentCharacterName(charNum, curCharName);
                    if (result)
                    {
                        returnStatus = SCENE_STATUS_ERROR | result;
                        vmFlags &= ~VMFLAG_PROCESS;
                        break;
                    }
                    WriteString(curCharName, 60, 60, rootInfo.defFormatCharName, 0);
                }
                curCharNum = charNum;
            }
            curSceneDataPC += 2;
            break;
        }
        case 0x13: //deltext
            delText:
            ClearTextBox();
            vmFlags &= ~VMFLAG_TEXTINBOX;
            vmFlags &= ~VMFLAG_PROCESS;
            returnStatus |= SCENE_STATUS_WIPETEXT; //for later, when text box wiping involves an animation
            break;
        case 0x14: //ynchoice
            break; //stub
        case 0x15: //choice2
            curSceneDataPC += 4; //stub
            break;
        case 0x16: //choice3
            curSceneDataPC += 4; //stub
            break;
        case 0x17: //choice4
            curSceneDataPC += 4; //stub
            break;
        case 0x20: //lut2
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (*varptr1 == 0)
            {
                *varptr1 = result;
                curSceneDataPC += 4;
                break;
            }
            curSceneDataPC += 2;
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            if (*varptr1 == 1)
            {
                *varptr1 = result;
            }
            else //escape condition
            {
                *varptr1 = 0;
            }
            break;
        case 0x21: //lut3
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (*varptr1 == 0)
            {
                *varptr1 = result;
                curSceneDataPC += 6;
                break;
            }
            curSceneDataPC += 2;
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (*varptr1 == 1)
            {
                *varptr1 = result;
                curSceneDataPC += 4;
                break;
            }
            curSceneDataPC += 2;
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            if (*varptr1 == 2)
            {
                *varptr1 = result;
            }
            else //escape condition
            {
                *varptr1 = 0;
            }
            break;
        case 0x22: //lut4
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (*varptr1 == 0)
            {
                *varptr1 = result;
                curSceneDataPC += 8;
                break;
            }
            curSceneDataPC += 2;
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (*varptr1 == 1)
            {
                *varptr1 = result;
                curSceneDataPC += 6;
                break;
            }
            curSceneDataPC += 2;
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (*varptr1 == 2)
            {
                *varptr1 = result;
                curSceneDataPC += 4;
                break;
            }
            curSceneDataPC += 2;
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            if (*varptr1 == 3)
            {
                *varptr1 = result;
            }
            else //escape condition
            {
                *varptr1 = 0;
            }
            break;
        case 0x23: //swapzn
            result  = (vmFlags & VMFLAG_Z) << 1;
            result |= (vmFlags & VMFLAG_N) >> 1;
            vmFlags = (vmFlags & ~(VMFLAG_Z | VMFLAG_N)) | result;
            break;
        case 0x24: //setvi
            vmSetvi:
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            *varptr1 = result;
            curSceneDataPC += 2;
            break;
        case 0x25: //setvv
            vmSetvv:
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            *varptr1 = *GetVariableRef(result);
            curSceneDataPC += 2;
            break;
        case 0x26: //csetvi
            if (vmFlags & VMFLAG_Z) goto vmSetvi;
            else 
            {
                curSceneDataPC += 4;
                break;
            }
        case 0x27: //csetvv
            if (vmFlags & VMFLAG_Z) goto vmSetvv;
            else 
            {
                curSceneDataPC += 4;
                break;
            }
        case 0x28: //cmpvi
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            vmFlags &= ~(VMFLAG_Z | VMFLAG_N);
            vmFlags |= VMFLAG_Z & (*varptr1 == result);
            vmFlags |= VMFLAG_N & (*varptr1 < result);
            break;
        case 0x29: //cmpvv
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            varptr2 = GetVariableRef(result);
            curSceneDataPC += 2;
            vmFlags &= ~(VMFLAG_Z | VMFLAG_N);
            vmFlags |= VMFLAG_Z & (*varptr1 == *varptr2);
            vmFlags |= VMFLAG_N & (*varptr1 < *varptr2);
            break;
        case 0x2A: //addvi
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            *varptr1 += result;
            curSceneDataPC += 2;
            vmFlags &= ~(VMFLAG_Z | VMFLAG_N);
            vmFlags |= VMFLAG_Z & (*varptr1 == 0);
            vmFlags |= VMFLAG_N & (*varptr1 < 0);
            break;
        case 0x2B: //addvv
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            *varptr1 += *GetVariableRef(result);
            curSceneDataPC += 2;
            vmFlags &= ~(VMFLAG_Z | VMFLAG_N);
            vmFlags |= VMFLAG_Z & (*varptr1 == 0);
            vmFlags |= VMFLAG_N & (*varptr1 < 0);
            break;
        case 0x2C: //subvi
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            *varptr1 -= result;
            curSceneDataPC += 2;
            vmFlags &= ~(VMFLAG_Z | VMFLAG_N);
            vmFlags |= VMFLAG_Z & (*varptr1 == 0);
            vmFlags |= VMFLAG_N & (*varptr1 < 0);
            break;
        case 0x2D: //subvv
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            varptr1 = GetVariableRef(result);
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            *varptr1 -= *GetVariableRef(result);
            curSceneDataPC += 2;
            vmFlags &= ~(VMFLAG_Z | VMFLAG_N);
            vmFlags |= VMFLAG_Z & (*varptr1 == 0);
            vmFlags |= VMFLAG_N & (*varptr1 < 0);
            break;
        case 0x2E: //ldflg
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            result = GetFlag(result);
            vmFlags &= ~VMFLAG_Z;
            vmFlags |= result;
            break;
        case 0x2F: //stflg
            result = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            SetFlag(result, vmFlags & VMFLAG_Z);
            break;
        default: //illegal/unimplemented opcode
            break;
        }
    }
    return returnStatus;
}
