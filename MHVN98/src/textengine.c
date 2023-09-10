//Text drawing engine

#include "x86ports.h"
#include "x86strops.h"
#include "pc98_crtbios.h"
#include "pc98_gdc.h"
#include "pc98_chargen.h"
#include "pc98_keyboard.h"
#include "pc98_egc.h"
#include "filehandling.h"
#include "unrealhwaddr.h"
#include "rootinfo.h"
#include "sceneengine.h"
#include "textengine.h"

TextInfo textInfo;

int ctHandle;
unsigned long charbuf[16];

unsigned long animCharBuf[16 * 16]; //For fade in animation, ring buffer
short charXs[16];
short charYs[16];
char charColours[16];
char charShadowed[16];
unsigned char charFade[16];
int chBufStartNum;
const char* stringToAnimWrite;
const char* curAnimStringPos;
short currentAnimWriteX;
short currentAnimWriteY;
short currentAnimNextWriteX;
short currentAnimNextWriteY;
short currentAnimDefaultFormat;
short currentAnimFormat;
unsigned char animReachedEndOfString;
int animLength;
short waitFrames;
short waitPerChar;

short textBoxlX;
short textBoxrX;
short textBoxtY;
short textBoxbY;

unsigned char shadowColours[16];

const unsigned short bayer4x4masks[64] =
{
	0x8888, 0x0000, 0x0000, 0x0000,
	0x8888, 0x0000, 0x2222, 0x0000,
	0x8888, 0x0000, 0xAAAA, 0x0000,
	0xAAAA, 0x0000, 0xAAAA, 0x0000,
	0xAAAA, 0x4444, 0xAAAA, 0x0000,
	0xAAAA, 0x4444, 0xAAAA, 0x1111,
	0xAAAA, 0x4444, 0xAAAA, 0x5555,
	0xAAAA, 0x5555, 0xAAAA, 0x5555,
	0xAAAA, 0xDDDD, 0xAAAA, 0x5555,
	0xAAAA, 0xDDDD, 0xAAAA, 0x7777,
	0xAAAA, 0xDDDD, 0xAAAA, 0xFFFF,
	0xAAAA, 0xFFFF, 0xAAAA, 0xFFFF,
	0xEEEE, 0xFFFF, 0xAAAA, 0xFFFF,
	0xEEEE, 0xFFFF, 0xBBBB, 0xFFFF,
	0xEEEE, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

char* customInfos[16];
char stringBuffer1[512];
char stringBuffer2[512];

void setShadowColours(const unsigned char* cols)
{
	memcpy32Seg(cols, shadowColours, 4);
}

int setupTextInfo()
{
	int realReadLen;
	int result = openFile(rootInfo.curTextDataPath, FILE_OPEN_READ, &ctHandle);
	if (result)
	{
		writeString("Error! Could not find text data file!", 172, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
		return result; //Error handler
	}
	readFile(ctHandle, 0x18, smallFileBuffer, &realReadLen);
	textInfo.systemTextFilePtr = *((unsigned long*)(smallFileBuffer));
	textInfo.creditsTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x04));
	textInfo.characterNamesFilePtr = *((unsigned long*)(smallFileBuffer + 0x08));
	textInfo.sceneTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x0C));
	textInfo.CGTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x10));
	textInfo.musicTextFilePtr = *((unsigned long*)(smallFileBuffer + 0x14));
	closeFile(ctHandle);
	return 0;
}

int loadCurrentCharacterName(int charNumber, char* nameBuffer)
{
	int realReadLen;
	unsigned long curfilepos;
	unsigned short charnamepos;
	int result = openFile(rootInfo.curTextDataPath, FILE_OPEN_READ, &ctHandle);
	if (result)
	{
		writeString("Error! Could not find text data file!", 172, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
		return result; //Error handler
	}
	seekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.characterNamesFilePtr + 2 * charNumber, &curfilepos);
	readFile(ctHandle, 2, &charnamepos, &realReadLen);
	seekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.characterNamesFilePtr + 2 * sceneInfo.numChars + charnamepos, &curfilepos);
	readFile(ctHandle, 64, nameBuffer, &realReadLen);
	closeFile(ctHandle);
	return 0;
}

