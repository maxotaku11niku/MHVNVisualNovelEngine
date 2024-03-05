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
         "-o [filename]  Defines the filename of the output file, with extension.\n"
         "-s             Convert input format text to Shift-JIS, which is currently necessary for valid text on MHVN98.");
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
    bool isShiftJIS = false;
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
        else if (!strcmp(argv[i], "-s")) isShiftJIS = true;
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

    int result = ArchiveText(outputFilename, (const char**)inputFilenames, numInputFiles, isShiftJIS);

    free(inputFilenames);
    if (!result)
    {
        printf("Success! Text archive %s prepared!\n", outputFilename);
    }
    return result;
}
