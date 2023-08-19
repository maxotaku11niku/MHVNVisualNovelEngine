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

const unsigned char stdShadowCols[16] = { 0x0, 0x0, 0x0, 0x1, 0x0, 0x4, 0x2, 0x6, 0x0, 0x0, 0x9, 0x3, 0x0, 0x5, 0xC, 0x8 };

unsigned long oldVsyncVector;
unsigned char oldInterruptMask;
volatile unsigned char vsynced = 0;

const char* magicNumber = "MHVN";

typedef struct
{
	unsigned short VNFlags;
	unsigned short numstvar;
	unsigned short defFormatNormal;
	unsigned short defFormatCharName;
	unsigned short defFormatMenuItem;
	unsigned short defFormatMenuItemSelected;
	unsigned short numLang;
	unsigned short curLang;
	char sceneDataPath[13];
	char langDataPath[13];
	char curTextDataPath[13];
	char BGDataPath[13];
	char spriteDataPath[13];
	char musicDataPath[13];
	char sfxDataPath[13];
	char systemDataPath[13];
} RootInfo;

typedef struct
{
	unsigned short numScenes;
	unsigned short numChars;
	unsigned long curSceneFilePtr;
	unsigned short curScene;
} SceneInfo;

typedef struct
{
	unsigned long systemTextFilePtr;
	unsigned long creditsTextFilePtr;
	unsigned long characterNamesFilePtr;
	unsigned long sceneTextFilePtr;
	unsigned long CGTextFilePtr;
	unsigned long musicTextFilePtr;
	unsigned short curNumSceneText;
} TextInfo;



RootInfo rootInfo;
SceneInfo sceneInfo;
TextInfo textInfo;

int sdHandle;
int ldHandle;
int ctHandle;
int bgHandle;
int sprHandle;
int mdHandle;
int sfxHandle;
int sysHandle;

