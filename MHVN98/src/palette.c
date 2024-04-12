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

#include "platform/x86strops.h"
#include "platform/pc98_gdc.h"
#include "palette.h"
#include "stdbuffer.h"

//2.14 fixed point, avoids the really costly division operation this way
const short c8bpcto2p14pcTable[256] = { 0,     64,    129,   193,   257,   321,   386,   450,   514,   578,   643,   707,   771,   835,   900,   964,
                                        1028,  1092,  1157,  1221,  1285,  1349,  1414,  1478,  1542,  1606,  1671,  1735,  1799,  1863,  1928,  1992,
                                        2056,  2120,  2185,  2249,  2313,  2377,  2442,  2506,  2570,  2634,  2699,  2763,  2827,  2891,  2956,  3020,
                                        3084,  3148,  3213,  3277,  3341,  3405,  3470,  3534,  3598,  3662,  3727,  3791,  3855,  3919,  3984,  4048,
                                        4112,  4176,  4241,  4305,  4369,  4433,  4498,  4562,  4626,  4690,  4755,  4819,  4883,  4947,  5012,  5076,
                                        5140,  5204,  5269,  5333,  5397,  5461,  5526,  5590,  5654,  5718,  5783,  5847,  5911,  5975,  6040,  6104,
                                        6168,  6232,  6297,  6361,  6425,  6489,  6554,  6618,  6682,  6746,  6811,  6875,  6939,  7003,  7068,  7132,
                                        7196,  7260,  7325,  7389,  7453,  7517,  7582,  7646,  7710,  7774,  7839,  7903,  7967,  8031,  8096,  8160,
                                        8224,  8288,  8353,  8417,  8481,  8545,  8610,  8674,  8738,  8802,  8867,  8931,  8995,  9059,  9124,  9188,
                                        9252,  9316,  9381,  9445,  9509,  9573,  9638,  9702,  9766,  9830,  9895,  9959,  10023, 10087, 10152, 10216,
                                        10280, 10344, 10409, 10473, 10537, 10601, 10666, 10730, 10794, 10858, 10923, 10987, 11051, 11115, 11180, 11244,
                                        11308, 11372, 11437, 11501, 11565, 11629, 11694, 11758, 11822, 11886, 11951, 12015, 12079, 12143, 12208, 12272,
                                        12336, 12400, 12465, 12529, 12593, 12657, 12722, 12786, 12850, 12914, 12979, 13043, 13107, 13171, 13236, 13300,
                                        13364, 13428, 13493, 13557, 13621, 13685, 13750, 13814, 13878, 13942, 14007, 14071, 14135, 14199, 14264, 14328,
                                        14392, 14456, 14521, 14585, 14649, 14713, 14778, 14842, 14906, 14970, 15035, 15099, 15163, 15227, 15292, 15356,
                                        15420, 15484, 15549, 15613, 15677, 15741, 15806, 15870, 15934, 15998, 16063, 16127, 16191, 16255, 16320, 16384 };

//Mostly for testing, since later the main palette will be changeable
const ColourRGB defaultPalette[16] =
{
    { 0x11, 0x11, 0x11 },
    { 0x77, 0x77, 0x77 },
    { 0xBB, 0x33, 0xBB },
    { 0xFF, 0x77, 0xFF },
    { 0x77, 0x11, 0x11 },
    { 0xDD, 0x44, 0x44 },
    { 0xFF, 0xBB, 0x77 },
    { 0xCC, 0xBB, 0x33 },
    { 0x22, 0x77, 0x33 },
    { 0x55, 0xDD, 0x55 },
    { 0x88, 0xFF, 0x55 },
    { 0xFF, 0xFF, 0x66 },
    { 0x33, 0x33, 0xBB },
    { 0x33, 0xAA, 0xFF },
    { 0x99, 0xFF, 0xFF },
    { 0xFF, 0xFF, 0xFF }
};

//Base palette
ColourRGB mainPalette[16];
//Palette to mix with the main palette
ColourRGB mixPalette[16];
//Palette to output to display (with the possibility of adjustment)
ColourRGB outPalette[16];

typedef struct
{
    short y; //2.14 fixed point number
    short u; //2.14 fixed point number
    short v; //2.14 fixed point number
} ColourYUV;