int loadSceneText(int sceneNumber, char* textDataBuffer, char** textPtrsBuffer)
{
	int realReadLen;
	unsigned long curfilepos;
	unsigned long scenedatpos;
	unsigned short numTexts;
	int result = openFile(rootInfo.curTextDataPath, FILE_OPEN_READ, &ctHandle);
	if (result)
	{
		writeString("Error! Could not find text data file!", 172, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
		return result; //Error handler
	}
	seekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.sceneTextFilePtr + 4 * sceneNumber, &curfilepos);
	readFile(ctHandle, 4, &scenedatpos, &realReadLen);
	seekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.sceneTextFilePtr + 4 * sceneInfo.numScenes, &curfilepos);
	readFile(ctHandle, 2, &numTexts, &realReadLen);
	readFile(ctHandle, 1024, smallFileBuffer, &realReadLen);
	for(int i = 0; i < numTexts; i++)
	{
		textPtrsBuffer[i] = *((unsigned short*)(smallFileBuffer) + i) + textDataBuffer;
	}
	seekFile(ctHandle, FILE_SEEK_ABSOLUTE, textInfo.sceneTextFilePtr + 4 * sceneInfo.numScenes + 2 * (numTexts + 1), &curfilepos);
	readFile(ctHandle, 0x8000, textDataBuffer, &realReadLen);
	if (realReadLen == 0x8000) readFile(ctHandle, 0x8000, textDataBuffer + 0x8000, &realReadLen);
	closeFile(ctHandle);
	return 0;
}

//Char data must be in 'edit-friendly' format
void boldenCharLeft(unsigned long* charb)
{
	for (int i = 0; i < 16; i++)
	{
		unsigned long convcharA = charb[i];
		unsigned long convcharB = convcharA << 1;
		convcharB |= convcharA;
		convcharA ^= convcharB;
		convcharB &= ~(convcharA << 1);
		charb[i] = convcharB;
	}
}

//Char data must be in 'edit-friendly' format
void boldenCharRight(unsigned long* charb)
{
	for (int i = 0; i < 16; i++)
	{
		unsigned long convcharA = charb[i];
		unsigned long convcharB = convcharA >> 1;
		convcharB |= convcharA;
		convcharA ^= convcharB;
		convcharB &= ~(convcharA >> 1);
		charb[i] = convcharB;
	}
}

//Char data must be in 'edit-friendly' format
void italiciseChar(unsigned long* charb)
{
	for (int i = 0; i < 16; i++)
	{
		unsigned long convchar = charb[i];
		convchar >>= 7 - (i >> 1);
		charb[i] = convchar;
	}
}

//Char data must be in 'edit-friendly' format
void underlineChar(unsigned long* charb, short underlineLen)
{
	long underLine = 0x80000000;
	if (underlineLen > 32) underlineLen = 32;
	underlineLen--;
	underLine >>= underlineLen;
	charb[14] |= underLine;
}

//Format doesn't matter
void maskChar(unsigned long* charb, const unsigned short* chosenMask)
{
	for (int i = 0; i < 16; i++)
	{
		unsigned long convchar = charb[i];
		unsigned long mask = chosenMask[i & 0x3];
		convchar &= mask | (mask << 16);
		charb[i] = convchar;
	}
}

//Char data must be in 'VRAM-compatible' format
void drawChar(const unsigned long* charb, short x, short y)
{
	unsigned short* planeptr = (unsigned short*)(y * 80 + ((x >> 3) & 0xFFFE) + (unsigned long)gdcPlane0_relptr);
	unsigned short xinblock = x & 0x000F;
	egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD | EGC_BITADDRESS_DEST(xinblock));
	if(xinblock) //Unaligned
	{
		for (int j = 0; j < 16; j++)
		{
			unsigned long charL = charb[j];
			*((unsigned long*)(&planeptr[40 * j])) = charL;
			planeptr[40 * j + 2] = (unsigned short)charL; //Dummy write to empty the EGC's shift buffer
		}
	}
	else //Aligned
	{
		for (int j = 0; j < 16; j++)
		{
			*((unsigned long*)(&planeptr[40 * j])) = charb[j];
		}
	}
}

