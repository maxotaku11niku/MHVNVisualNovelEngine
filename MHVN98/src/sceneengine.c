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
 * Scene bytecode loader and interpreter
 */

#include "platform/x86strops.h"
#include "platform/memalloc.h"
#include "platform/filehandling.h"
#include "platform/pc98_egc.h"
#include "stdbuffer.h"
#include "rootinfo.h"
#include "sceneengine.h"
#include "textengine.h"
#include "graphics.h"

#define VMFLAG_Z         0x01
#define VMFLAG_N         0x02
#define VMFLAG_TEXTINBOX 0x40
#define VMFLAG_PROCESS   0x80

#define ASYNC_PALETTE 0x01
#define ASYNC_SCROLL  0x02
#define ASYNC_USER    0x80

#define APAL_BFADEIN    1
#define APAL_BFADEOUT   2
#define APAL_WFADEIN    3
#define APAL_WFADEOUT   4
#define APAL_PFADEIN    5
#define APAL_PFADEOUT   6
#define APAL_PHUEROTATE 7

#define ASCR_SHAKE 1

#define SVAR_BASE    0x0000
#define SFLG_BASE    0x0020
#define GVAR_BASE    0x0080
#define GFLG_BASE    0x0100
#define LVAR_BASE    0x0400
#define LFLG_BASE    0x0600
#define VARSPACE_TOP 0x1000

#define STYPE_YNCHOICE 0
#define STYPE_CHOICE2  1
#define STYPE_CHOICE3  2
#define STYPE_CHOICE4  3

SceneInfo sceneInfo;
unsigned char curSceneData[1024];
unsigned short curSceneDataPC;
unsigned char vmFlags;

unsigned char curAsyncActions;
unsigned char curAsyncPaletteAction;

unsigned char curAsyncScrollAction;
short curShakeAmp; //8.8 fixed point number
unsigned short curShakeAdv; //1.15 fixed point number
short curShakeDampFactor; //2.14 fixed point number
unsigned short curShakePoint; //1.15 fixed point number

int returnStatus;
unsigned char selectionType;
char selectedOption;
unsigned short selectedVar;
unsigned short selectedFirstText;
unsigned short curCharNum;
unsigned short nextTextNum;
char curCharName[64];
unsigned int curTextArray[256];
__far char* sceneTextBuffer = 0;

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
    fileptr handle;
    int result = OpenFile(rootInfo.sceneDataPath, DOSFILE_OPEN_READ, &handle);
    if (result)
    {
        WriteString("Error! Could not find scene data file!", 168, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result;
    }
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, 4 + 4 * sceneNum, &curfilepos);
    __far unsigned char* sdp = &scenedatpos;
    ReadFile(handle, 4, sdp, &realReadLen);
    SeekFile(handle, DOSFILE_SEEK_ABSOLUTE, 4 + 4 * sceneInfo.numScenes + scenedatpos, &curfilepos);
    __far unsigned char* csd = curSceneData;
    ReadFile(handle, sizeof(curSceneData), csd, &realReadLen);
    CloseFile(handle);
    result = LoadSceneText(sceneNum, sceneTextBuffer, curTextArray);
    if (result) return result;
    curSceneDataPC = 0;
    returnStatus = 0;
    curCharNum = 0xFFFF;
    curAsyncActions = 0;
    vmFlags = VMFLAG_PROCESS;
    return 0;
}