static ColourYUV RGBToYUV(ColourRGB col)
{
    ColourYUV outcol;
    //2.14 fixed point
    long sr = (long)c8bpcto2p14pcTable[col.r];
    long sg = (long)c8bpcto2p14pcTable[col.g];
    long sb = (long)c8bpcto2p14pcTable[col.b];
    //4.28 fixed point results
    long ly =  3483  * sr + 11718 * sg + 1183 * sb;
    long lu = -1637  * sr - 5507  * sg + 7144 * sb;
    long lv =  10076 * sr - 9152  * sg - 924  * sb;
    //Back to 2.14 fixed point
    outcol.y = (short)(ly >> 14);
    outcol.u = (short)(lu >> 14);
    outcol.v = (short)(lv >> 14);
    return outcol;
}

//Cut-down version to optimise the colourise filter slightly
static ColourYUV RGBToUV(ColourRGB col)
{
    ColourYUV outcol;
    //2.14 fixed point
    long sr = (long)c8bpcto2p14pcTable[col.r];
    long sg = (long)c8bpcto2p14pcTable[col.g];
    long sb = (long)c8bpcto2p14pcTable[col.b];
    //4.28 fixed point results
    long lu = -1637  * sr - 5507  * sg + 7144 * sb;
    long lv =  10076 * sr - 9152  * sg - 924  * sb;
    //Back to 2.14 fixed point
    outcol.u = (short)(lu >> 14);
    outcol.v = (short)(lv >> 14);
    return outcol;
}

static ColourRGB YUVToRGB(ColourYUV col)
{
    ColourRGB outcol;
    //2.14 fixed point
    long ly = (long)col.y;
    long lu = (long)col.u;
    long lv = (long)col.v;
    ly <<= 14; //YUV-specific optimisation
    //4.28 fixed point results
    long lr = ly              + 20977 * lv; //YUV-specific optimisation
    long lg = ly - 3520  * lu - 6236  * lv;
    long lb = ly + 34865 * lu; //YUV-specific optimisation
    //rounding
    lr += 526344; lg += 526344; lb += 526344;
    lr /= 1052688; lg /= 1052688; lb /= 1052688;
    //upper 16 bits guaranteed to be all zero or one at this point, so try not to muck around with 32-bit numbers at this point
    short sr = (short)lr;
    short sg = (short)lg;
    short sb = (short)lb;
    //clamping
    if (sr > 0xFF) sr = 0xFF;
    else if (sr < 0) sr = 0;
    if (sg > 0xFF) sg = 0xFF;
    else if (sg < 0) sg = 0;
    if (sb > 0xFF) sb = 0xFF;
    else if (sb < 0) sb = 0;
    outcol.r = (unsigned char)sr;
    outcol.g = (unsigned char)sg;
    outcol.b = (unsigned char)sb;
    return outcol;
}

void SetMainPalette(const ColourRGB* pal)
{
    Memcpy16Near(pal, mainPalette, 24);
}

void SetMixPaletteToSingleColour(unsigned char r, unsigned char g, unsigned char b)
{
    ColourRGB col = { r, g, b };
    for (int i = 0; i < 16; i++)
    {
        mixPalette[i] = col;
    }
}

void SetMixPaletteToMainAdd(short r, short g, short b)
{
    for (int i = 0; i < 16; i++)
    {
        ColourRGB mainCol = mainPalette[i];
        short or = ((short)mainCol.r) + r;
        if (or < 0) or = 0; else if (or > 0xFF) or = 0xFF;
        short og = ((short)mainCol.g) + g;
        if (og < 0) og = 0; else if (og > 0xFF) og = 0xFF;
        short ob = ((short)mainCol.b) + b;
        if (ob < 0) ob = 0; else if (ob > 0xFF) ob = 0xFF;
        ColourRGB outCol = { or, og, ob };
        mixPalette[i] = outCol;
    }
}

