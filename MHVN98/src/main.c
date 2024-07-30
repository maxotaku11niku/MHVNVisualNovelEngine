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
 * Entry point and main loop
 */

#include "platform/x86interrupt.h"
#include "platform/doscalls.h"
#include "platform/pc98_gdc.h"
#include "platform/pc98_egc.h"
#include "platform/pc98_keyboard.h"
#include "platform/pc98_interrupt.h"
#include "unicode.h"
#include "isr.h"
#include "textengine.h"
#include "rootinfo.h"
#include "sceneengine.h"
#include "palette.h"
#include "graphics.h"

const unsigned char stdShadowCols[16] = { 0x0, 0x0, 0x0, 0x2, 0x0, 0x4, 0x5, 0x0, 0x0, 0x8, 0x9, 0x7, 0x0, 0xC, 0xD, 0x1 };

InterruptFuncPtr oldVsyncVector;
unsigned char oldInterruptMask;

int CheckCompatibility()
{
    //Check if we are running on a PC-98. MHVN98 only runs on a PC-98.
    unsigned int outres;
    __asm volatile (
        "mov $0x1000, %0\n\t"
        "clc\n\t" //According to a contributor to doslib, some IBM-compatible PC BIOSes set the carry flag on return from an invalid software interrupt
        "mov $0x1000, %%ax\n\t" //Those same BIOSes might return with ax != 0x1000, so we must account for them
        "int $0x1A\n\t" //Printer BIOS software interrupt vector (on a PC-98, usually invalid on other machines)
        "jc .buggyIBM%=\n\t" //We account for those weird BIOSes here (other BIOSes leave things alone, the PC-98 BIOS mucks around with ax but not the carry flag)
        "mov %%ax, %0\n\t"
        ".buggyIBM%=:"
    : "=rm" (outres) : : "%ax");
    if (outres == 0x1000)
    {
        DOSConsoleWriteString("Unsupported platform! MHVN98 only runs on a PC-98!\n$");
        return 0; //Not a PC-98
    }

    //Check if the PC-98 we are running on supports the EGC (which we absolutely need)
    GDCSetMode2(GDC_MODE2_16COLOURS); //Try to set 16 colour mode. All PC-98s with an EGC support at least this.
    *((volatile unsigned __far short*)GDC_PLANE0) = 0xAAAA;
    *((volatile unsigned __far short*)GDC_PLANE1) = 0x6969;
    *((volatile unsigned __far short*)GDC_PLANE2) = 0x3901;
    *((volatile unsigned __far short*)GDC_PLANE3) = 0xCAFE; //Write something to VRAM
    //We're going to do something that the EGC can do that the GRCG cannot, just to be absolutely sure
    EGCEnable();
    EGCSetPlaneAccess(0xF);
    EGCSetMask(0xFFFF);
    EGCSetPatternAndReadSource(EGC_PATTERNSOURCE_BGCOLOUR);
    EGCSetReadWriteMode(EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP(EGC_ROP_SRC ^ EGC_ROP_DST)); //Just XOR with the source from the CPU
    EGCSetBitAddressTransferDirection(EGC_BLOCKTRANSFER_FORWARD);
    EGCSetBitLength(16);
    *((volatile unsigned __far short*)GDC_PLANES) = 0xFEED; //Should affect all planes at once if the EGC is on

    __asm volatile ( //Short busy loop just to make sure the EGC has done its job before we check the results
        "mov $8, %%ax\n\t"
        ".busy%=: dec %%ax\n\t"
        "jnz .busy%=\n\t"
    : : : "%ax");

    EGCDisable(); //Helps properly check what happened to VRAM

    __asm volatile ( //Short busy loop just to make sure the EGC has done its job before we check the results
        "mov $8, %%ax\n\t"
        ".busy%=: dec %%ax\n\t"
        "jnz .busy%=\n\t"
    : : : "%ax");

    unsigned short p0 = *((volatile unsigned __far short*)GDC_PLANE0);
    unsigned short p1 = *((volatile unsigned __far short*)GDC_PLANE1);
    unsigned short p2 = *((volatile unsigned __far short*)GDC_PLANE2);
    unsigned short p3 = *((volatile unsigned __far short*)GDC_PLANE3);
    if (p0 != (0xAAAA ^ 0xFEED) || p1 != (0x6969 ^ 0xFEED) || p2 != (0x3901 ^ 0xFEED) || p3 != (0xCAFE ^ 0xFEED))
    {
        DOSConsoleWriteString("Unsupported platform! EGC is required!\n$");
        return 0;
    }

    //TODO: Check for optional features that we make use of

    return 1; //Everything is OK
}

