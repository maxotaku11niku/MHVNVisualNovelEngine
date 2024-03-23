//File calls wrapper
//Maxim Hoxha 2023-2024

#include "filehandling.h"
#include "doscalls.h"

unsigned char smallFileBuffer[1024];

//Creates a file (overwriting an existing file) and gives the handle back in 'handle' which is passed by reference and merely written to
int CreateFile(const char* path, unsigned short attributes, fileptr* handle)
{
    return DOSCreateFile(path, attributes, handle);
}

//Opens a file and gives the handle back in 'handle' which is passed by reference and merely written to
int OpenFile(const char* path, unsigned char attribute, fileptr* handle)
{
    return DOSOpenFile(path, attribute, handle);
}

//Closes a file with the given handle
int CloseFile(fileptr handle)
{
    return DOSCloseFile(handle);
}

//Reads 'len' bytes from the file given by the handle in 'handle' into the buffer pointed to by 'buffer' (a flat pointer based at the code segment), putting the actual number of bytes read into 'readbytes'
int ReadFile(fileptr handle, unsigned short len, __far void* buffer, unsigned short* readbytes)
{
    return DOSReadFile(handle, len, buffer, readbytes);
}

//Writes 'len' bytes to the file given by the handle in 'handle' from the buffer pointed to by 'buffer' (a flat pointer based at the code segment), putting the actual number of bytes written into 'readbytes'
int WriteFile(fileptr handle, unsigned short len, __far const void* buffer, unsigned short* writebytes)
{
    return DOSWriteFile(handle, len, buffer, writebytes);
}

//Deletes a file
int DeleteFile(const char* path)
{
    return DOSDeleteFile(path);
}

//Seek 'len' bytes in the file given by the handle in 'handle' according to the specified method, putting the new file position in 'pos'
int SeekFile(fileptr handle, unsigned char method, unsigned long len, unsigned long* pos)
{
    return DOSSeekFile(handle, method, len, pos);
}
