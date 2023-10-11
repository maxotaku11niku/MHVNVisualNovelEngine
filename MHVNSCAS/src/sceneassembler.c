#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int ParseInputFile(const char* contents, const long length, const char* filename)
{
    return 0;
}

int AssembleScenes(const char* outputFilename, const char** inputFilenames, const int numInputFiles)
{
    unsigned char* inputFileContents;
    int numInvalidFiles = 0;
    FILE* outputFileHandle = fopen(outputFilename, "wb");
    if (outputFileHandle == NULL)
    {
        printf("CRITICAL ERROR - Output file %s could not be opened!", outputFilename);
        return 1;
    }

    for (int i = 0; i < numInputFiles; i++)
    {
        //Keep only one file's contents in memory at any given time to manage memory size (for safety I guess)
        FILE* inputFileHandle = fopen(inputFilenames[i], "rb");
        if (inputFileHandle == NULL)
        {
            printf("ERROR - Input file %s is invalid! Skipping over it.", inputFilenames[i]);
            numInvalidFiles++;
            continue;
        }
        fseek(inputFileHandle, 0, SEEK_END); //Hopefully should set the file pointer to the end of the file
        const long fileLen = ftell(inputFileHandle) + 1; //So we can get the file's length!
        fseek(inputFileHandle, 0, SEEK_SET);
        inputFileContents = malloc(fileLen);
        inputFileContents[fileLen - 1] = EOT; //Put EOT character in buffer so that the parser knows when the file has ended
        fread(inputFileContents, 1, fileLen, inputFileHandle);
        fclose(inputFileHandle);

        int result = ParseInputFile(inputFileContents, fileLen, inputFilenames[i]);
        free(inputFileContents);
    }

    if (numInvalidFiles >= numInputFiles)
    {
        printf("CRITICAL ERROR - All input files referenced are invalid!");
        fclose(outputFileHandle);
        remove(outputFilename);
        return 1;
    }

    return 0;
}