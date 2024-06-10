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

//width - 1
#define GLYPHCACHEINFO_WIDTHMASK 0x0F
#define GLYPHCACHEINFO_INVALID   0xFF

//Loads character ranges from the font file, and glyphs into the ASCII character cache
void InitFontFile();

//Returns a single Unicode codepoint from a UTF-8 string, advancing the string pointer to the next character
unsigned int UTF8CharacterDecode(const unsigned char** pstr);

//Loads a glyph directly from the current font file, returns some info. The glyph buffer must be 16 bits per row and 16 rows high. (TODO)
unsigned char LoadGlyphFromFile(unsigned int code, unsigned char* buffer);

//Tries to load a glyph into the glyph cache, returns the index of the requested glyph data, or -1 if the character can never be found in the glyph cache.
int LoadGlyphCacheWithCharacter(unsigned int code);

//Returns the width of a character. Width will be -1 if the character is not in the font
int UnicodeGetCharacterWidth(unsigned int code);

//Returns the width of a character, and puts the glyph data into buffer assuming the edit-friendly format. Width will be -1 if the character is not in the font
int UnicodeGetCharacterData(unsigned int code, unsigned long* buffer);

//Swap glyph data format from edit-friendly to VRAM-compatible, one-way operation, can pack into 16 bits per character row if necessary
void SwapCharDataFormats(unsigned long* buffer, int bits32);