int SetupSceneEngine()
{
    unsigned int realReadLen;
    fileptr handle;
    int result = OpenFile(rootInfo.sceneDataPath, DOSFILE_OPEN_READ, &handle);
    if (result)
    {
        WriteString("Error! Could not find scene data file!", 168, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        return result;
    }
    __far unsigned char* fb = smallFileBuffer;
    ReadFile(handle, 4, fb, &realReadLen);
    sceneInfo.numScenes = *((unsigned short*)(smallFileBuffer));
    sceneInfo.numChars = *((unsigned short*)(smallFileBuffer + 0x02));
    sceneTextBuffer = MemAlloc(0x10000);
    sceneInfo.curScene = 0xFFFF;
    CloseFile(handle);
    return LoadNewScene(0);
}

int FreeSceneEngine()
{
    if (sceneTextBuffer == 0) return 0;
    else MemFree(sceneTextBuffer);
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
    Draw9SliceBoxInnerRegion(textBoxImgInfo);
}

void ClearCharacterName()
{
    Draw9SliceBoxInnerRegion(charNameBoxImgInfo);
}

void SwitchChoice(char dir)
{
    selectedOption += dir;
    int cx = choiceBoxInnerBounds.pos.x;
    int cy = choiceBoxInnerBounds.pos.y;
    Draw9SliceBoxInnerRegion(choiceBoxImgInfo);
    switch (selectionType)
    {
        case STYPE_YNCHOICE:
            if (selectedOption < 0) selectedOption = 1;
            else if (selectedOption > 1) selectedOption = 0;
            if (selectedOption)
            {
                WriteString("Yes", cx, cy, rootInfo.defFormatMenuItemSelected, 0);
                WriteString("No", cx, cy + 16, rootInfo.defFormatMenuItem, 0);
            }
            else
            {
                WriteString("Yes", cx, cy, rootInfo.defFormatMenuItem, 0);
                WriteString("No", cx, cy + 16, rootInfo.defFormatMenuItemSelected, 0);
            }
            break;
        case STYPE_CHOICE2:
            if (selectedOption < 0) selectedOption = 1;
            else if (selectedOption > 1) selectedOption = 0;
            if (selectedOption)
            {
                WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItem, 0);
                WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItemSelected, 0);
            }
            else
            {
                WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItemSelected, 0);
                WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItem, 0);
            }
            break;
        case STYPE_CHOICE3:
            if (selectedOption < 0) selectedOption = 2;
            else if (selectedOption > 2) selectedOption = 0;
            switch (selectedOption)
            {
                case 0:
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItemSelected, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], cx, cy + 32, rootInfo.defFormatMenuItem, 0);
                    break;
                case 1:
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItemSelected, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], cx, cy + 32, rootInfo.defFormatMenuItem, 0);
                    break;
                case 2:
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], cx, cy + 32, rootInfo.defFormatMenuItemSelected, 0);
                    break;
            }
            break;
        case STYPE_CHOICE4:
            if (selectedOption < 0) selectedOption = 3;
            else if (selectedOption > 3) selectedOption = 0;
            switch (selectedOption)
            {
                case 0:
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItemSelected, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], cx, cy + 32, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 3], cx, cy + 48, rootInfo.defFormatMenuItem, 0);
                    break;
                case 1:
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItemSelected, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], cx, cy + 32, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 3], cx, cy + 48, rootInfo.defFormatMenuItem, 0);
                    break;
                case 2:
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], cx, cy + 32, rootInfo.defFormatMenuItemSelected, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 3], cx, cy + 48, rootInfo.defFormatMenuItem, 0);
                    break;
                case 3:
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText], cx, cy, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], cx, cy + 16, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], cx, cy + 32, rootInfo.defFormatMenuItem, 0);
                    WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 3], cx, cy + 48, rootInfo.defFormatMenuItemSelected, 0);
                    break;
            }
            break;
    }
}

void CommitChoice()
{
    short* varRef;
    switch (selectionType)
    {
        case STYPE_YNCHOICE:
            if (selectedOption) vmFlags |= VMFLAG_Z;
            else vmFlags &= ~(VMFLAG_Z);
            break;
        case STYPE_CHOICE2:
        case STYPE_CHOICE3:
        case STYPE_CHOICE4:
            varRef = GetVariableRef(selectedVar);
            if (varRef != 0)
            {
                *varRef = selectedOption;
            }
            break;
    }
    choiceBoxImgInfo->flags &= ~(IMAGE_DRAWREQ);
    EGCSetToMonochromeDrawMode();
    returnStatus &= ~(SCENE_STATUS_MAKING_CHOICE);
    ControlProcess(1);
}

void EndUserWait()
{
    curAsyncActions &= ~ASYNC_USER;
}


