//Text drawing engine

#include "x86ports.h"
#include "x86strops.h"
#include "pc98_crtbios.h"
#include "pc98_gdc.h"
#include "pc98_chargen.h"
#include "pc98_keyboard.h"
#include "pc98_egc.h"
#include "unrealhwaddr.h"
#include "textengine.h"

unsigned long charbuf[16];

unsigned long animCharBuf[16 * 16]; //For fade in animation, ring buffer
short charXs[16];
short charYs[16];
char charColours[16];
char charShadowed[16];
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

void setShadowColours(const unsigned char* cols)
{
	memcpy32Seg(cols, shadowColours, 4);
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

void startAnimatedStringToWrite(const char* str, const short x, const short y, short format)
{
	stringToAnimWrite = str;
	curAnimStringPos = str;
	currentAnimWriteX = x;
	currentAnimWriteY = y;
	currentAnimNextWriteX = x;
	currentAnimNextWriteY = y;
	currentAnimDefaultFormat = format;
	currentAnimFormat = format;
	chBufStartNum = 0;
	animReachedEndOfString = 0;
	animLength = 16;
	memset32Seg(0, animCharBuf, 256);
}

int stringWriteAnimationFrame(int skip)
{
	if (skip) //If a skip is requested, just write the entire string out with its intended formatting, and return the end of animation signal
	{
		animReachedEndOfString = 1;
		animLength = 0;
		writeString(stringToAnimWrite, currentAnimWriteX, currentAnimWriteY, currentAnimDefaultFormat);
		return 1;
	}
	char ch;
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
						break;
					case 0x0A: //LF
						curY += 16;
						break;
					case 0x0D: //CR
						curX = x;
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
							case 0x05: //unassigned
							case 0x06: //unassigned
							case 0x07: //unassigned
							case 0x08: //unassigned
							case 0x09: //unassigned
							case 0x0A: //unassigned
							case 0x0B: //unassigned
							case 0x0C: //unassigned
							case 0x0D: //unassigned
							case 0x0E: //unassigned
							case 0x0F: //reset sections
								format  = ((ch & 0x1 ? defFormat : format) & FORMAT_PART_MAIN);
								format |= ((ch & 0x2 ? defFormat : format) & FORMAT_PART_FONT);
								format |= ((ch & 0x4 ? defFormat : format) & FORMAT_PART_FADE);
								format |= ((ch & 0x8 ? defFormat : format) & FORMAT_PART_COLOUR);
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
				curX += 8;
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
				curX += actualWidth;
				break;
			}
		}
		else
		{
			nullTerm = 1;
			break;
		}
	}
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
		int fadeStart = (16 - animLength + i) * 4;
		if (charShadowed[chBufNum])
		{
			egc_fgcolour(shadowColours[charColours[chBufNum]]);
			drawCharMask(&animCharBuf[16 * chBufNum], charXs[chBufNum] + 1, charYs[chBufNum] + 1, bayer4x4masks + fadeStart);
		}
		egc_fgcolour(charColours[chBufNum]);
		drawCharMask(&animCharBuf[16 * chBufNum], charXs[chBufNum], charYs[chBufNum], bayer4x4masks + fadeStart);
	}
	chBufStartNum++;
	chBufStartNum &= 0xF;
	if (animLength <= 0) return 1;
	else return 0;
}

void writeString(const char* str, const short x, const short y, short format)
{
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
		ch = *str++;
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
						ch = *str++;
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
							case 0x05: //unassigned
							case 0x06: //unassigned
							case 0x07: //unassigned
							case 0x08: //unassigned
							case 0x09: //unassigned
							case 0x0A: //unassigned
							case 0x0B: //unassigned
							case 0x0C: //unassigned
							case 0x0D: //unassigned
							case 0x0E: //unassigned
							case 0x0F: //reset sections
								format  = ((ch & 0x1 ? defFormat : format) & FORMAT_PART_MAIN);
								format |= ((ch & 0x2 ? defFormat : format) & FORMAT_PART_FONT);
								format |= ((ch & 0x4 ? defFormat : format) & FORMAT_PART_FADE);
								format |= ((ch & 0x8 ? defFormat : format) & FORMAT_PART_COLOUR);
								break;
						}
						break;
					case 0x20: //space
						getCharacterDataEditFriendly(0x2000, charbuf);
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
				ch = *str++;
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