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
 * Palette handling functions
 */

#pragma once

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} ColourRGB;

//Sets the 'main' palette, which is the base palette for the moment
void SetMainPalette(const ColourRGB* pal);
//Sets the 'mix' palette to a single colour. This palette is used to mix with the main palette
void SetMixPaletteToSingleColour(unsigned char r, unsigned char g, unsigned char b);
void SetMixPaletteToSingleColour5bpc(unsigned char r, unsigned char g, unsigned char b);
//Sets the mix palette to the main palette added with a single colour
void SetMixPaletteToMainAdd(short r, short g, short b);
void SetMixPaletteToMainAdd5bpc(unsigned char r, unsigned char g, unsigned char b);
//Sets the mix palette to the main palette but with mod added to the luminosity. mod is a 2.14 fixed point number
void SetMixPaletteToMainLuminosityMod(short mod);
void SetMixPaletteToMainLuminosityMod8bpc(unsigned char mod);
//Sets the mix palette to the main palette but with the saturation multiplied by mod. mod is a 4.12 fixed point number
void SetMixPaletteToMainSaturationMod(short mod);
void SetMixPaletteToMainSaturationMod8bpc(unsigned char mod);
//Sets the mix palette to the main palette but with mod added to the hue. mod is a 1.15 fixed point number in units of full revolutions
void SetMixPaletteToMainHueMod(unsigned short mod);
void SetMixPaletteToMainHueMod8bpc(unsigned char mod);
//Sets the mix palette to the main palette but with all chromas set the same as the given colour
void SetMixPaletteToMainColourised(unsigned char r, unsigned char g, unsigned char b);
void SetMixPaletteToMainColourised5bpc(unsigned char r, unsigned char g, unsigned char b);
//Sets the mix palette to the main palette inverted
void SetMixPaletteToMainInvert();
//Straight copies the main palette to the 'out' palette, which is one step before actual display
void CopyMainPaletteToOut();
//Sets the out palette to a blend of the main and mix palettes, mixAmt's range is 0x00-0xFF, where 0x00 corresponds to the main palette and 0xFF corresponds to the mix palette
void MixPalettes(short mixAmt);
//Straight copies the out palette to the display palette, after any necessary conversions
void SetDisplayPaletteToOut();
//Sets the display palette to the out palette with brightness modfication
void SetDisplayPaletteToOutBrightnessModify(short add);
//Sets the display palette to the out palette with hue rotation
void SetDisplayPaletteToOutHueRotate(unsigned short mod);
//Sets up everything using the default palette
void SetDefaultPalette();
