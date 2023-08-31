//Text drawing engine
#pragma once

#define FORMAT_BOLD 0x0001
#define FORMAT_ITALIC 0x0002
#define FORMAT_UNDERLINE 0x0004
#define FORMAT_SHADOW 0x0008
#define FORMAT_FONT_DEFAULT 0x0000
#define FORMAT_FONT_ALTERNATE 0x0010
#define FORMAT_FADE(n) (((n) << 8) & 0x0F00)
#define FORMAT_COLOUR(n) (((n) << 12) & 0xF000)
#define FORMAT_FADE_GET(f) (((f) & 0x0F00) >> 8)
#define FORMAT_COLOUR_GET(f) (((f) & 0xF000) >> 12)
#define FORMAT_PART_MAIN 0x000F
#define FORMAT_PART_FONT 0x00F0
#define FORMAT_PART_FADE 0x0F00
#define FORMAT_PART_COLOUR 0xF000

typedef struct
{
	unsigned long systemTextFilePtr;
	unsigned long creditsTextFilePtr;
	unsigned long characterNamesFilePtr;
	unsigned long sceneTextFilePtr;
	unsigned long CGTextFilePtr;
	unsigned long musicTextFilePtr;
} TextInfo;

extern TextInfo textInfo;

//Set the indices of the shadow colours
void setShadowColours(const unsigned char* cols);

//Set up text info from the current language
int setupTextInfo();

//Load the given character's name
int loadCurrentCharacterName(int charNumber, char* nameBuffer);

//Load the given scene number's text
int loadSceneText(int sceneNumber, char* textDataBuffer, char** textPtrsBuffer);

//Set the custom protagonist info item appropriately
void setCustomInfo(int num, char* str);

//Write a string (str) to the screen at position (x, y) with format given by 'format'.
void writeString(const char* str, const short x, const short y, short format);

//Start an animated string write of string 'str', starting from position (x, y) with format given by 'format'
void startAnimatedStringToWrite(const char* str, const short x, const short y, short format);

//Do the next frame of the string write animation
int stringWriteAnimationFrame(int skip);