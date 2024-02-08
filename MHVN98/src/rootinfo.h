#pragma once

typedef struct
{
    unsigned short VNFlags;
    unsigned short numstvar_glob;
    unsigned short numflags_glob;
    unsigned short numstvar_loc;
    unsigned short numflags_loc;
    unsigned short defFormatNormal;
    unsigned short defFormatCharName;
    unsigned short defFormatMenuItem;
    unsigned short defFormatMenuItemSelected;
    unsigned short numLang;
    unsigned short curLang;
    char sceneDataPath[13];
    char langDataPath[13];
    char curTextDataPath[13];
    char BGDataPath[13];
    char spriteDataPath[13];
    char musicDataPath[13];
    char sfxDataPath[13];
    char systemDataPath[13];
} RootInfo;

extern RootInfo rootInfo;

//Read in the root info from ROOTINFO.DAT
int ReadInRootInfo();
//Initialise the language setting
int InitLanguage(unsigned short lang);
//Change the current language
int ChangeLanguage(unsigned short newLang);
