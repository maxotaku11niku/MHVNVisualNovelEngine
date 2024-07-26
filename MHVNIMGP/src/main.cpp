/* MHVNIMGP - Image file packer for the MHVN Visual Novel Engine
 * Copyright (c) 2024 Maxim Hoxha
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
 * Entry point and command line parsing
 */

#include <stdio.h>
#include <string.h>
#include "imagepacker.h"

void DisplayHelp(void)
{
    puts("Usage:\n\n"
         "mhvnlink -h\n"
         "mhvnimgp -o [output filename] <options> [filename of image list file]\n\n"
         "Options:\n\n"
         "-h             Displays this help.\n"
         "-o [filename]  Defines the filename of the output file, with extension.\n"
         "ONE OF THESE FLAGS MUST BE USED, BUT NOT BOTH!:\n"
         "-b             Set to 'background' mode, suitable for creating background image archives.\n"
         "-s             Set to 'sprite' mode, suitable for creating sprite image archives.");
}

int main(int argc, char** argv)
{
    puts("MHVNIMGP - MHVN image packer tool - Version 0.0.1 - Maxim Hoxha 2024");

    if (argc < 2)
    {
        puts("CRITICAL ERROR - You haven't specified any files or options!");
        DisplayHelp();
        return 1;
    }

    char* outputFilename = NULL;
    char* inputFilename = NULL;
    int mode = -1;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-o"))
        {
            i++;
            if (i >= argc)
            {
                printf("CRITICAL ERROR - Where's the output filename?!\n");
                DisplayHelp();
                return 1;
            }
            outputFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-b"))
        {
            mode = PACK_MODE_BG;
        }
        else if (!strcmp(argv[i], "-s"))
        {
            mode = PACK_MODE_SPR;
        }
        else if (!strcmp(argv[i], "-h"))
        {
            DisplayHelp();
            return 0;
        }
        else
        {
            inputFilename = argv[i];
        }
    }

    if (outputFilename == NULL)
    {
        printf("CRITICAL ERROR - Where's the output filename?!\n");
        DisplayHelp();
        return 1;
    }

    if (inputFilename == NULL)
    {
        printf("CRITICAL ERROR - Where's the input filename?!");
        DisplayHelp();
        return 1;
    }

    if (mode < 0)
    {
        printf("CRITICAL ERROR - Mode not specified!");
        DisplayHelp();
        return 1;
    }

    int result = PackImages(inputFilename, outputFilename, mode);

    if (!result)
    {
        if (mode == PACK_MODE_BG) printf("Success! Background data %s prepared!\n", outputFilename);
        else if (mode == PACK_MODE_SPR) printf("Success! Sprite data %s prepared!\n", outputFilename);
    }
    return result;
}