void SetMixPaletteToMainLuminosityMod(short mod)
{
    if (mod >= 16384) Memset16Near(0xFFFF, mixPalette, 24);
    else if (mod <= -16384) Memset16Near(0x0000, mixPalette, 24);
    else if (mod < 0)
    {
        long mulval = 16384 + ((long)mod);
        for (int i = 0; i < 16; i++)
        {
            ColourRGB mainCol = mainPalette[i];
            //4.28 fixed point results
            long lr = (long)c8bpcto2p14pcTable[mainCol.r];
            long lg = (long)c8bpcto2p14pcTable[mainCol.g];
            long lb = (long)c8bpcto2p14pcTable[mainCol.b];
            lr *= mulval; lg *= mulval; lb *= mulval;
            //rounding
            lr += 526344; lg += 526344; lb += 526344;
            lr /= 1052688; lg /= 1052688; lb /= 1052688;
            //upper 16 bits guaranteed to be all zero or one at this point, so try not to muck around with 32-bit numbers at this point
            short sr = (short)lr;
            short sg = (short)lg;
            short sb = (short)lb;
            //clamping
            if (sr > 0xFF) sr = 0xFF;
            else if (sr < 0) sr = 0;
            if (sg > 0xFF) sg = 0xFF;
            else if (sg < 0) sg = 0;
            if (sb > 0xFF) sb = 0xFF;
            else if (sb < 0) sb = 0;
            mainCol.r = (unsigned char)sr;
            mainCol.g = (unsigned char)sg;
            mainCol.b = (unsigned char)sb;
            mixPalette[i] = mainCol;
        }
    }
    else if (mod > 0)
    {
        long mulval = 16384 - ((long)mod);
        for (int i = 0; i < 16; i++)
        {
            ColourRGB mainCol = mainPalette[i];
            //4.28 fixed point results
            long lr = (long)c8bpcto2p14pcTable[mainCol.r];
            long lg = (long)c8bpcto2p14pcTable[mainCol.g];
            long lb = (long)c8bpcto2p14pcTable[mainCol.b];
            lr = 16384 - lr; lg = 16384 - lg; lb = 16384 - lb;
            lr *= mulval; lg *= mulval; lb *= mulval;
            lr = 268435456 - lr; lg = 268435456 - lg; lb = 268435456 - lb;
            //rounding
            lr += 526344; lg += 526344; lb += 526344;
            lr /= 1052688; lg /= 1052688; lb /= 1052688;
            //upper 16 bits guaranteed to be all zero or one at this point, so try not to muck around with 32-bit numbers at this point
            short sr = (short)lr;
            short sg = (short)lg;
            short sb = (short)lb;
            //clamping
            if (sr > 0xFF) sr = 0xFF;
            else if (sr < 0) sr = 0;
            if (sg > 0xFF) sg = 0xFF;
            else if (sg < 0) sg = 0;
            if (sb > 0xFF) sb = 0xFF;
            else if (sb < 0) sb = 0;
            mainCol.r = (unsigned char)sr;
            mainCol.g = (unsigned char)sg;
            mainCol.b = (unsigned char)sb;
            mixPalette[i] = mainCol;
        }
    }
    else Memcpy16Near(mainPalette, mixPalette, 24);
}

void SetMixPaletteToMainSaturationMod(short mod)
{
    if (mod == 4096)
    {
        Memcpy16Near(mainPalette, mixPalette, 24);
        return;
    }
    for (int i = 0; i < 16; i++)
    {
        ColourRGB mainCol = mainPalette[i];
        ColourYUV mainYUV = RGBToYUV(mainCol);
        //6.26 fixed point results
        long newu = ((long)(mainYUV.u)) * ((long)(mod));
        long newv = ((long)(mainYUV.v)) * ((long)(mod));
        //Back to 2.14 fixed point
        newu >>= 12; newv >>= 12;
        mainYUV.u = (short)newu;
        mainYUV.v = (short)newv;
        mixPalette[i] = YUVToRGB(mainYUV);
    }
}