//Char data must be in 'VRAM-compatible' format
void drawCharMask(const unsigned long* charb, short x, short y, const unsigned short* chosenMask)
{
	unsigned short* planeptr = (unsigned short*)(y * 80 + ((x >> 3) & 0xFFFE) + (unsigned long)gdcPlane0_relptr);
	unsigned short xinblock = x & 0x000F;
	egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD | EGC_BITADDRESS_DEST(xinblock));
	if(xinblock) //Unaligned
	{
		for (int j = 0; j < 16; j++)
		{
			unsigned long mask = chosenMask[j & 0x3];
			mask |= (mask << 16);
			unsigned long charL = charb[j];
			charL &= mask;
			*((unsigned long*)(&planeptr[40 * j])) = charL;
			planeptr[40 * j + 2] = (unsigned short)charL; //Dummy write to empty the EGC's shift buffer
		}
	}
	else //Aligned
	{
		for (int j = 0; j < 16; j++)
		{
			unsigned long mask = chosenMask[j & 0x3];
			mask |= (mask << 16);
			unsigned long charL = charb[j];
			*((unsigned long*)(&planeptr[40 * j])) = charL & mask;
		}
	}
}

void setCustomInfo(int num, char* str)
{
	customInfos[num] = str;
}

const char* preprocessString(const char* str, int autolb, short lx, short rx, short ty, short by)
{
	//Inject variable strings
	char ch = *str++;
	char* pch = stringBuffer1;
	while (ch) //stop when the parent string's null terminator is reached
	{
		if (ch == 0x1B)
		{
			ch = *str++;
			if ((ch & 0xF0) == 0x50)
			{
				char* ich = customInfos[ch & 0x0F];
				ch = *ich++;
				while (ch) //inject entire string (like sprintf would do)
				{
					*pch++ = ch;
					ch = *ich++;
				}
			}
			else //escape command is not an information inject -> put literal characters
			{
				*pch++ = 0x1B;
				*pch++ = ch;
			}
		}
		else *pch++ = ch; //no escape -> put literal character
		ch = *str++;
	}
	*pch = 0; //null terminate, as usual
	if (!autolb) return stringBuffer1;
	//Insert automatic line breaks
	pch = stringBuffer2;
	char* sspch = stringBuffer1;
	char* bpp = 0;
	ch = *sspch++;
	short curX = lx;
	short curY = ty;
	while (ch)
	{
		if (ch < 0x21) //control characters
		{
			switch (ch)
			{
				case 0x09: //Tab
					bpp = pch;
					short delX = curX - lx + 32;
					delX &= 0xFFE0; //Tab stops every 4 halfwidth characters
					curX = lx + delX;
					break;
				case 0x0A: //LF
					curY += 16;
					break;
				case 0x0D: //CR
					curX = lx;
					break;
				case 0x20: //Space
					bpp = pch;
					curX += 8;
					break;
			}
		}
		else if (ch <= 0x7F || (ch > 0x9F && ch < 0xE0)) //Single byte
		{
			if (ch == '-') bpp = pch + 1;
			curX += 8;
		}
		else //double byte
		{
			unsigned short twobytecode = ch << 8;
			ch = *str++;
			if (!ch)
			{
				break; //Standard null termination
			}
			twobytecode |= ch;
			twobytecode = sjisToInternalCode(twobytecode);
			curX += (twobytecode >= 0x0900 && twobytecode < 0x0C00) ? 8 : 16; //The characters in this range are logically halfwidth
		}
		if (curY >= by) //Do not allow text to overflow the box
		{
			break;
		}
		else if (curX >= rx)
		{
			if (!bpp) bpp = pch; //Emergency newline if the break point was not set earlier
			else
			{
				sspch -= pch - bpp;
			}
			*bpp++ = 0x0D; *bpp++ = 0x0A; //Insert newline in the most appropriate position if the string is about to overflow the box
			pch = bpp;
			bpp = 0;
			curX = lx;
			curY += 16;
		}
		else *pch++ = ch;
		ch = *sspch++;
	}
	*pch = 0; //null terminate, as usual
	return stringBuffer2;
}

