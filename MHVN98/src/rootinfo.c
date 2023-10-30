#include "x86strops.h"
#include "filehandling.h"
#include "textengine.h"
#include "rootinfo.h"

RootInfo rootInfo;

static const char* magicNumber = "MHVN";

int readInRootInfo()
{
    memsetSeg(0, &rootInfo, sizeof(RootInfo)); //Zero out the root info structure
	int handle;
	int realReadLen;
	int result = openFile("ROOTINFO.DAT", FILE_OPEN_READ, &handle);
	if (result)
	{
		writeString("Error! Could not find ROOTINFO.DAT!", 180, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
		return result; //Error handler
	}
	readFile(handle, 0x7C, smallFileBuffer, &realReadLen);
	for (int i = 0; i < 4; i++)
	{
		if (smallFileBuffer[i] != magicNumber[i])
		{
			writeString("Error! ROOTINFO.DAT is not valid!", 188, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
			result = 0xFF;
			return result; //Wrong file format error
		}
	}
	//Read in all root info
	rootInfo.VNFlags = *((unsigned short*)(smallFileBuffer + 0x04));
	rootInfo.numstvar_glob = *((unsigned short*)(smallFileBuffer + 0x06));
	rootInfo.numflags_glob = *((unsigned short*)(smallFileBuffer + 0x08));
	rootInfo.numstvar_loc = *((unsigned short*)(smallFileBuffer + 0x0A));
	rootInfo.numflags_loc = *((unsigned short*)(smallFileBuffer + 0x0C));
	rootInfo.defFormatNormal = *((unsigned short*)(smallFileBuffer + 0x0E));
	rootInfo.defFormatCharName = *((unsigned short*)(smallFileBuffer + 0x10));
	rootInfo.defFormatMenuItem = *((unsigned short*)(smallFileBuffer + 0x12));
	rootInfo.defFormatMenuItemSelected = *((unsigned short*)(smallFileBuffer + 0x14));
	for (int i = 0; i < 12; i++)
	{
		rootInfo.sceneDataPath[i] = smallFileBuffer[0x16 + i];
		rootInfo.langDataPath[i] = smallFileBuffer[0x22 + i];
		rootInfo.BGDataPath[i] = smallFileBuffer[0x2E + i];
		rootInfo.spriteDataPath[i] = smallFileBuffer[0x3A + i];
		rootInfo.musicDataPath[i] = smallFileBuffer[0x46 + i];
		rootInfo.sfxDataPath[i] = smallFileBuffer[0x52 + i];
		rootInfo.systemDataPath[i] = smallFileBuffer[0x5E + i];
	}
	closeFile(handle);
    return 0;
}

int initLanguage(unsigned int lang)
{
	int handle;
	int realReadLen;
	int result = openFile(rootInfo.langDataPath, FILE_OPEN_READ, &handle);
	if (result)
	{
		writeString("Error! Could not find language data file!", 156, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
		return result; //Error handler
	}
	readFile(handle, 1024, smallFileBuffer, &realReadLen);
	rootInfo.numLang = *((unsigned short*)(smallFileBuffer));
	rootInfo.curLang = lang;
	char* tdPath = smallFileBuffer + smallFileBuffer[2 + 4 * lang];
	for (int i = 0; i < 12; i++)
	{
		const char ch = tdPath[i];
		if (ch) rootInfo.curTextDataPath[i] = ch;
		else break;
	}
	closeFile(handle);
	return 0;
}

int changeLanguage(unsigned int newLang)
{
	if (newLang == rootInfo.curLang) return 0; //Do not go through all this if we're not actually changing the language
	int handle;
	int realReadLen;
	int result = openFile(rootInfo.langDataPath, FILE_OPEN_READ, &handle);
	if (result)
	{
		writeString("Error! Could not find language data file!", 156, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
		return result; //Error handler
	}
	readFile(handle, 1024, smallFileBuffer, &realReadLen);
	rootInfo.curLang = newLang;
	char* tdPath = smallFileBuffer + smallFileBuffer[2 + 4 * newLang];
	for (int i = 0; i < 12; i++)
	{
		const char ch = tdPath[i];
		if (ch) rootInfo.curTextDataPath[i] = ch;
		else break;
	}
	closeFile(handle);
	return 0;
}