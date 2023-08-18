//PC-98 character generator interface
#pragma once

//Obtains character data from the character generator in a VRAM-compatible format
void getCharacterData(unsigned short code, unsigned long* buffer);
//Obtains character data from the character generator in a format more convenient for modifying the data with
void getCharacterDataEditFriendly(unsigned short code, unsigned long* buffer);
//Puts character data into a custom slot in the character generator's RAM, make sure the data is in VRAM-compatible format
void setCharacterData(unsigned short code, const unsigned long* buffer);
//Swaps between character data formats
void swapCharDataFormats(unsigned long* buffer);
//Gets the correct internal code from a Shift-JIS code
unsigned short sjisToInternalCode(unsigned short code);