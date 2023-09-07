#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "textarchiver.h"

void DisplayHelp(void)
{
    printf("Usage:\n\n");
    printf("mhvntxar -h\n");
    printf("mhvntxar -o [output filename] <options> [list of filenames, space separated]\n\n");
    printf("Options:\n\n");
    printf("-h             Displays this help.\n");
    printf("-o [filename]  Defines the filename of the output file, with extension.\n");
    printf("-s             Convert input format text to Shift-JIS, which is currently necessary for valid text on MHVN98.\n");
}

int main(int argc, char** argv)
{
    printf("MHVNTXAR - MHVN text archiver tool - Version 0.0.1 - Maxim Hoxha 2023\n");

    if (argc < 2)
    {
        printf("You haven't specified any files or options!\n");
        DisplayHelp();
        return 1;
    }

    char* outputFilename;
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
                printf("Where's the output filename?!\n");
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

    if (numInputFiles <= 0)
    {
        printf("We've got no input files to work with!");
        DisplayHelp();
        free(inputFilenames);
        return 1;
    }

    for (int i = 0; i < numInputFiles; i++)
    {
        printf("%s\n", inputFilenames[i]);
    }

    int result = ArchiveText(outputFilename, (const char**)inputFilenames, numInputFiles, isShiftJIS);

    free(inputFilenames);
    return result;
}