int main(void)
{
    int result = CheckCompatibility();
    if (!result) return 0xFF; //Quit immediately on unsupported platforms

    //Set up graphics first
    GDCSetDisplayMode(640, 400, 440);
    GDCStopText();
    GDCStartGraphics();
    GDCSetGraphicsLineScale(1);
    GDCSetMode1(GDC_MODE1_LINEDOUBLE_ON);
    GDCSetMode1(GDC_MODE1_COLOUR);
    GDCSetGraphicsDisplayPage(0);
    GDCSetGraphicsDrawPage(0);
    GDCSetMode2(GDC_MODE2_16COLOURS);
    GDCSetDisplayRegion(0x0000, 400);
    GDCScrollSimpleGraphics(0);
    SetDefaultPalette();
    EGCEnable();
    EGCSetPlaneAccess(0xF);
    EGCSetMask(0xFFFF);
    EGCSetBGColour(0xC);
    EGCClearScreen();
    EGCSetToMonochromeDrawMode();
    SetShadowColours(stdShadowCols);
    
    result = ReadInRootInfo();
    if (result) goto errorquit;
    InitFontFile(); //If it fails, it fails. Not a fatal error, and if it happens it would be very obvious.
    result = InitLanguage(0);
    if (result) goto errorquit;
    result = SetupTextInfo();
    if (result) goto errorquit;
    result = SetupSceneEngine();
    if (result) goto errorquit;
    result = InitialiseGraphicsSystem();
    if (result) goto errorquit;

    SetCustomInfo(0, "Player"); //For testing
    
    oldInterruptMask = PC98GetPrimaryInterruptMask();
    intsoff();
    oldVsyncVector = GetInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC);
    SetInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC, VsyncInterrupt);
    PC98AddPrimaryInterrupts(INTERRUPT_MASK_VSYNC);
    intson();

    textBoxInnerBounds.pos.x = 80;
    textBoxInnerBounds.pos.y = 288;
    textBoxInnerBounds.size.x = 480;
    textBoxInnerBounds.size.y = 64;
    Rect2Int textBoxOuterBounds;
    textBoxOuterBounds.pos.x = textBoxInnerBounds.pos.x - 16;
    textBoxOuterBounds.pos.y = textBoxInnerBounds.pos.y - 16;
    textBoxOuterBounds.size.x = textBoxInnerBounds.size.x + 32;
    textBoxOuterBounds.size.y = textBoxInnerBounds.size.y + 32;
    textBoxImgInfo = RegisterTextBox(&textBoxOuterBounds);
    textBoxImgInfo->flags |= IMAGE_DRAWREQ;

    charNameBoxInnerBounds.pos.x = 80;
    charNameBoxInnerBounds.pos.y = 240;
    charNameBoxInnerBounds.size.x = 160;
    charNameBoxInnerBounds.size.y = 16;
    Rect2Int charNameBoxOuterBounds;
    charNameBoxOuterBounds.pos.x = charNameBoxInnerBounds.pos.x - 16;
    charNameBoxOuterBounds.pos.y = charNameBoxInnerBounds.pos.y - 16;
    charNameBoxOuterBounds.size.x = charNameBoxInnerBounds.size.x + 32;
    charNameBoxOuterBounds.size.y = charNameBoxInnerBounds.size.y + 32;
    charNameBoxImgInfo = RegisterCharNameBox(&charNameBoxOuterBounds);

    choiceBoxInnerBounds.pos.x = 336;
    choiceBoxInnerBounds.pos.y = 160;
    choiceBoxInnerBounds.size.x = 256;
    choiceBoxInnerBounds.size.y = 64;
    Rect2Int choiceBoxOuterBounds;
    choiceBoxOuterBounds.pos.x = choiceBoxInnerBounds.pos.x - 16;
    choiceBoxOuterBounds.pos.y = choiceBoxInnerBounds.pos.y - 16;
    choiceBoxOuterBounds.size.x = choiceBoxInnerBounds.size.x + 32;
    choiceBoxOuterBounds.size.y = choiceBoxInnerBounds.size.y + 32;
    choiceBoxImgInfo = RegisterChoiceBox(&choiceBoxOuterBounds);

    unsigned char hasFinshedStringAnim = 0;
    unsigned char textSkip = 0;
    int sceneProcessResult;
    vsynced = 0;
    GDCInterruptReset(); //Prevents a spinlock
    while (1)
    {
        while (1) //Wait for vsync
        {
            __asm ("hlt"); //Save energy
            if(vsynced) break;
        }
        DoDrawRequests();
        sceneProcessResult = SceneDataProcess();
        SceneAsyncActionProcess();
        if (sceneProcessResult & SCENE_STATUS_ERROR)
        {
            result = sceneProcessResult & (~SCENE_STATUS_ERROR);
            break;
        }
        else if (sceneProcessResult & SCENE_STATUS_FINALEND) break;
        else if (sceneProcessResult & SCENE_STATUS_WIPETEXT)
        {
            ControlProcess(1);
        }
        else if (!hasFinshedStringAnim && (sceneProcessResult & SCENE_STATUS_RENDERTEXT))
        {
            hasFinshedStringAnim = StringWriteAnimationFrame(textSkip);
        }
        else
        {
            textSkip = 0;
        }
        vsynced = 0;
        if (key_pressed(K_ENTER)) //Next lines
        {
            if (sceneProcessResult & SCENE_STATUS_MAKING_CHOICE)
            {
                CommitChoice();
                EndUserWait();
            }
            else if (hasFinshedStringAnim)
            {
                ControlProcess(1);
                EndUserWait();
                hasFinshedStringAnim = 0;
                textSkip = 0;
            }
            else
            {
                textSkip = 1;
            }
        }
        if (key_pressed(K_UP)) //Select up
        {
            if (sceneProcessResult & SCENE_STATUS_MAKING_CHOICE)
            {
                SwitchChoice(SELECT_UP);
            }
        }
        if (key_pressed(K_DOWN)) //Select down
        {
            if (sceneProcessResult & SCENE_STATUS_MAKING_CHOICE)
            {
                SwitchChoice(SELECT_DOWN);
            }
        }
        if (key_pressed(K_ESC)) //Quit
        {
            break;
        }
        UpdatePrevKeyStatus();
    }
    intsoff();
    PC98SetPrimaryInterruptMask(oldInterruptMask);
    SetInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC, oldVsyncVector);
    intson();
    FreeGraphicsSystem();
    errorquit:
    if (result)
    {
        WriteString("Press Enter to quit.", 240, 200, FORMAT_SHADOW | FORMAT_COLOUR(0xF), 0);
        while (!key_is_down(K_ENTER))
        {
            UpdatePrevKeyStatus();
        }
    }
    GDCSetPaletteColour(0x0, 0x0, 0x0, 0x0);
    EGCSetBGColour(0x0);
    EGCClearScreen();
    EGCDisable();
    GDCSetDisplayMode(640, 400, 440);
    GDCStartText();
    GDCStopGraphics();
    FreeSceneEngine();
    return result;
}
