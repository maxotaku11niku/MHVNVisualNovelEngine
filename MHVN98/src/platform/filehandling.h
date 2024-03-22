//DOS file calls wrapper
//Maxim Hoxha 2023-2024

#pragma once

#include "doscalls.h"

extern unsigned char smallFileBuffer[1024];

int CreateFile(const char* path, unsigned char attributes, unsigned short* const handle);
//int OpenFile(const char* path, unsigned char attribute, unsigned short* const handle);
//int CloseFile(unsigned short handle);
//int ReadFile(unsigned short handle, unsigned short len, void* buffer, unsigned short* const readbytes);
int WriteFile(unsigned short handle, unsigned short len, const void* buffer, unsigned short* const writebytes);
int DeleteFile(const char* path);
int SeekFile(unsigned short handle, unsigned char method, unsigned long len, unsigned long* pos);