void startAnimatedStringToWrite(const char* str, const short x, const short y, short format)
{
	const char* pstr = preprocessString(str, 1, x, textBoxrX, y, textBoxbY);
	stringToAnimWrite = pstr;
	curAnimStringPos = pstr;
	currentAnimWriteX = x;
	currentAnimWriteY = y;
	currentAnimNextWriteX = x;
	currentAnimNextWriteY = y;
	currentAnimDefaultFormat = format;
	currentAnimFormat = format;
	chBufStartNum = 0;
	animReachedEndOfString = 0;
	animLength = 16;
	waitFrames = 0;
	waitPerChar = 0;
	memset32Seg(0, animCharBuf, 256);
	memset32Seg(0xFFFFFFFF, charFade, 4);
}

int stringWriteAnimationFrame(int skip)
{
	if (skip) //If a skip is requested, just write the entire string out with its intended formatting, and return the end of animation signal
	{
		animReachedEndOfString = 1;
		animLength = 0;
		writeString(stringToAnimWrite, currentAnimWriteX, currentAnimWriteY, currentAnimDefaultFormat, 0);
		return 1;
	}
	unsigned char ch;
	unsigned short twobytecode;
	const char* str = curAnimStringPos;
	short defFormat = currentAnimDefaultFormat;
	short format = currentAnimFormat;
	short x = currentAnimWriteX;
	short y = currentAnimWriteY;
	short curX = currentAnimNextWriteX;
	short curY = currentAnimNextWriteY;
	char nullTerm = animReachedEndOfString;
	unsigned long* nextCharBuf = animCharBuf + 16 * chBufStartNum;
	//Input new characters into the buffer
	if (waitFrames <= 0)
	{
		while (!nullTerm)
		{
			ch = *str++;
			if (ch)
			{
				if (ch < 0x20) //control characters
				{
					switch (ch)
					{
						case 0x09: //Tab
							short delX = curX - x + 32;
							delX &= 0xFFE0; //Tab stops every 4 halfwidth characters
							curX = x + delX;
							waitFrames = waitPerChar;
							break;
						case 0x0A: //LF
							curY += 16;
							break;
						case 0x0D: //CR
							curX = x;
							waitFrames = waitPerChar;
							break;
						case 0x1B: //ESC, used for formatting escape sequences
							ch = *str++;
							switch ((ch & 0xF0) >> 4)
							{
								case 0x00: //intentional nop
									if (ch & 0x0F) break;
									else
									{
										nullTerm = 1; //null terminate
										break;
									}
								case 0x01: //set formatting flags, section 0
									format = (format & (~FORMAT_PART_MAIN)) | (ch & 0x0F);
									break;
								case 0x02: //set formatting flags, section 1
									format = (format & (~FORMAT_PART_FONT)) | ((ch & 0x0F) << 4);
									break;
								case 0x03: //set formatting flags, section 2
									format = (format & (~FORMAT_PART_FADE)) | ((ch & 0x0F) << 8);
									break;
								case 0x04: //set formatting flags, text colour
									format = (format & (~FORMAT_PART_COLOUR)) | ((ch & 0x0F) << 12);
									break;
								case 0x05: //custom protagonist information inject (this is handled by a preprocessor, so do nothing here)
									break;
								case 0x06: //wait for some time before continuing
									waitFrames += 10 * ((ch & 0x0F) + 1);
									goto LdrawT;
								case 0x07: //set wait between characters
									waitPerChar = ch & 0x0F;
									break;
								case 0x08: //unassigned
								case 0x09: //unassigned
								case 0x0A: //unassigned
								case 0x0B: //unassigned
								case 0x0C: //unassigned
								case 0x0D: //unassigned
								case 0x0E: //unassigned
									break;
								case 0x0F: //reset sections
									format = (format & (~FORMAT_PART_MAIN)) | ((ch & 0x1 ? defFormat : format) & FORMAT_PART_MAIN);
									format = (format & (~FORMAT_PART_FONT)) | ((ch & 0x2 ? defFormat : format) & FORMAT_PART_FONT);
									format = (format & (~FORMAT_PART_FADE)) | ((ch & 0x4 ? defFormat : format) & FORMAT_PART_FADE);
									format = (format & (~FORMAT_PART_COLOUR)) | ((ch & 0x8 ? defFormat : format) & FORMAT_PART_COLOUR);
									break;
							}
							break;
					}
				}
				else if (ch <= 0x7F || (ch > 0x9F && ch < 0xE0)) //Single byte
				{
					if (format & FORMAT_FONT_ALTERNATE)
					{
						char isKana = ch & 0x80;
						if (*str == '\xDE') //dakuten
						{
							if (ch == 0xB3)
							{
								ch = 0x65;
								str++;
							}
							else if (ch >= 0xB6 && ch <= 0xC4)
							{
								ch -= 0x50;
								str++;
							}
							else if (ch >= 0xCA && ch <= 0xCE)
							{
								ch -= 0xCA;
								ch <<= 1;
								ch += 0x75;
								str++;
							}
						}
						else if (*str == '\xDF' && (ch >= 0xCA && ch <= 0xCE)) //handakuten
						{
							ch -= 0xCA;
							ch <<= 1;
							ch += 0x76;
							str++;
						}
						twobytecode = (ch & 0x7F) | (isKana ? 0x0A00 : 0x0900);
					}
					else twobytecode = ch << 8;
					getCharacterDataEditFriendly(twobytecode, nextCharBuf);
					boldenCharLeft(nextCharBuf);
					if (format & FORMAT_ITALIC) italiciseChar(nextCharBuf);
					if (format & FORMAT_BOLD) boldenCharRight(nextCharBuf);
					if (format & FORMAT_UNDERLINE) underlineChar(nextCharBuf, 8);
					if (format & FORMAT_PART_FADE) maskChar(nextCharBuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)]);
					swapCharDataFormats(nextCharBuf);
					charColours[chBufStartNum] = FORMAT_COLOUR_GET(format);
					if (format & FORMAT_SHADOW) charShadowed[chBufStartNum] = 1;
					else charShadowed[chBufStartNum] = 0;
					charXs[chBufStartNum] = curX;
					charYs[chBufStartNum] = curY;
					charFade[chBufStartNum] = 0;
					curX += 8;
					waitFrames = waitPerChar;
					break;
				}
				else //double byte
				{
					twobytecode = ch << 8;
					ch = *str++;
					if (!ch)
					{
						nullTerm = 1;
						break; //Standard null termination
					}
					twobytecode |= ch;
					twobytecode = sjisToInternalCode(twobytecode);
					short actualWidth = (twobytecode >= 0x0900 && twobytecode < 0x0C00) ? 8 : 16; //The characters in this range are logically halfwidth
					getCharacterDataEditFriendly(twobytecode, nextCharBuf);
					boldenCharLeft(nextCharBuf);
					if (format & FORMAT_ITALIC) italiciseChar(nextCharBuf);
					if (format & FORMAT_BOLD) boldenCharRight(nextCharBuf);
					if (format & FORMAT_UNDERLINE) underlineChar(nextCharBuf, actualWidth);
					if (format & FORMAT_PART_FADE) maskChar(nextCharBuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)]);
					swapCharDataFormats(nextCharBuf);
					charColours[chBufStartNum] = FORMAT_COLOUR_GET(format);
					if (format & FORMAT_SHADOW) charShadowed[chBufStartNum] = 1;
					else charShadowed[chBufStartNum] = 0;
					charXs[chBufStartNum] = curX;
					charYs[chBufStartNum] = curY;
					charFade[chBufStartNum] = 0;
					curX += actualWidth;
					waitFrames = waitPerChar;
					break;
				}
			}
			else
			{
				nullTerm = 1;
				break;
			}
		}
	}
	else
	{
		waitFrames--;
	}
	LdrawT:
	curAnimStringPos = str;
	currentAnimFormat = format;
	currentAnimNextWriteX = curX;
	currentAnimNextWriteY = curY;
	animReachedEndOfString = nullTerm;
	if (nullTerm)
	{
		chBufStartNum--;
		chBufStartNum &= 0xF;
		animLength--;
	}
	for (int i = 0; i < animLength; i++)
	{
		int chBufNum = (chBufStartNum - i) & 0xF;
		int fadeStart = charFade[chBufNum] * 4;
		if (fadeStart < 64)
		{
			charFade[chBufNum]++;
			const int curcol = charColours[chBufNum];
			if (charShadowed[chBufNum])
			{
				egc_fgcolour(shadowColours[curcol]);
				drawCharMask(&animCharBuf[16 * chBufNum], charXs[chBufNum] + 1, charYs[chBufNum] + 1, bayer4x4masks + fadeStart);
			}
			egc_fgcolour(curcol);
			drawCharMask(&animCharBuf[16 * chBufNum], charXs[chBufNum], charYs[chBufNum], bayer4x4masks + fadeStart);
		}
	}
	chBufStartNum++;
	chBufStartNum &= 0xF;
	if (animLength <= 0) return 1;
	else return 0;
}

