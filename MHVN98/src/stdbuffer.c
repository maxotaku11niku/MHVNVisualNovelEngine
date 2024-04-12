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
 * Some buffers that I couldn't really put anywhere else
 */

//Stores one half of a sine, 2.14 signed fixed point
const short sineTable[256] = { 0,     201,   402,   603,   804,   1005,  1205,  1406,  1606,  1806,  2006,  2205,  2404,  2603,  2801,  2999,
                               3196,  3393,  3590,  3786,  3981,  4176,  4370,  4563,  4756,  4948,  5139,  5330,  5520,  5708,  5897,  6084,
                               6270,  6455,  6639,  6823,  7005,  7186,  7366,  7545,  7723,  7900,  8076,  8250,  8423,  8595,  8765,  8935,
                               9102,  9269,  9434,  9598,  9760,  9921,  10080, 10238, 10394, 10549, 10702, 10853, 11003, 11151, 11297, 11442,
                               11585, 11727, 11866, 12004, 12140, 12274, 12406, 12537, 12665, 12792, 12916, 13039, 13160, 13279, 13395, 13510,
                               13623, 13733, 13842, 13949, 14053, 14155, 14256, 14354, 14449, 14543, 14635, 14724, 14811, 14896, 14978, 15059,
                               15137, 15213, 15286, 15357, 15426, 15493, 15557, 15619, 15679, 15736, 15791, 15843, 15893, 15941, 15986, 16029,
                               16069, 16107, 16143, 16176, 16207, 16235, 16261, 16284, 16305, 16324, 16340, 16353, 16364, 16373, 16379, 16383,
                               16384, 16383, 16379, 16373, 16364, 16353, 16340, 16324, 16305, 16284, 16261, 16235, 16207, 16176, 16143, 16107,
                               16069, 16029, 15986, 15941, 15893, 15843, 15791, 15736, 15679, 15619, 15557, 15493, 15426, 15357, 15286, 15213,
                               15137, 15059, 14978, 14896, 14811, 14724, 14635, 14543, 14449, 14354, 14256, 14155, 14053, 13949, 13842, 13733,
                               13623, 13510, 13395, 13279, 13160, 13039, 12916, 12792, 12665, 12537, 12406, 12274, 12140, 12004, 11866, 11727,
                               11585, 11442, 11297, 11151, 11003, 10853, 10702, 10549, 10394, 10238, 10080, 9921,  9760,  9598,  9434,  9269,
                               9102,  8935,  8765,  8595,  8423,  8250,  8076,  7900,  7723,  7545,  7366,  7186,  7005,  6823,  6639,  6455,
                               6270,  6084,  5897,  5708,  5520,  5330,  5139,  4948,  4756,  4563,  4370,  4176,  3981,  3786,  3590,  3393,
                               3196,  2999,  2801,  2603,  2404,  2205,  2006,  1806,  1606,  1406,  1205,  1005,  804,   603,   402,   201 };

//1.15 fixed point numbers, in units of full revolutions, top-right quadrant only, other quadrants are just this table rotated plus a constant, axes are handled separately
//                        x =     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16
const short atan2Table[256] = { 4096, 2418, 1678, 1278, 1029,  861,  740,  649,  577,  520,  473,  434,  400,  372,  347,  326,   //y =  1
                                5774, 4096, 3067, 2418, 1984, 1678, 1451, 1278, 1140, 1029,  938,  861,  796,  740,  691,  649,   //y =  2
                                6514, 5125, 4096, 3356, 2818, 2418, 2112, 1871, 1678, 1520, 1389, 1278, 1183, 1101, 1029,  967,   //y =  3
                                6914, 5774, 4836, 4096, 3519, 3067, 2707, 2418, 2181, 1984, 1819, 1678, 1557, 1451, 1359, 1278,   //y =  4
                                7163, 6208, 5374, 4673, 4096, 3623, 3235, 2913, 2645, 2418, 2225, 2059, 1915, 1789, 1678, 1580,   //y =  5
                                7331, 6514, 5774, 5125, 4569, 4096, 3696, 3356, 3067, 2818, 2604, 2418, 2255, 2112, 1984, 1871,   //y =  6
                                7452, 6741, 6080, 5485, 4957, 4496, 4096, 3749, 3447, 3185, 2956, 2754, 2576, 2418, 2277, 2151,   //y =  7
                                7543, 6914, 6321, 5774, 5279, 4836, 4443, 4096, 3790, 3519, 3279, 3067, 2877, 2707, 2555, 2418,   //y =  8
                                7615, 7052, 6514, 6011, 5547, 5125, 4745, 4402, 4096, 3822, 3576, 3356, 3158, 2980, 2818, 2672,   //y =  9
                                7672, 7163, 6672, 6208, 5774, 5374, 5007, 4673, 4370, 4096, 3848, 3623, 3420, 3235, 3067, 2913,   //y = 10
                                7719, 7254, 6803, 6373, 5967, 5588, 5236, 4913, 4616, 4344, 4096, 3869, 3662, 3473, 3300, 3141,   //y = 11
                                7758, 7331, 6914, 6514, 6133, 5774, 5438, 5125, 4836, 4569, 4323, 4096, 3888, 3696, 3519, 3356,   //y = 12
                                7792, 7396, 7009, 6635, 6277, 5937, 5616, 5315, 5034, 4772, 4530, 4304, 4096, 3903, 3724, 3558,   //y = 13
                                7820, 7452, 7091, 6741, 6403, 6080, 5774, 5485, 5212, 4957, 4719, 4496, 4289, 4096, 3916, 3749,   //y = 14
                                7845, 7501, 7163, 6833, 6514, 6208, 5915, 5637, 5374, 5125, 4892, 4673, 4468, 4276, 4096, 3928,   //y = 15
                                7866, 7543, 7225, 6914, 6612, 6321, 6041, 5774, 5520, 5279, 5051, 4836, 4634, 4443, 4264, 4096 }; //y = 16

