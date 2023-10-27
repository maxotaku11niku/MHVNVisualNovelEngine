#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sceneassembler.h"

void DisplayHelp(void)
{
    printf("Usage:\n\n");
    printf("mhvnscas -h\n");
    printf("mhvnscas -o [output filename] <options> [list of filenames, space separated]\n\n");
    printf("Options:\n\n");
    printf("-h             Displays this help.\n");
    printf("-o [filename]  Defines the filename of the output file, with extension.\n");
}

int main(int argc, char** argv)
{
    printf("MHVNSCAS - MHVN scene assembler tool - Version 0.0.1 - Maxim Hoxha 2023\n");

    if (argc < 2)
    {
        printf("CRITICAL ERROR - You haven't specified any files or options!\n");
        DisplayHelp();
        return 1;
    }

    char* outputFilename = NULL;
    char** inputFilenames;
    int numInputFiles = 0;

    inputFilenames = (char**)malloc(sizeof(char*) * argc); //Overallocation is expected to be minimal

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

    int result = AssembleScenes(outputFilename, (const char**)inputFilenames, numInputFiles);

    free(inputFilenames);
    if (!result)
    {
        printf("Success! Scene data %s prepared!", outputFilename);
    }
    return result;
}