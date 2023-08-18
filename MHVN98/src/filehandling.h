//DOS file calls wrapper
#pragma once

#include "doscalls.h"

int createFile(const char* path, unsigned char attributes, int* const handle);
int openFile(const char* path, unsigned char attribute, int* const handle);
int closeFile(int handle);
int readFile(int handle, int len, void* buffer, int* const readbytes);
int writeFile(int handle, int len, const void* buffer, int* const writebytes);
int deleteFile(const char* path);
int seekFile(int handle, unsigned char method, unsigned long len, unsigned long* pos);