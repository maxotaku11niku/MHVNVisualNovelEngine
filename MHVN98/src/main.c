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
#include "platform/pc98_crtbios.h"
#include "platform/pc98_gdc.h"
#include "platform/pc98_egc.h"
#include "platform/pc98_keyboard.h"
#include "platform/pc98_interrupt.h"
#include "isr.h"
#include "textengine.h"
#include "rootinfo.h"
#include "sceneengine.h"
#include "graphics.h"

const unsigned char stdShadowCols[16] = { 0x0, 0x0, 0x0, 0x2, 0x0, 0x4, 0x5, 0x0, 0x0, 0x8, 0x9, 0x7, 0x0, 0xC, 0xD, 0x1 };

InterruptFuncPtr oldVsyncVector;
unsigned char oldInterruptMask;

int main(void)
{
    //Set up graphics first
    PC98BIOSTextOff();
    PC98BIOSGraphicsOn();
    PC98BIOSGraphicsSetMode(CRT_MODE_GRAPHIC_PAGE0 | CRT_MODE_GRAPHIC_COLOUR | CRT_MODE_GRAPHIC_640x400);
    GDCSetMode2(GDC_MODE2_16COLOURS);
    GDCSetPaletteColour(0x0, 0x1, 0x1, 0x1);
    GDCSetPaletteColour(0x1, 0x7, 0x7, 0x7);
    GDCSetPaletteColour(0x2, 0xB, 0x3, 0xB);
    GDCSetPaletteColour(0x3, 0xF, 0x7, 0xF);
    GDCSetPaletteColour(0x4, 0x7, 0x1, 0x1);
    GDCSetPaletteColour(0x5, 0xD, 0x4, 0x4);
    GDCSetPaletteColour(0x6, 0xF, 0xB, 0x7);
    GDCSetPaletteColour(0x7, 0xC, 0xB, 0x3);
    GDCSetPaletteColour(0x8, 0x2, 0x7, 0x3);
    GDCSetPaletteColour(0x9, 0x5, 0xD, 0x5);
    GDCSetPaletteColour(0xA, 0x8, 0xF, 0x5);
    GDCSetPaletteColour(0xB, 0xF, 0xF, 0x6);
    GDCSetPaletteColour(0xC, 0x3, 0x3, 0xB);
    GDCSetPaletteColour(0xD, 0x3, 0xA, 0xF);
    GDCSetPaletteColour(0xE, 0x9, 0xF, 0xF);
    GDCSetPaletteColour(0xF, 0xF, 0xF, 0xF);
    EGCEnable();
    EGCSetPlaneAccess(0xF);
    EGCSetMask(0xFFFF);
    EGCSetBGColour(0xC);
    EGCClearScreen();
    EGCSetToMonochromeDrawMode();
    SetShadowColours(stdShadowCols);
    
    int result = ReadInRootInfo();
    if (result) goto errorquit;
    result = InitLanguage(0);
    if (result) goto errorquit;
    result = SetupTextInfo();
    if (result) goto errorquit;
    result = SetupSceneEngine();
    if (result) goto errorquit;

    SetCustomInfo(0, "Player"); //For testing
    
    oldInterruptMask = PC98GetPrimaryInterruptMask();
    intsoff();
    oldVsyncVector = GetInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC);
    SetInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC, VsyncInterrupt);
    PC98AddPrimaryInterrupts(INTERRUPT_MASK_VSYNC);
    intson();

    InitialiseGraphicsSystem();
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
            }
            else if (hasFinshedStringAnim)
            {
                ControlProcess(1);
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
    PC98BIOSGraphicsOff();
    PC98BIOSTextOn();
    FreeSceneEngine();
    return result;
}
