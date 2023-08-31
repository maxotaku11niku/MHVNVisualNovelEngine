//DOS file calls wrapper

#include "x86segments.h"
#include "doscalls.h"

unsigned char smallFileBuffer[1024];

//Creates a file (overwriting an existing file) and gives the handle back in 'handle' which is passed by reference and merely written to
int createFile(const char* path, unsigned char attributes, int* const handle)
{
	int iserr;
	unsigned short codeSeg;
	getcs(codeSeg);
	unsigned short bufSeg = ((unsigned int)path) >> 4;
	unsigned short bufOffset = ((unsigned int)path) - (bufSeg << 4);
	bufSeg += codeSeg; //Compensate for flat addressing
	int21_createfile(bufSeg, bufOffset, attributes, *handle, iserr);
	if(iserr)
	{
		return *handle;
	}
	return 0;
}

//Opens a file and gives the handle back in 'handle' which is passed by reference and merely written to
int openFile(const char* path, unsigned char attribute, int* const handle)
{
	int iserr;
	unsigned short codeSeg;
	getcs(codeSeg);
	unsigned short bufSeg = ((unsigned int)path) >> 4;
	unsigned short bufOffset = ((unsigned int)path) - (bufSeg << 4);
	bufSeg += codeSeg; //Compensate for flat addressing
	int21_openfile(bufSeg, bufOffset, attribute, *handle , iserr);
	if(iserr)
	{
		return *handle;
	}
	return 0;
}

//Closes a file with the given handle
int closeFile(int handle)
{
	int errcode;
	int iserr;
	int21_closefile(handle, errcode, iserr);
	if(iserr)
	{
		return errcode;
	}
	return 0;
}

//Reads 'len' bytes from the file given by the handle in 'handle' into the buffer pointed to by 'buffer' (a flat pointer based at the code segment), putting the actual number of bytes read into 'readbytes'
int readFile(int handle, int len, void* buffer, int* const readbytes)
{
	int iserr;
	unsigned short codeSeg;
	getcs(codeSeg);
	unsigned short bufSeg = ((unsigned int)buffer) >> 4;
	unsigned short bufOffset = ((unsigned int)buffer) - (bufSeg << 4);
	bufSeg += codeSeg; //Compensate for flat addressing
	int21_readfile(handle, len, bufSeg, bufOffset, *readbytes, iserr);
	if(iserr)
	{
		return *readbytes;
	}
	return 0;
}

//Writes 'len' bytes to the file given by the handle in 'handle' from the buffer pointed to by 'buffer' (a flat pointer based at the code segment), putting the actual number of bytes written into 'readbytes'
int writeFile(int handle, int len, const void* buffer, int* const writebytes)
{
	int iserr;
	unsigned short codeSeg;
	getcs(codeSeg);
	unsigned short bufSeg = ((unsigned int)buffer) >> 4;
	unsigned short bufOffset = ((unsigned int)buffer) - (bufSeg << 4);
	bufSeg += codeSeg; //Compensate for flat addressing
	int21_writefile(handle, len, bufSeg, bufOffset, *writebytes, iserr);
	if(iserr)
	{
		return *writebytes;
	}
	return 0;
}

//Deletes a file
int deleteFile(const char* path)
{
	int errcode;
	int iserr;
	unsigned short codeSeg;
	getcs(codeSeg);
	unsigned short bufSeg = ((unsigned int)path) >> 4;
	unsigned short bufOffset = ((unsigned int)path) - (bufSeg << 4);
	bufSeg += codeSeg; //Compensate for flat addressing
	int21_deletefile(bufSeg, bufOffset, errcode, iserr);
	if(iserr)
	{
		return errcode;
	}
	return 0;
}

//Seek 'len' bytes in the file given by the handle in 'handle' according to the specified method, putting the new file position in 'pos'
int seekFile(int handle, unsigned char method, unsigned long len, unsigned long* pos)
{
	int iserr;
	unsigned int upperlen = len >> 16;
	unsigned int lowerlen = len & 0x0000FFFF;
	unsigned int upperpos;
	unsigned int lowerpos;
	int21_seekfile(handle, method, upperlen, lowerlen, upperpos, lowerpos, iserr);
	*pos = (upperpos << 16) + lowerpos;
	if(iserr)
	{
		return lowerpos;
	}
	return 0;
}