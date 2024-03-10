#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "linker.h"

void DisplayHelp(void)
{
    puts("Usage:\n\n"
         "mhvnlink -h\n"
         "mhvnlink -o [output directory] [options]\n\n"
         "Options:\n\n"
         "-h             Displays this help.\n"
         "ALL OF THESE FLAGS MUST BE USED!:\n"
         "-o [directory]    Defines the directory into which ROOTINFO.DAT and every required data archive is put into.\n"
         "-d [filename]     Defines the filename of the master descriptor file.\n"
         "-s [filename]     Defines the filename of the scene data object file.\n"
         "-t [filename]     Defines the filename of a text data object file to include. This option can be specified as many times as necessary.\n"
         "-bg [filename]    Defines the filename of the background image data object file.\n"
         "-spr [filename]   Defines the filename of the sprite data object file.\n"
         "-m [filename]     Defines the filename of the music data object file.\n"
         "-sfx [filename]   Defines the filename of the sound effect data object file.\n"
         "-sys [filename]   Defines the filename of the system data object file.");
}

int main(int argc, char** argv)
{
    puts("MHVNLINK - MHVN linker tool - Version 0.0.1 - Maxim Hoxha 2024");

    if (argc < 2)
    {
        puts("CRITICAL ERROR - You haven't specified any files or options!");
        DisplayHelp();
        return 1;
    }

    const char* outputDirectory;
    const char* masterDescriptorFilename;
    const char* sceneDataFilename;
    const char* bgDataFilename;
    const char* spriteDataFilename;
    const char* musicDataFilename;
    const char* soundEffectDataFilename;
    const char* systemDataFilename;
    const char** textDataFilenames;
    int numTextDataFiles = 0;

    textDataFilenames = (const char**)malloc(sizeof(char*) * argc); //Overallocation is expected to be minimal

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-o"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            outputDirectory = argv[i];
        }
        else if (!strcmp(argv[i], "-d"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            masterDescriptorFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-s"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            sceneDataFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-t"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            textDataFilenames[numTextDataFiles] = argv[i];
            numTextDataFiles++;
        }
        else if (!strcmp(argv[i], "-bg"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            bgDataFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-spr"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            spriteDataFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-m"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            musicDataFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-sfx"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            soundEffectDataFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-sys"))
        {
            i++;
            if (i >= argc)
            {
                break;
            }
            systemDataFilename = argv[i];
        }
        else if (!strcmp(argv[i], "-h"))
        {
            DisplayHelp();
            free(textDataFilenames);
            return 0;
        }
        else
        {
            puts("CRITICAL ERROR - Unrecognised option!");
            DisplayHelp();
            return 2;
        }
    }

    bool outputDirectoryNull = outputDirectory == NULL;
    bool masterDescriptorFilenameNull = masterDescriptorFilename == NULL;
    bool sceneDataFilenameNull = sceneDataFilename == NULL;
    bool bgDataFilenameNull = bgDataFilename == NULL;
    bool spriteDataFilenameNull = spriteDataFilename == NULL;
    bool musicDataFilenameNull = musicDataFilename == NULL;
    bool soundEffectDataFilenameNull = soundEffectDataFilename == NULL;
    bool systemDataFilenameNull = systemDataFilename == NULL;
    bool textDataFilenamesNull = numTextDataFiles == 0;

    if (outputDirectoryNull)
    {
        puts("CRITICAL ERROR - Output directory not specified!");
        free(textDataFilenames);
        DisplayHelp();
        return 1;
    }
    if (masterDescriptorFilenameNull)
    {
        puts("CRITICAL ERROR - Master descriptor file not specified!");
        free(textDataFilenames);
        DisplayHelp();
        return 1;
    }
    if (sceneDataFilenameNull || textDataFilenamesNull)
    {
        puts("CRITICAL ERROR - Missing data files!:");
        if (sceneDataFilenameNull) puts("  Scene data");
        if (textDataFilenamesNull) puts("  Text data");
        free(textDataFilenames);
        DisplayHelp();
        return 1;
    }
    if (bgDataFilenameNull || spriteDataFilenameNull || musicDataFilenameNull || soundEffectDataFilenameNull || systemDataFilenameNull)
    {
        puts("WARNING - Missing data files, however these data files have not yet been specified so they're not required at the moment:");
        if (bgDataFilenameNull)
        {
            puts("  Background image data");
            bgDataFilename = "BGIMAGE.odat";
        }
        if (spriteDataFilenameNull)
        {
            puts("  Sprite data");
            spriteDataFilename = "SPRITE.odat";
        }
        if (musicDataFilenameNull)
        {
            puts("  Music data");
            musicDataFilename = "MUSIC.odat";
        }
        if (soundEffectDataFilenameNull)
        {
            puts("  Sound effect data");
            soundEffectDataFilename = "SOUNDFX.odat";
        }
        if (systemDataFilenameNull)
        {
            puts("  System data");
            systemDataFilename = "SYSTEM.odat";
        }
        puts("You'll need to include these in the future, so make sure your build files have placeholders!");
    }

    int result = LinkVN(outputDirectory, masterDescriptorFilename, sceneDataFilename, textDataFilenames, numTextDataFiles, bgDataFilename, spriteDataFilename, musicDataFilename, soundEffectDataFilename, systemDataFilename);
    free(textDataFilenames);
    return result;
}
