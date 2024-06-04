/* MHVNTXAR - Text data archiver for the MHVN Visual Novel Engine
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
 * Entry point and command line parsing
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "textarchiver.h"

void DisplayHelp(void)
{
    puts("Usage:\n\n"
         "mhvntxar -h\n"
         "mhvntxar -o [output filename] <options> [list of filenames, space separated]\n\n"
         "Options:\n\n"
         "-h             Displays this help.\n"
         "-o [filename]  Defines the filename of the output file, with extension.");
}

int main(int argc, char** argv)
{
    printf("MHVNTXAR - MHVN text archiver tool - Version 0.1.0 - Maxim Hoxha 2023-2024\n");

    if (argc < 2)
    {
        printf("CRITICAL ERROR - You haven't specified any files or options!\n");
        DisplayHelp();
        return 1;
    }

    char* outputFilename = NULL;
    char** inputFilenames;
    int numInputFiles = 0;

    inputFilenames = malloc(sizeof(char*) * argc); //Overallocation is expected to be minimal

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-o"))
        {
            i++;
            if (i >= argc)
            {
                printf("CRITICAL ERROR - Where's the output filename?!\n");
                DisplayHelp();
                free(inputFilenames);
                return 1;
            }
            outputFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-h"))
        {
            DisplayHelp();
            free(inputFilenames);
            return 0;
        }
        else
        {
            inputFilenames[numInputFiles] = argv[i];
            numInputFiles++;
        }
    }

    if (outputFilename == NULL)
    {
        printf("CRITICAL ERROR - Where's the output filename?!\n");
        DisplayHelp();
        free(inputFilenames);
        return 1;
    }

    if (numInputFiles <= 0)
    {
        printf("CRITICAL ERROR - We've got no input files to work with!");
        DisplayHelp();
        free(inputFilenames);
        return 1;
    }

    int result = ArchiveText(outputFilename, (const char**)inputFilenames, numInputFiles);

    free(inputFilenames);
    if (!result)
    {
        printf("Success! Text archive %s prepared!\n", outputFilename);
    }
    return result;
}
