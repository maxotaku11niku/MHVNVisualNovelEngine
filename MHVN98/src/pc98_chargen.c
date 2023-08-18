#include "x86ports.h"

void getCharacterData(unsigned short code, unsigned long* buffer)
{
	portOutB(0xA1, (unsigned char)code);
	portOutB(0xA3, (unsigned char)(code >> 8)); //Put JIS code in
	if (code & 0x00FF) //Fullwidth JIS code
	{
		for (int i = 0; i < 16; i++)
		{
			portOutB(0xA5, (unsigned char)i);
			unsigned char rowhalf = portInB(0xA9);
			unsigned long row = (rowhalf << 8); //Read in right half
			portOutB(0xA5, ((unsigned char)i) | 0x20);
			rowhalf = portInB(0xA9);
			row |= rowhalf; //Read in left half
			buffer[i] = row;
		}
	}
	else //Halfwidth JIS code
	{
		for (int i = 0; i < 16; i++)
		{

			portOutB(0xA5, (unsigned char)i);
			unsigned long row = portInB(0xA9); //Read in left half only (both halves are the same, we're technically reading from the right side though)
			buffer[i] = row;
		}
	}
}

void getCharacterDataEditFriendly(unsigned short code, unsigned long* buffer)
{
	portOutB(0xA1, (unsigned char)code);
	portOutB(0xA3, (unsigned char)(code >> 8)); //Put JIS code in
	if (code & 0x00FF) //Fullwidth JIS code
	{
		for (int i = 0; i < 16; i++)
		{
			portOutB(0xA5, (unsigned char)i);
			unsigned char rowhalf = portInB(0xA9);
			unsigned long row = (rowhalf << 16); //Read in right half
			portOutB(0xA5, ((unsigned char)i) | 0x20);
			rowhalf = portInB(0xA9);
			row |= (rowhalf << 24); //Read in left half
			buffer[i] = row;
		}
	}
	else //Halfwidth JIS code
	{
		for (int i = 0; i < 16; i++)
		{
			portOutB(0xA5, (unsigned char)i);
			unsigned long row = portInB(0xA9); //Read in left half only (both halves are the same, we're technically reading from the right side though)
			buffer[i] = (row << 24);
		}
	}
}

void setCharacterData(unsigned short code, const unsigned long* buffer)
{
	portOutB(0xA1, (unsigned char)code);
	portOutB(0xA3, (unsigned char)(code >> 8)); //Put JIS code in
	for (int i = 0; i < 16; i++)
	{
		unsigned long row = buffer[i];
		portOutB(0xA5, (unsigned char)i);
		portOutB(0xA9, (row >> 8) & 0x000000FF); //Write right half
		portOutB(0xA5, ((unsigned char)i) | 0x20);
		portOutB(0xA9, row & 0x000000FF); //Write left half
	}
}

void swapCharDataFormats(unsigned long* buffer)
{
	for (int i = 0; i < 16; i++)
	{
		unsigned long row = buffer[i];
		unsigned long temprow = (row & 0x000000FF) << 24;
		temprow |= (row & 0x0000FF00) << 8;
		temprow |= (row & 0x00FF0000) >> 8;
		buffer[i] = temprow | ((row & 0xFF000000) >> 24);
	}
}

unsigned short sjisToInternalCode(const unsigned short code)
{
	const unsigned char firstByte = code >> 8;
	const unsigned char secondByte = code & 0x00FF;
	const unsigned char isEvenUpperByte = secondByte >= 0x9F;
	const unsigned char outputLower = secondByte - 0x1F - (secondByte >= 0x7F) - 0x5E * isEvenUpperByte;
	const unsigned char outputUpper = (firstByte - 0x70 - 0x40 * (firstByte >= 0xA0)) * 2 - 0x21 + isEvenUpperByte; //Internal upper bytes are JIS-X-0208 upper bytes - 0x20
	return (outputUpper << 8) + outputLower;
}