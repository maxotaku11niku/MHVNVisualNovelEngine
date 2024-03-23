//File calls wrapper
//Maxim Hoxha 2023-2024

#pragma once

#include "doscalls.h"

typedef doshandle fileptr;

extern unsigned char smallFileBuffer[1024];

int CreateFile(const char* path, unsigned short attributes, fileptr* handle);
int OpenFile(const char* path, unsigned char attribute, fileptr* handle);
int CloseFile(fileptr handle);
int ReadFile(fileptr handle, unsigned short len, __far void* buffer, unsigned short* readbytes);
int WriteFile(fileptr handle, unsigned short len, __far const void* buffer, unsigned short* writebytes);
int DeleteFile(const char* path);
int SeekFile(fileptr handle, unsigned char method, unsigned long len, unsigned long* pos);