unsigned char smallFileBuffer[1024];
unsigned char curSceneData[1024];
unsigned short curSceneDataPC;
char curCharName[64];
char* curTextArray[256];

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
	
	memset8Seg(0, &rootInfo, sizeof(RootInfo)); //Zero out the root info structure
	int rinfHandle;
	int realReadLen;
	int result = openFile("ROOTINFO.DAT", FILE_OPEN_READ, &rinfHandle);
	if (result)
	{
		writeString("Error! Could not find ROOTINFO.DAT!", 180, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF));
		goto errorquit; //Error handler
	}
	readFile(rinfHandle, 0x7C, smallFileBuffer, &realReadLen);
	for (int i = 0; i < 4; i++)
	{
		if (smallFileBuffer[i] != magicNumber[i])
		{
			writeString("Error! ROOTINFO.DAT is not valid!", 188, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF));
			result = 0xFF;
			goto errorquit; //Wrong file format error
		}
	}
	//Read in all root info
	rootInfo.VNFlags = *((unsigned short*)(smallFileBuffer + 0x04));
	rootInfo.numstvar = *((unsigned short*)(smallFileBuffer + 0x06));
	rootInfo.defFormatNormal = *((unsigned short*)(smallFileBuffer + 0x08));
	rootInfo.defFormatCharName = *((unsigned short*)(smallFileBuffer + 0x0A));
	rootInfo.defFormatMenuItem = *((unsigned short*)(smallFileBuffer + 0x0C));
	rootInfo.defFormatMenuItemSelected = *((unsigned short*)(smallFileBuffer + 0x0E));
	for (int i = 0; i < 12; i++)
	{
		rootInfo.sceneDataPath[i] = smallFileBuffer[0x10 + i];
		rootInfo.langDataPath[i] = smallFileBuffer[0x1C + i];
		rootInfo.BGDataPath[i] = smallFileBuffer[0x28 + i];
		rootInfo.spriteDataPath[i] = smallFileBuffer[0x34 + i];
		rootInfo.musicDataPath[i] = smallFileBuffer[0x40 + i];
		rootInfo.sfxDataPath[i] = smallFileBuffer[0x4C + i];
		rootInfo.systemDataPath[i] = smallFileBuffer[0x58 + i];
	}
	closeFile(rinfHandle);
	//Read in current text data file path from langauge data descriptor file (number currently hardcoded)
	result = openFile(rootInfo.langDataPath, FILE_OPEN_READ, &ldHandle);
	if (result)
	{
		writeString("Error! Could not find language data file!", 156, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF));
		goto errorquit; //Error handler
	}
	readFile(ldHandle, 1024, smallFileBuffer, &realReadLen);
	rootInfo.numLang = *((unsigned short*)(smallFileBuffer));
	rootInfo.curLang = 0;
	char* tdPath = smallFileBuffer + smallFileBuffer[2 + 4 * rootInfo.curLang];
	for (int i = 0; i < 12; i++)
	{
		const char ch = tdPath[i];
		if (ch) rootInfo.curTextDataPath[i] = ch;
		else break;
	}
	closeFile(ldHandle);
	//Read in current scene data from file
	result = openFile(rootInfo.sceneDataPath, FILE_OPEN_READ, &sdHandle);
	if (result)
	{
		writeString("Error! Could not find scene data file!", 168, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF));
		goto errorquit; //Error handler
	}
	readFile(sdHandle, 8, smallFileBuffer, &realReadLen);
	sceneInfo.numScenes = *((unsigned short*)(smallFileBuffer));
	sceneInfo.numChars = *((unsigned short*)(smallFileBuffer + 0x02));
	sceneInfo.curSceneFilePtr = *((unsigned long*)(smallFileBuffer + 0x04));
	sceneInfo.curScene = 0;
	readFile(sdHandle, 1024, curSceneData, &realReadLen);
	closeFile(sdHandle); //Close for now since we only expect the test to have one scene.
	//Read in current text data from file
	result = openFile(rootInfo.curTextDataPath, FILE_OPEN_READ, &ctHandle);
	if (result)
	{
		writeString("Error! Could not find text data file!", 172, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF));
		goto errorquit; //Error handler
	}
	readFile(ctHandle, 1024, smallFileBuffer, &realReadLen);
	textInfo.systemTextFilePtr = *((unsigned long*)(smallFileBuffer));
	textInfo.creditsTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x04));
	textInfo.characterNamesFilePtr = *((unsigned long*)(smallFileBuffer + 0x08));
	textInfo.sceneTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x0C));
	textInfo.CGTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x10));
	textInfo.musicTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x14));
	memcpy32Seg(smallFileBuffer + textInfo.characterNamesFilePtr + (2 * sceneInfo.numChars), curCharName, 16); //Don't worry, null termination to the rescue!
	textInfo.curNumSceneText = *((unsigned short*)(smallFileBuffer + textInfo.sceneTextFilePtr + (4 * sceneInfo.numScenes)));
	char* startOfText = smallFileBuffer + textInfo.sceneTextFilePtr + (4 * sceneInfo.numScenes) + 2 * (textInfo.curNumSceneText + 1);
	unsigned short* textPtrs = (unsigned short*)(smallFileBuffer + textInfo.sceneTextFilePtr + (4 * sceneInfo.numScenes) + 2);
	for (int i = 0; i < textInfo.curNumSceneText; i++)
	{
		curTextArray[i] = startOfText + textPtrs[i];
	}
	closeFile(ctHandle); //Close for now since we only expect the test to have one scene.
	
	setCustomInfo(0, "Player"); //For testing
	writeString(curCharName, 60, 60, rootInfo.defFormatCharName);
	
	oldInterruptMask = getPrimaryInterruptMask();
	intsoff();
	oldVsyncVector = getInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC);
	setInterruptFunction(INTERRUPT_VECTOR_VSYNC, vsyncInterrupt);
	addPrimaryInterrupts(INTERRUPT_MASK_VSYNC);
	intson();
	
	startAnimatedStringToWrite(curTextArray[0], 64, 128, rootInfo.defFormatNormal);
	
	int hasFinshedStringAnim = 0;
	int textSkip = 0;
	int strnum = 0;
	while (1)
	{
		while (1) //Wait for vsync
		{
			if(vsynced) break;
		}
		if (!hasFinshedStringAnim)
		{
			hasFinshedStringAnim = stringWriteAnimationFrame(textSkip);
		}
		else textSkip = 0;
		vsynced = 0;
		if (key_pressed(K_ENTER)) //Next lines
		{
			if (hasFinshedStringAnim)
			{
				clearLinesEGC(128, 65);
				egc_patdatandreadmode(EGC_PATTERNSOURCE_FGCOLOUR);
				egc_rwmode(EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP((EGC_ROP_SRC & EGC_ROP_PAT) | ((~EGC_ROP_SRC) & EGC_ROP_DST)));
				egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
				egc_bitlen(32);
				strnum++;
				strnum %= textInfo.curNumSceneText;
				if (!strnum) break;
				startAnimatedStringToWrite(curTextArray[strnum], 64, 128, rootInfo.defFormatNormal);
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