//Integer square root results
const unsigned char sqrtTable[256] = { 0,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,
                                       4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,
                                       5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
                                       6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
                                       8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
                                       8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
                                       9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
                                       10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11,
                                       11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
                                       12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                                       12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13,
                                       13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
                                       13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                                       14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                                       14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
                                       15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 };

unsigned char smallFileBuffer[1024];

short Sin(unsigned int x)
{
    unsigned short realIdx = x >> 6;
    short sVal = sineTable[realIdx & 0xFF];
    if (realIdx & 0x100) sVal = -sVal;
    return sVal;
}

short Cos(unsigned int x)
{
    return Sin(x + 0x2000); //Should be inlined
}

unsigned int Atan2(short y, short x)
{
    //Axis escapes
    if (y == 0)
    {
        if (x >= 0) return 0;
        else return 16384;
    }
    if (x == 0)
    {
        if (y > 0) return 8192;
        else return 24576;
    }
    //Find quadrant and get absolute value of y and x
    unsigned short uy;
    unsigned short ux;
    unsigned int quad;
    if (y > 0)
    {
        uy = y;
        if (x > 0)
        {
            quad = 0;
            ux = x;
        }
        else
        {
            quad = 8192;
            if (x == -0x8000) ux = 0x8000; //Handle asymmetry
            else ux = -x;
        }
    }
    else
    {
        if (y == -0x8000) uy = 0x8000;
        else uy = -y;
        if (x > 0)
        {
            quad = 24576;
            ux = x;
        }
        else
        {
            quad = 16384;
            if (x == -0x8000) ux = 0x8000; //Handle asymmetry
            else ux = -x;
        }
    }
    //Find first estimate
    unsigned char msby = 0;
    unsigned char msbx = 0;
    unsigned short checker = 0x8000;
    for (int i = 15; i >= 0; i--)
    {
        if (uy & checker)
        {
            msby = i;
            break;
        }
        checker >>= 1;
    }
    checker = 0x8000;
    for (int i = 15; i >= 0; i--)
    {
        if (ux & checker)
        {
            msbx = i;
            break;
        }
        checker >>= 1;
    }
    unsigned char msb;
    if (msby > msbx) msb = msby;
    else msb = msbx;
    unsigned short iy = uy;
    unsigned short ix = ux;
    checker = 0x0000;
    if (msb > 4)
    {
        iy = uy >> (msb - 4);
        ix = ux >> (msb - 4);
        checker = ((unsigned short)0xFFFF) >> (20 - msb);
    }
    if (iy > 16) iy >>= 1;
    if (ix > 16) ix >>= 1;
    if (quad & 0x2000) //swap indices in some quadrants
    {
        unsigned short temp = iy;
        iy = ix;
        ix = temp;
    }
    unsigned int fest = atan2Table[(iy-1)*16 + (ix-1)];
    unsigned int res;
    if (uy & checker || ux & checker) //Check if we need to refine our estimate
    {
        res = fest;
    }
    else res = fest; //Our estimate is as good as it ever will be

    return res + quad;
}

unsigned short Sqrt(unsigned long x)
{
    if (x < 256)
    {
        return sqrtTable[(unsigned short)x];
    }

    unsigned long x0;
    if (x < 65536)
    {
        x0 = 16 * (((unsigned long)sqrtTable[(unsigned short)(x >> 8)]) + 1);
    }
    else if (x < 16777216)
    {
        x0 = 256 * (((unsigned long)sqrtTable[(unsigned short)(x >> 16)]) + 1);
    }
    else
    {
        x0 = 4096 * (((unsigned long)sqrtTable[(unsigned short)(x >> 24)]) + 1);
    }

    unsigned long x1 = (x0 + (x/x0))/2;
    while (x1 < x0)
    {
        x0 = x1;
        x1 = (x0 + (x/x0))/2;
    }

    return (unsigned short)x0;
}