void SetMixPaletteToMainHueMod(unsigned short mod)
{
    if (mod == 0)
    {
        Memcpy16Near(mainPalette, mixPalette, 24);
        return;
    }
    for (int i = 0; i < 16; i++)
    {
        ColourRGB mainCol = mainPalette[i];
        ColourYUV mainYUV = RGBToYUV(mainCol);
        unsigned int hue = Atan2(mainYUV.v, mainYUV.u);
        hue += mod;
        //4.28 fixed point result
        long sat = ((long)mainYUV.u) * ((long)mainYUV.u) + ((long)mainYUV.v) * ((long)mainYUV.v);
        sat = (long)(Sqrt(sat)); //18.14 fixed point
        //4.28 fixed point result
        long newu = sat * ((long)(Cos(hue)));
        long newv = sat * ((long)(Sin(hue)));
        //Back to 2.14 fixed point
        newu >>= 14; newv >>= 14;
        mainYUV.u = (short)newu;
        mainYUV.v = (short)newv;
        mixPalette[i] = YUVToRGB(mainYUV);
    }
}

void SetMixPaletteToMainColourised(unsigned char r, unsigned char g, unsigned char b)
{
    ColourRGB crgb = { r, g, b };
    ColourYUV cyuv = RGBToUV(crgb);
    for (int i = 0; i < 16; i++)
    {
        ColourRGB mainCol = mainPalette[i];
        ColourYUV mainYUV = RGBToYUV(mainCol);
        mainYUV.u = cyuv.u;
        mainYUV.v = cyuv.v;
        mixPalette[i] = YUVToRGB(mainYUV);
    }
}

void SetMixPaletteToMainInvert()
{
    for (int i = 0; i < 16; i++)
    {
        ColourRGB mainCol = mainPalette[i];
        ColourRGB outCol;
        outCol.r = 0xFF - mainCol.r;
        outCol.g = 0xFF - mainCol.g;
        outCol.b = 0xFF - mainCol.b;
        mixPalette[i] = outCol;
    }
}

void CopyMainPaletteToOut()
{
    Memcpy16Near(mainPalette, outPalette, 24);
}

void MixPalettes(short mixAmt)
{
    if (mixAmt <= 0) //No mix
    {
        Memcpy16Near(mainPalette, outPalette, 24);
        return;
    }
    else if (mixAmt >= 0xFF) //Full mix
    {
        Memcpy16Near(mixPalette, outPalette, 24);
        return;
    }
    short mainAmt = 0xFF - mixAmt;
    for (int i = 0; i < 16; i++) //Linear interpolation time
    {
        ColourRGB mainCol = mainPalette[i];
        ColourRGB mixCol = mixPalette[i];
        ColourRGB outCol;
        outCol.r = ((mainAmt * ((short)mainCol.r)) + (mixAmt * ((short)mixCol.r)))/0xFF;
        outCol.g = ((mainAmt * ((short)mainCol.g)) + (mixAmt * ((short)mixCol.g)))/0xFF;
        outCol.b = ((mainAmt * ((short)mainCol.b)) + (mixAmt * ((short)mixCol.b)))/0xFF;
        outPalette[i] = outCol;
    }
}

void SetDisplayPaletteToOut()
{
    for (int i = 0; i < 16; i++)
    {
        //Convert each 8bpc colour to a 4bpc colour
        ColourRGB col = outPalette[i];
        unsigned short or = col.r;
        or += 0x08; or /= 0x11;
        unsigned short og = col.g;
        og += 0x08; og /= 0x11;
        unsigned short ob = col.b;
        ob += 0x08; ob /= 0x11;
        GDCSetPaletteColour(i, or, og, ob);
    }
}

void SetDisplayPaletteToOutBrightnessModify(short add)
{
    for (int i = 0; i < 16; i++)
    {
        //Convert each 8bpc colour to a 4bpc colour
        ColourRGB col = outPalette[i];
        short or = col.r;
        or += 0x08 + add; or /= 0x11;
        if (or < 0) or = 0; else if (or > 0x0F) or = 0x0F;
        short og = col.g;
        og += 0x08 + add; og /= 0x11;
        if (og < 0) og = 0; else if (og > 0x0F) og = 0x0F;
        short ob = col.b;
        ob += 0x08 + add; ob /= 0x11;
        if (ob < 0) ob = 0; else if (ob > 0x0F) ob = 0x0F;
        GDCSetPaletteColour(i, or, og, ob);
    }
}

void SetDefaultPalette()
{
    SetMainPalette(defaultPalette);
    CopyMainPaletteToOut();
    SetDisplayPaletteToOut();
}
