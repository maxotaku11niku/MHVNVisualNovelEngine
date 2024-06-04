/* MHVN98 - PC98 executable for the MHVN Visual Novel Engine
 * Copyright (c) 2023-2024 Maxim Hoxha
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Unicode handling
 */

#pragma once

//Temporary function used in lieu of loading fonts from files
void LoadASCIICharactersFromFontROM();

//Returns a single Unicode codepoint from a UTF-8 string, advancing the string pointer to the next character
unsigned int UTF8CharacterDecode(const unsigned char** pstr);

//Returns the width of a character. Width will be -1 if the character is not in the font
int UnicodeGetCharacterWidth(unsigned int code);

//Returns the width of a character, and puts the glyph data into buffer assuming the edit-friendly format. Width will be -1 if the character is not in the font
int UnicodeGetCharacterData(unsigned int code, unsigned long* buffer);

//Swap glyph data format from edit-friendly to VRAM-compatible, one-way operation, can pack into 16 bits per character row if necessary
void SwapCharDataFormats(unsigned long* buffer, int bits32);