void writeString(const char* str, const short x, const short y, short format, int autolb)
{
	const char* pstr = preprocessString(str, autolb, x, textBoxrX, y, textBoxbY);
	unsigned char ch = 0xFF;
	short twobytecode;
	short curX = x;
	short curY = y;
	const short defFormat = format;
	/*
		CCCC FFFF 000T SUIB
		B -> bold flag
		I -> italic flag
		U -> underline flag
		S -> shadow flag
		T -> halfwidth font (0 -> JIS-X-0201 default, 1 -> NEC specific)
		F -> fade value (0 -> no fade, F -> almost complete fade)
		C -> text colour (shadow colour is automatically picked)
	*/
	while (1)
	{
		ch = *pstr++;
		if (ch)
		{
			if (ch <= 0x20) //control characters
			{
				switch (ch)
				{
					case 0x09: //Tab
						short delX = curX - x + 32;
						delX &= 0xFFE0; //Tab stops every 4 halfwidth characters
						curX = x + delX;
						break;
					case 0x0A: //LF
						curY += 16;
						break;
					case 0x0D: //CR
						curX = x;
						break;
					case 0x1B: //ESC, used for formatting escape sequences
						ch = *pstr++;
						switch ((ch & 0xF0) >> 4)
						{
							case 0x00: //intentional nop
								if (ch & 0x0F) break;
								else return; //null terminate
							case 0x01: //set formatting flags, section 0
								format = (format & (~FORMAT_PART_MAIN)) | (ch & 0x0F);
								break;
							case 0x02: //set formatting flags, section 1
								format = (format & (~FORMAT_PART_FONT)) | ((ch & 0x0F) << 4);
								break;
							case 0x03: //set formatting flags, section 2
								format = (format & (~FORMAT_PART_FADE)) | ((ch & 0x0F) << 8);
								break;
							case 0x04: //set formatting flags, text colour
								format = (format & (~FORMAT_PART_COLOUR)) | ((ch & 0x0F) << 12);
								break;
							case 0x05: //custom protagonist information inject (this is handled by a preprocessor, so do nothing here)
							case 0x06: //wait for some time before continuing (pointless here, so do nothing)
							case 0x07: //set wait between characters (pointless here, so do nothing)
							case 0x08: //unassigned
							case 0x09: //unassigned
							case 0x0A: //unassigned
							case 0x0B: //unassigned
							case 0x0C: //unassigned
							case 0x0D: //unassigned
							case 0x0E: //unassigned
								break;
							case 0x0F: //reset sections
								format = (format & (~FORMAT_PART_MAIN)) | ((ch & 0x1 ? defFormat : format) & FORMAT_PART_MAIN);
								format = (format & (~FORMAT_PART_FONT)) | ((ch & 0x2 ? defFormat : format) & FORMAT_PART_FONT);
								format = (format & (~FORMAT_PART_FADE)) | ((ch & 0x4 ? defFormat : format) & FORMAT_PART_FADE);
								format = (format & (~FORMAT_PART_COLOUR)) | ((ch & 0x8 ? defFormat : format) & FORMAT_PART_COLOUR);
								break;
						}
						break;
					case 0x20: //space
						memset32Seg(0, charbuf, 16);
						if (format & FORMAT_UNDERLINE)
						{
							underlineChar(charbuf, 8);
							if (format & FORMAT_PART_FADE) maskChar(charbuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)]);
							swapCharDataFormats(charbuf);
							if (format & FORMAT_SHADOW)
							{
								egc_fgcolour(shadowColours[FORMAT_COLOUR_GET(format)]);
								drawChar(charbuf, curX + 1, curY + 1);
							}
							egc_fgcolour(FORMAT_COLOUR_GET(format));
							drawChar(charbuf, curX, curY);
						}
						curX += 8;
						break;
				}
			}
			else if (ch <= 0x7F || (ch > 0x9F && ch < 0xE0)) //Single byte
			{
				if (format & FORMAT_FONT_ALTERNATE)
				{
					char isKana = ch & 0x80;
					if (*pstr == '\xDE') //dakuten
					{
						if (ch == 0xB3)
						{
							ch = 0x65;
							pstr++;
						}
						else if (ch >= 0xB6 && ch <= 0xC4)
						{
							ch -= 0x50;
							pstr++;
						}
						else if (ch >= 0xCA && ch <= 0xCE)
						{
							ch -= 0xCA;
							ch <<= 1;
							ch += 0x75;
							pstr++;
						}
					}
					else if (*pstr == '\xDF' && (ch >= 0xCA && ch <= 0xCE)) //handakuten
					{
						ch -= 0xCA;
						ch <<= 1;
						ch += 0x76;
						pstr++;
					}
					twobytecode = (ch & 0x7F) | (isKana ? 0x0A00 : 0x0900);
				}
				else twobytecode = ch << 8;
				getCharacterDataEditFriendly(twobytecode, charbuf);
				boldenCharLeft(charbuf);
				if (format & FORMAT_ITALIC) italiciseChar(charbuf);
				if (format & FORMAT_BOLD) boldenCharRight(charbuf);
				if (format & FORMAT_UNDERLINE) underlineChar(charbuf, 8);
				if (format & FORMAT_PART_FADE) maskChar(charbuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)]);
				swapCharDataFormats(charbuf);
				if (format & FORMAT_SHADOW)
				{
					egc_fgcolour(shadowColours[FORMAT_COLOUR_GET(format)]);
					drawChar(charbuf, curX + 1, curY + 1);
				}
				egc_fgcolour(FORMAT_COLOUR_GET(format));
				drawChar(charbuf, curX, curY);
				curX += 8;
			}
			else //double byte
			{
				twobytecode = ch << 8;
				ch = *pstr++;
				if (!ch) return; //Standard null termination
				twobytecode |= ch;
				twobytecode = sjisToInternalCode(twobytecode);
				short actualWidth = (twobytecode >= 0x0900 && twobytecode < 0x0C00) ? 8 : 16; //The characters in this range are logically halfwidth
				getCharacterDataEditFriendly(twobytecode, charbuf);
				boldenCharLeft(charbuf);
				if (format & FORMAT_ITALIC) italiciseChar(charbuf);
				if (format & FORMAT_BOLD) boldenCharRight(charbuf);
				if (format & FORMAT_UNDERLINE) underlineChar(charbuf, actualWidth);
				if (format & FORMAT_PART_FADE) maskChar(charbuf, &bayer4x4masks[60 - (FORMAT_FADE_GET(format) << 2)]);
				swapCharDataFormats(charbuf);
				if (format & FORMAT_SHADOW)
				{
					egc_fgcolour(shadowColours[FORMAT_COLOUR_GET(format)]);
					drawChar(charbuf, curX + 1, curY + 1);
				}
				egc_fgcolour(FORMAT_COLOUR_GET(format));
				drawChar(charbuf, curX, curY);
				curX += actualWidth;
			}
		}
		else break; //Standard null termination
	}
}