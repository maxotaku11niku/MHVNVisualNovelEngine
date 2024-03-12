#include "platform/x86strops.h"
#include "platform/x86interrupt.h"
//#include "platform/filehandling.h"
//#include "platform/memalloc.h"
//#include "platform/unreal_interrupts.h"
//#include "platform/unrealhwaddr.h"
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
    TextOff();
    GraphicsOn();
    GraphicsSetMode(CRT_MODE_GRAPHIC_PAGE0 | CRT_MODE_GRAPHIC_COLOUR | CRT_MODE_GRAPHIC_640x400);
    GraphicsSetMode2(GDC_MODE2_16COLOURS);
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
    egc_planeaccess(0xF);
    egc_mask(0xFFFF);
    egc_bgcolour(0xC);
    ClearScreenEGC();
    SetEGCToMonochromeDrawMode();
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
    
    oldInterruptMask = GetPrimaryInterruptMask();
    intsoff();
    oldVsyncVector = GetInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC);
    SetInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC, VsyncInterrupt);
    AddPrimaryInterrupts(INTERRUPT_MASK_VSYNC);
    intson();

    textBoxlX = 80;
    textBoxtY = 288;
    textBoxrX = 560;
    textBoxbY = 352;
    unsigned char hasFinshedStringAnim = 0;
    unsigned char textSkip = 0;
    int sceneProcessResult;
    vsynced = 0;
    LoadTextBoxIntoVRAM();
    DrawTextBox(textBoxlX-16, textBoxtY-16, textBoxrX - textBoxlX + 32, textBoxbY - textBoxtY + 32);
    gdc_interruptreset(); //Prevents a spinlock
    while (1)
    {
        while (1) //Wait for vsync
        {
            __asm ("hlt"); //Save energy
            if(vsynced) break;
        }
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
            if (hasFinshedStringAnim)
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
        if (key_pressed(K_ESC)) //Quit
        {
            break;
        }
        UpdatePrevKeyStatus();
    }
    intsoff();
    SetPrimaryInterruptMask(oldInterruptMask);
    SetInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC, oldVsyncVector);
    intson();
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
    egc_bgcolour(0x0);
    ClearScreenEGC();
    EGCDisable();
    GraphicsOff();
    TextOn();
    FreeSceneEngine();
    return result;
}
