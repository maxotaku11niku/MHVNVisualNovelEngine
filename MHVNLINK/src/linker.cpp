#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int LinkVN(const char* dir, const char* masterDesc, const char* sceneData, const char** textData, const char* bgData, const char* spriteData, const char* musicData, const char* sfxData, const char* sysData)
{
    char fullDir[512];
    strcpy(fullDir, dir);
    strcat(fullDir, "/");
    char rootinfoFilename[512];
    strcpy(rootinfoFilename, fullDir);
    strcat(rootinfoFilename, "ROOTINFO.DAT");

    //Write root info
    unsigned char* rootinfo = (unsigned char*)calloc(0x6A, 1);
    rootinfo[0] = 'M'; rootinfo[1] = 'H'; rootinfo[2] = 'V'; rootinfo[3] = 'N'; //Magic number
    *((uint16_t*)(&rootinfo[0x04])) = 0x0000; //VNflags (currently empty)
    *((uint16_t*)(&rootinfo[0x06])) = 0x0000; //numstvar_glob (currently empty)
    *((uint16_t*)(&rootinfo[0x08])) = 0x0000; //numflags_glob (currently empty)
    *((uint16_t*)(&rootinfo[0x0A])) = 0x0000; //numstvar_loc (currently empty)
    *((uint16_t*)(&rootinfo[0x0C])) = 0x0000; //numflags_loc (currently empty)
    *((uint16_t*)(&rootinfo[0x0E])) = 0xF008; //def_format_norm (currently fixed)
    *((uint16_t*)(&rootinfo[0x10])) = 0xF009; //format_char (currently fixed)
    *((uint16_t*)(&rootinfo[0x12])) = 0xF008; //format_menu_n (currently fixed)
    *((uint16_t*)(&rootinfo[0x14])) = 0xE009; //format_menu_S (currently fixed)

    char* filenameptr = (char*)(&rootinfo[0x16]);
    for (int i = 0; i < 8; i++)
    {
        const char ch = sceneData[i];
        if (ch == '.')
        {
            break;
        }
        *filenameptr++ = ch;
    }
    *filenameptr++ = '.'; *filenameptr++ = 'D'; *filenameptr++ = 'A'; *filenameptr++ = 'T';

    strcpy((char*)(&rootinfo[0x22]), "LANGUAGE.DAT"); //fixed by design

    filenameptr = (char*)(&rootinfo[0x2E]);
    for (int i = 0; i < 8; i++)
    {
        const char ch = bgData[i];
        if (ch == '.')
        {
            break;
        }
        *filenameptr++ = ch;
    }
    *filenameptr++ = '.'; *filenameptr++ = 'D'; *filenameptr++ = 'A'; *filenameptr++ = 'T';

    filenameptr = (char*)(&rootinfo[0x3A]);
    for (int i = 0; i < 8; i++)
    {
        const char ch = spriteData[i];
        if (ch == '.')
        {
            break;
        }
        *filenameptr++ = ch;
    }
    *filenameptr++ = '.'; *filenameptr++ = 'D'; *filenameptr++ = 'A'; *filenameptr++ = 'T';

    filenameptr = (char*)(&rootinfo[0x46]);
    for (int i = 0; i < 8; i++)
    {
        const char ch = musicData[i];
        if (ch == '.')
        {
            break;
        }
        *filenameptr++ = ch;
    }
    *filenameptr++ = '.'; *filenameptr++ = 'D'; *filenameptr++ = 'A'; *filenameptr++ = 'T';

    filenameptr = (char*)(&rootinfo[0x52]);
    for (int i = 0; i < 8; i++)
    {
        const char ch = sfxData[i];
        if (ch == '.')
        {
            break;
        }
        *filenameptr++ = ch;
    }
    *filenameptr++ = '.'; *filenameptr++ = 'D'; *filenameptr++ = 'A'; *filenameptr++ = 'T';

    filenameptr = (char*)(&rootinfo[0x5E]);
    for (int i = 0; i < 8; i++)
    {
        const char ch = sysData[i];
        if (ch == '.')
        {
            break;
        }
        *filenameptr++ = ch;
    }
    *filenameptr++ = '.'; *filenameptr++ = 'D'; *filenameptr++ = 'A'; *filenameptr++ = 'T';

    FILE* rootinfofile = fopen(rootinfoFilename, "w");
    fwrite(rootinfo, 1, 0x6A, rootinfofile);
    fclose(rootinfofile);

    free(rootinfo);
    return 0;
}
