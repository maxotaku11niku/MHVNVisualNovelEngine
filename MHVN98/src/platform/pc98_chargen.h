//PC98 character generator interface
//Maxim Hoxha 2023-2024

#pragma once

//Obtains character data from the character generator in a VRAM-compatible format
void GetCharacterData(unsigned short code, unsigned long* buffer);
//Obtains character data from the character generator in a format more convenient for modifying the data with
void GetCharacterDataEditFriendly(unsigned short code, unsigned long* buffer);
//Puts character data into a custom slot in the character generator's RAM, make sure the data is in VRAM-compatible format
void SetCharacterData(unsigned short code, const unsigned long* buffer);
//Swaps between character data formats, can pack into 16 bits per character row if necessary (part of unicode.h now)
//void SwapCharDataFormats(unsigned long* buffer, int bits32);
//Gets the correct internal code from a Shift-JIS code
unsigned short SjisToInternalCode(unsigned short code);
