#include "x86ports.h"

void GetCharacterData(unsigned short code, unsigned long* buffer)
{
    PortOutB(0xA1, (unsigned char)code);
    PortOutB(0xA3, (unsigned char)(code >> 8)); //Put JIS code in
    if (code & 0x00FF) //Fullwidth JIS code
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            PortOutB(0xA5, i);
            unsigned char rowhalf = PortInB(0xA9);
            unsigned long row = (rowhalf << 8); //Read in right half
            PortOutB(0xA5, i | 0x20);
            rowhalf = PortInB(0xA9);
            row |= rowhalf; //Read in left half
            buffer[i] = row;
        }
    }
    else //Halfwidth JIS code
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            PortOutB(0xA5, i);
            unsigned long row = PortInB(0xA9); //Read in left half only (both halves are the same, we're technically reading from the right side though)
            buffer[i] = row;
        }
    }
}

void GetCharacterDataEditFriendly(unsigned short code, unsigned long* buffer)
{
    PortOutB(0xA1, (unsigned char)code);
    PortOutB(0xA3, (unsigned char)(code >> 8)); //Put JIS code in
    if (code & 0x00FF) //Fullwidth JIS code
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            PortOutB(0xA5, i);
            unsigned char rowhalf = PortInB(0xA9);
            unsigned long row = (rowhalf << 16); //Read in right half
            PortOutB(0xA5, i | 0x20);
            rowhalf = PortInB(0xA9);
            row |= (rowhalf << 24); //Read in left half
            buffer[i] = row;
        }
    }
    else //Halfwidth JIS code
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            PortOutB(0xA5, i);
            unsigned long row = PortInB(0xA9); //Read in left half only (both halves are the same, we're technically reading from the right side though)
            buffer[i] = (row << 24);
        }
    }
}

void SetCharacterData(unsigned short code, const unsigned long* buffer)
{
    PortOutB(0xA1, (unsigned char)code);
    PortOutB(0xA3, (unsigned char)(code >> 8)); //Put JIS code in
    for (unsigned char i = 0; i < 16; i++)
    {
        unsigned long row = buffer[i];
        PortOutB(0xA5, i);
        PortOutB(0xA9, (row >> 8) & 0x000000FF); //Write right half
        PortOutB(0xA5, i | 0x20);
        PortOutB(0xA9, row & 0x000000FF); //Write left half
    }
}

void SwapCharDataFormats(unsigned long* buffer)
{
    for (unsigned char i = 0; i < 16; i++)
    {
        unsigned long row = buffer[i];
        unsigned long temprow = (row & 0x000000FF) << 24;
        temprow |= (row & 0x0000FF00) << 8;
        temprow |= (row & 0x00FF0000) >> 8;
        buffer[i] = temprow | ((row & 0xFF000000) >> 24);
    }
}

unsigned short SjisToInternalCode(const unsigned short code)
{
    const unsigned char firstByte = code >> 8;
    const unsigned char secondByte = code & 0x00FF;
    const unsigned char isEvenUpperByte = secondByte >= 0x9F;
    const unsigned char outputLower = secondByte - 0x1F - (secondByte >= 0x7F) - 0x5E * isEvenUpperByte;
    const unsigned char outputUpper = (firstByte - 0x70 - 0x40 * (firstByte >= 0xA0)) * 2 - 0x21 + isEvenUpperByte; //Internal upper bytes are JIS-X-0208 upper bytes - 0x20
    return (outputUpper << 8) + outputLower;
}