void SceneAsyncActionProcess()
{
    unsigned char ca = curAsyncActions;

    if (ca & ASYNC_PALETTE) //Palette animations
    {
        unsigned char atype = curAsyncPaletteAction;
        switch (atype)
        {
            case APAL_BFADEIN:
                break;
            case APAL_BFADEOUT:
                break;
            case APAL_WFADEIN:
                break;
            case APAL_WFADEOUT:
                break;
            case APAL_PFADEIN:
                break;
            case APAL_PFADEOUT:
                break;
            case APAL_PHUEROTATE:
                break;
            default:
                break;
        }
        ca &= ~ASYNC_PALETTE; //stub, TODO
    }

    if (ca & ASYNC_SCROLL) //Scroll animations
    {
        unsigned char atype = curAsyncScrollAction;
        switch (atype)
        {
            case ASCR_SHAKE:
            {
                long cval = Cos(curShakePoint);
                cval *= (long)curShakeAmp; //10.22 fixed point
                cval += 0x00200000;
                cval >>= 22; //rounded integer
                unsigned int sval = ((short)cval) + 400;
                GDCScrollSimpleGraphics(sval % 400);

                curShakePoint += curShakeAdv;
                long midAmp = ((long)curShakeAmp) * ((long)curShakeDampFactor); //10.22 fixed point
                curShakeAmp = (short)(midAmp >> 14); //back to 8.8 fixed point
                if (curShakeAmp < 0x0080) //end when shaking can't be seen anymore
                {
                    GDCScrollSimpleGraphics(0);
                    ca &= ~ASYNC_SCROLL;
                }
            }
                break;
            default:
                break;
        }
    }

    if (ca == 0) ControlProcess(1);

    curAsyncActions = ca;
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
                ControlProcess(0);
                returnStatus |= SCENE_STATUS_FINALEND;
                break;
            }
            result = LoadNewScene(sNum);
            if (result)
            {
                returnStatus = SCENE_STATUS_ERROR | result;
                ControlProcess(0);
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
        case 0x08: //palsetcol
            curSceneDataPC += 2; //stub, TODO
            break;
        case 0x09: //paladdcol
            curSceneDataPC += 2; //stub, TODO
            break;
        case 0x0A: //palsetlum
            curSceneDataPC += 1; //stub, TODO
            break;
        case 0x0B: //palsetsat
            curSceneDataPC += 1; //stub, TODO
            break;
        case 0x0C: //palsethue
            curSceneDataPC += 1; //stub, TODO
            break;
        case 0x0D: //palcolourise
            curSceneDataPC += 2; //stub, TODO
            break;
        case 0x0E: //palinvert
            break; //stub, TODO
        case 0x0F: //nowait
            break; //stub, TODO
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
            StartAnimatedStringToWrite(sceneTextBuffer + curTextArray[nextTextNum], textBoxInnerBounds.pos.x, textBoxInnerBounds.pos.y, rootInfo.defFormatNormal);
            nextTextNum++;
            vmFlags |= VMFLAG_TEXTINBOX;
            ControlProcess(0);
            curAsyncActions |= ASYNC_USER;
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
                    charNameBoxImgInfo->flags |= IMAGE_DRAWREQ;
                    DoDrawRequests();
                    if (result)
                    {
                        returnStatus = SCENE_STATUS_ERROR | result;
                        ControlProcess(0);
                        break;
                    }
                    WriteString(curCharName, charNameBoxInnerBounds.pos.x, charNameBoxInnerBounds.pos.y, rootInfo.defFormatCharName, 0);
                }
                else
                {
                    charNameBoxImgInfo->flags &= ~(IMAGE_DRAWREQ);
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
            ControlProcess(0);
            returnStatus |= SCENE_STATUS_WIPETEXT; //for later, when text box wiping involves an animation
            break;
        case 0x14: //ynchoice
            selectedOption = 1;
            selectionType = STYPE_YNCHOICE;
            ControlProcess(0);
            curAsyncActions |= ASYNC_USER;
            returnStatus |= SCENE_STATUS_MAKING_CHOICE;
            choiceBoxInnerBounds.size.y = 32;
            choiceBoxImgInfo->boundRect.size.y = 64;
            choiceBoxImgInfo-> flags |= IMAGE_DRAWREQ;
            DoDrawRequests();
            WriteString("Yes", choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y, rootInfo.defFormatMenuItemSelected, 0);
            WriteString("No", choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y + 16, rootInfo.defFormatMenuItem, 0);
            break;
        case 0x15: //choice2
            selectedOption = 0;
            selectionType = STYPE_CHOICE2;
            ControlProcess(0);
            curAsyncActions |= ASYNC_USER;
            returnStatus |= SCENE_STATUS_MAKING_CHOICE;
            selectedVar = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            selectedFirstText = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            choiceBoxInnerBounds.size.y = 32;
            choiceBoxImgInfo->boundRect.size.y = 64;
            choiceBoxImgInfo-> flags |= IMAGE_DRAWREQ;
            DoDrawRequests();
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y, rootInfo.defFormatMenuItemSelected, 0);
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y + 16, rootInfo.defFormatMenuItem, 0);
            break;
        case 0x16: //choice3
            selectedOption = 0;
            selectionType = STYPE_CHOICE3;
            ControlProcess(0);
            curAsyncActions |= ASYNC_USER;
            returnStatus |= SCENE_STATUS_MAKING_CHOICE;
            selectedVar = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            selectedFirstText = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            choiceBoxInnerBounds.size.y = 48;
            choiceBoxImgInfo->boundRect.size.y = 80;
            choiceBoxImgInfo-> flags |= IMAGE_DRAWREQ;
            DoDrawRequests();
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y, rootInfo.defFormatMenuItemSelected, 0);
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y + 16, rootInfo.defFormatMenuItem, 0);
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y + 32, rootInfo.defFormatMenuItem, 0);
            break;
        case 0x17: //choice4
            selectedOption = 0;
            selectionType = STYPE_CHOICE4;
            ControlProcess(0);
            curAsyncActions |= ASYNC_USER;
            returnStatus |= SCENE_STATUS_MAKING_CHOICE;
            selectedVar = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            selectedFirstText = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
            choiceBoxInnerBounds.size.y = 64;
            choiceBoxImgInfo->boundRect.size.y = 96;
            choiceBoxImgInfo-> flags |= IMAGE_DRAWREQ;
            DoDrawRequests();
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y, rootInfo.defFormatMenuItemSelected, 0);
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 1], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y + 16, rootInfo.defFormatMenuItem, 0);
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 2], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y + 32, rootInfo.defFormatMenuItem, 0);
            WriteString(sceneTextBuffer + curTextArray[selectedFirstText + 3], choiceBoxInnerBounds.pos.x, choiceBoxInnerBounds.pos.y + 48, rootInfo.defFormatMenuItem, 0);
            break;
        case 0x18: //bfadein
            curAsyncActions |= ASYNC_PALETTE;
            curAsyncPaletteAction = APAL_BFADEIN;
            curSceneDataPC += 1; //stub, TODO
            goto vmDecideWait;
        case 0x19: //bfadeout
            curAsyncActions |= ASYNC_PALETTE;
            curAsyncPaletteAction = APAL_BFADEOUT;
            curSceneDataPC += 1; //stub, TODO
            goto vmDecideWait;
        case 0x1A: //wfadein
            curAsyncActions |= ASYNC_PALETTE;
            curAsyncPaletteAction = APAL_WFADEIN;
            curSceneDataPC += 1; //stub, TODO
            goto vmDecideWait;
        case 0x1B: //wfadeout
            curAsyncActions |= ASYNC_PALETTE;
            curAsyncPaletteAction = APAL_WFADEOUT;
            curSceneDataPC += 1; //stub, TODO
            goto vmDecideWait;
        case 0x1C: //pfadein
            curAsyncActions |= ASYNC_PALETTE;
            curAsyncPaletteAction = APAL_PFADEIN;
            curSceneDataPC += 1; //stub, TODO
            goto vmDecideWait;
        case 0x1D: //pfadeout
            curAsyncActions |= ASYNC_PALETTE;
            curAsyncPaletteAction = APAL_PFADEOUT;
            curSceneDataPC += 1; //stub, TODO
            goto vmDecideWait;
        case 0x1E: //phuerotate
            curAsyncActions |= ASYNC_PALETTE;
            curAsyncPaletteAction = APAL_PHUEROTATE;
            curSceneDataPC += 1; //stub, TODO
            goto vmDecideWait;
        case 0x1F: //shake
            curAsyncActions |= ASYNC_SCROLL;
            curAsyncScrollAction = ASCR_SHAKE;
            {
                unsigned short arg = *((unsigned short*)(curSceneData + curSceneDataPC));
                unsigned short amp = arg & 0x003F;
                curShakeAmp = amp << 8;
                unsigned short period = (arg >> 6) & 0x001F;
                curShakeAdv = (1 << 14)/(period+1);
                unsigned short damp = (arg >> 11) & 0x001F;
                curShakeDampFactor = (1 << 14) - (damp << 6);
                curShakePoint = 0;
                curSceneDataPC += 2;
            }
            vmDecideWait:
            if (*(curSceneData + curSceneDataPC) == 0x0F) curSceneDataPC += 1; //Next instruction is nowait -> do not halt processing
            else ControlProcess(0); //Else, halt it until the async action is complete
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
