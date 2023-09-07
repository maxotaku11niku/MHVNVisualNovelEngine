#include <stdbool.h>
#include <stdio.h>

unsigned char somebuf[1024];

int ArchiveText(const char* outputFilename, const char** inputFilenames, const int numInputFiles, const bool sjis)
{
    FILE* firstInput = fopen(inputFilenames[0], "rb");
    if (firstInput == NULL)
    {
        return 1;
    }
    fread(somebuf, sizeof(unsigned char), 1024, firstInput);
    fclose(firstInput);
    return 0;
}