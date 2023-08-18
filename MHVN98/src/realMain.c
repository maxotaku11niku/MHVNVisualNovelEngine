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

const char* testStrings[8] =
{
	"Well, would you look at that, we've got ourselves\r\na good bunch of text in front of us.",
	"Believe it or not, we can change the\x1B\x4F\r\ncolour\x1B\xFF of the text!",
	"Or perhaps you'd like a bit of\x1B\x19 bold text\x1B\xFF\r\nin your life.",
	"Maybe some\x1B\x1A italics\x1B\xFF too, if you're good.",
	"I can even put an \x1B\x1Cunderline\x1B\xFF on this text as well!",
	"\x1B\x38Or perhaps you'd like some masked text?",
	"\x1B\x20I wonder if you can tell what's different about this text?",
	"Well that's all for now,\r\n\x1B\x1B\x1B\x47have a great day!"
};

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

RootInfo rootInfo;

int sdHandle;
int ldHandle;
int ctHandle;
int bgHandle;
int sprHandle;
int mdHandle;
int sfxHandle;
int sysHandle;

unsigned char smallFileBuffer[1024];

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
		writeString("Error! Could not find ROOTINFO.DAT!", 180, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF));
		goto errorquit; //Error handler
	}
	readFile(rinfHandle, 0x7C, smallFileBuffer, &realReadLen);
	for (int i = 0; i < 4; i++)
	{
		if (smallFileBuffer[i] != magicNumber[i])
		{
			writeString("Error! ROOTINFO.DAT is not valid!", 188, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF));
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
	result = openFile(rootInfo.langDataPath, FILE_OPEN_READ, &sdHandle);
	if (result)
	{
		writeString("Error! Could not find language data file!", 156, 184, FORMAT_SHADOW | FORMAT_COLOUR(0xF));
		goto errorquit; //Error handler
	}
	readFile(sdHandle, 1024, smallFileBuffer, &realReadLen);
	rootInfo.numLang = *((unsigned short*)(smallFileBuffer));
	char* tdPath = smallFileBuffer + smallFileBuffer[2 + 4 * rootInfo.curLang];
	for (int i = 0; i < 12; i++)
	{
		const char ch = tdPath[i];
		if (ch) rootInfo.curTextDataPath[i] = ch;
		else break;
	}
	closeFile(sdHandle);
	
	//For now we are going to ignore the root info structure
	
	writeString("Hello, World!\r\n\x1B\x1FIt's time for some text!", 60, 60, FORMAT_BOLD | FORMAT_ITALIC | FORMAT_SHADOW | FORMAT_FONT_ALTERNATE | FORMAT_COLOUR(0xE));
	
	oldInterruptMask = getPrimaryInterruptMask();
	intsoff();
	oldVsyncVector = getInterruptFunctionRaw(INTERRUPT_VECTOR_VSYNC);
	setInterruptFunction(INTERRUPT_VECTOR_VSYNC, vsyncInterrupt);
	addPrimaryInterrupts(INTERRUPT_MASK_VSYNC);
	intson();
	
	startAnimatedStringToWrite(testStrings[0], 64, 128, FORMAT_SHADOW | FORMAT_FONT_ALTERNATE | FORMAT_COLOUR(0xB));
	
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
				strnum %= 8;
				startAnimatedStringToWrite(testStrings[strnum], 64, 128, FORMAT_SHADOW | FORMAT_FONT_ALTERNATE | FORMAT_COLOUR(0xB));
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
		writeString("Press Enter to quit.", 240, 200, FORMAT_SHADOW | FORMAT_COLOUR(0xF));
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