#include "x86strops.h"
#include "pc98_crtbios.h"
#include "pc98_gdc.h"
#include "pc98_egc.h"
#include "unreal_keyboard.h"
#include "pc98_interrupt.h"
#include "isr.h"
#include "unrealhwaddr.h"
#include "textengine.h"
#include "filehandling.h"
#include "unreal_interrupts.h"
#include "rootinfo.h"
#include "sceneengine.h"

const unsigned char stdShadowCols[16] = { 0x0, 0x0, 0x0, 0x1, 0x0, 0x4, 0x2, 0x6, 0x0, 0x0, 0x9, 0x3, 0x0, 0x5, 0xC, 0x8 };

unsigned long oldVsyncVector;
unsigned char oldInterruptMask;
volatile unsigned char vsynced = 0;

int bgHandle;
int sprHandle;
int mdHandle;
int sfxHandle;
int sysHandle;

int realMain(void)
{
	//Set up graphics first
	textOff();
	graphicsOn();
	graphicsSetMode(CRT_MODE_GRAPHIC_PAGE0 | CRT_MODE_GRAPHIC_COLOUR | CRT_MODE_GRAPHIC_640x400);
	graphicsSetMode2(GDC_16COLOURS);
	gdc_setpalettecolour(0x0, 0x1, 0x1, 0x1);
	gdc_setpalettecolour(0x1, 0x7, 0x0, 0x0);
	gdc_setpalettecolour(0x2, 0x2, 0x7, 0x3);
	gdc_setpalettecolour(0x3, 0xD, 0x4, 0x4);
	gdc_setpalettecolour(0x4, 0x3, 0x3, 0xB);
	gdc_setpalettecolour(0x5, 0x3, 0xA, 0xF);
	gdc_setpalettecolour(0x6, 0x5, 0xD, 0x5);
	gdc_setpalettecolour(0x7, 0x8, 0xF, 0x5);
	gdc_setpalettecolour(0x8, 0x7, 0x7, 0x7);
	gdc_setpalettecolour(0x9, 0xB, 0x3, 0xB);
	gdc_setpalettecolour(0xA, 0xF, 0x7, 0xF);
	gdc_setpalettecolour(0xB, 0xF, 0xB, 0x7);
	gdc_setpalettecolour(0xC, 0xC, 0xB, 0x3);
	gdc_setpalettecolour(0xD, 0x9, 0xF, 0xF);
	gdc_setpalettecolour(0xE, 0xF, 0xF, 0x6);
	gdc_setpalettecolour(0xF, 0xF, 0xF, 0xF);
	egcEnable();
	egc_planeaccess(0xF);
	egc_mask(0xFFFF);
	egc_bgcolour(0x4);
	clearScreenEGC();
	egc_patdatandreadmode(EGC_PATTERNSOURCE_FGCOLOUR);
	egc_rwmode(EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP((EGC_ROP_SRC & EGC_ROP_PAT) | ((~EGC_ROP_SRC) & EGC_ROP_DST)));
	egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
	egc_bitlen(32);
	setShadowColours(stdShadowCols);
	
	int result = readInRootInfo();
	if (result) goto errorquit;
	result = initLanguage(0);
	if (result) goto errorquit;
	result = setupTextInfo();
	if (result) goto errorquit;
	result = setupSceneEngine();
	if (result) goto errorquit;

	setCustomInfo(0, "Player"); //For testing
	
	oldInterruptMask = getPrimaryInterruptMask();
	intsoff();
	oldVsyncVector = getInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC);
	setInterruptFunction(INTERRUPT_VECTOR_VSYNC, vsyncInterrupt);
	addPrimaryInterrupts(INTERRUPT_MASK_VSYNC);
	intson();
	
	int hasFinshedStringAnim = 0;
	int textSkip = 0;
	int strnum = 0;
	int sceneProcessResult;
	while (1)
	{
		while (1) //Wait for vsync
		{
			if(vsynced) break;
		}
		sceneProcessResult = sceneDataProcess();
		if (sceneProcessResult & SCENE_STATUS_ERROR)
		{
			result = sceneProcessResult & (~SCENE_STATUS_ERROR);
			break;
		}
		else if (sceneProcessResult & SCENE_STATUS_FINALEND) break;
		else if (sceneProcessResult & SCENE_STATUS_WIPETEXT)
		{
			controlProcess(1);
		}
		else if (!hasFinshedStringAnim && (sceneProcessResult & SCENE_STATUS_RENDERTEXT))
		{
			hasFinshedStringAnim = stringWriteAnimationFrame(textSkip);
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
				controlProcess(1);
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
		updatePrevKeyStatus();
	}
	intsoff();
	setPrimaryInterruptMask(oldInterruptMask);
	setInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC, oldVsyncVector);
	intson();
	errorquit:
	if (result)
	{
		writeString("Press Enter to quit.", 240, 200, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF));
		while (!key_is_down(K_ENTER))
		{
			updatePrevKeyStatus();
		}
	}
	gdc_setpalettecolour(0x0, 0x0, 0x0, 0x0);
	egc_bgcolour(0x0);
	clearScreenEGC();
	egcDisable();
	graphicsOff();
	textOn();
	return result;
}