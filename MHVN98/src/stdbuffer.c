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
