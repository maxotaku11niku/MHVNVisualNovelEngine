//MS-DOS system call macros
#pragma once

//Function error codes
#define DOSERROR_BADFUNCTION 0x0001
#define DOSERROR_FNOTFOUND 0x0002
#define DOSERROR_PNOTFOUND 0x0003
#define DOSERROR_TOOMANYFILES 0x0004
#define DOSERROR_ACCESSDENIED 0x0005
#define DOSERROR_BADHANDLE 0x0006
#define DOSERROR_NOMEMORYCONTROL 0x0007
#define DOSERROR_OUTOFMEMORY 0x0008
#define DOSERROR_BADMEMORYBLOCK 0x0009
#define DOSERROR_BADENV 0x000A
#define DOSERROR_BADFORMAT 0x000B
#define DOSERROR_BADACCESS 0x000C
#define DOSERROR_DEVICEDIFFERENT 0x0011
#define DOSERROR_NOMOREFILES 0x0012

//INT 21 function 00 - Terminate Program (do not actually use this, as it was superseded by function 4C)
#define int21_oldTerminate() asm volatile ("movb $0, %%ah\n\tint $33" : : : "ah")
//INT 21 function 01 - Input Single Character From Console (put into 'char')
#define int21_conReadChar(char) asm volatile ("movb $1, %%ah\n\tint $33" : "=a" (char) : )
//INT 21 function 02 - Output Single Character (char) To Console
#define int21_conWriteChar(char) asm volatile ("movb $2, %%ah\n\tint $33" : : "d" (char))
//INT 21 function 03 - Input Single Character From Aux Port (put into 'char')
#define int21_auxReadChar(char) asm volatile ("movb $3, %%ah\n\tint $33" : "=a" (char) : )
//INT 21 function 04 - Output Single Character (char) To Aux Port
#define int21_auxWriteChar(char) asm volatile ("movb $4, %%ah\n\tint $33" : : "d" (char))
//INT 21 function 05 - Output Single Character (char) To Printer Port
#define int21_printerWriteChar(char) asm volatile ("movb $5, %%ah\n\tint $33" : : "d" (char))
//INT 21 function 09 - Output String (pointed to by strptr) To Console (must be terminated by '$')
#define int21_conWriteString(strptr) asm volatile ("movb $9, %%ah\n\tint $33" : : "d" (strptr))

//INT 21 function 3C - Create File (pathname pointed to by 'path' (null-terminated), attributes in 'attrs', returns a handle in 'handle', can return an error code)
#define int21_createfile(pathseg, path, attrs, handle, errorflag) asm volatile ("movw %w2, %%ds\n\tmovb $60, %%ah\n\tint $33\n\tsbbb %1, %1\n\tmovw %%cs, %w2\n\tmovw %w2, %%ds" : "=a" (handle), "=r" (errorflag), "+r" (pathseg) : "d" (path), "c" (attrs))
//Supporting defines
#define FILE_ATTRIBUTE_READONLY 0x0001
#define FILE_ATTRIBUTE_HIDDEN 0x0002
#define FILE_ATTRIBUTE_SYSTEM 0x0004
#define FILE_ATTRIBUTE_VOLUMELABEL 0x0008
#define FILE_ATTRIBUTE_SUBDIR 0x0010
#define FILE_ATTRIBUTE_ARCHIVE 0x0020
//INT 21 function 3D - Open File (pathname pointed to by 'path' (null-terminated), attribute in 'attr', returns a handle in 'handle', can return an error code)
#define int21_openfile(pathseg, path, attr, handle, errorflag) asm volatile ("movw %w2, %%ds\n\tmovb $61, %%ah\n\tint $33\n\tsbbb %1, %1\n\tmovw %%cs, %w2\n\tmovw %w2, %%ds" : "=a" (handle), "=r" (errorflag), "+r" (pathseg) : "d" (path), "a" (attr))
//Supporting defines
#define FILE_OPEN_READ 0x00
#define FILE_OPEN_WRITE 0x01
#define FILE_OPEN_READWRITE 0x02
//INT 21 function 3E - Close File (given by handle in 'handle', can return an error code)
#define int21_closefile(handle, errorcode, errorflag) asm volatile ("movb $62, %%ah\n\tint $33\n\tsbbb %1, %1" : "=a" (errorcode), "=r" (errorflag) : "b" (handle))
//INT 21 function 3F - Read From a File (given by handle in 'handle', puts 'len' bytes into array pointed to by 'buffer', returns the number of bytes actually read in 'readbytes', can return an error code)
#define int21_readfile(handle, len, bufseg, buffer, readbytes, errorflag) asm volatile ("movw %w2, %%ds\n\tmovb $63, %%ah\n\tint $33\n\tsbbb %1, %1\n\tmovw %%cs, %w2\n\tmovw %w2, %%ds" : "=a" (readbytes), "=r" (errorflag), "+r" (bufseg), "=m" (*((unsigned char*)buffer)) : "b" (handle), "c" (len), "d" (buffer))
//INT 21 function 40 - Write To a File (given by handle in 'handle', puts 'len' bytes into the file from the array pointed to by 'buffer', returns the number of bytes actually written in 'writebytes', can return an error code)
#define int21_writefile(handle, len, bufseg, buffer, writebytes, errorflag) asm volatile ("movw %w2, %%ds\n\tmovb $64, %%ah\n\tint $33\n\tsbbb %1, %1\n\tmovw %%cs, %w2\n\tmovw %w2, %%ds" : "=a" (writebytes), "=r" (errorflag), "+r" (bufseg) : "b" (handle), "c" (len), "d" (buffer), "m" (*((unsigned char*)buffer)))
//INT 21 function 41 - Delete File (pathname pointed to by 'path' (null-terminated), can return an error code)
#define int21_deletefile(pathseg, path, errorcode, errorflag) asm volatile ("movw %w2, %%ds\n\tmovb $65, %%ah\n\tint $33\n\tsbbb %1, %1\n\tmovw %%cs, %w2\n\tmovw %w2, %%ds" : "=a" (errorcode), "=r" (errorflag), "+r" (pathseg) : "d" (path))
//INT 21 function 42 - Seek In File (given by handle in 'handle', moves the file pointer by chunklen:len bytes, according to the method in 'method', returns the new file position in poschunk:pos, can return an error code)
#define int21_seekfile(handle, method, chunklen, len, poschunk, pos, errorflag) asm volatile ("movb $66, %%ah\n\tint $33\n\tsbbb %2, %2" : "=d" (poschunk), "=a" (pos), "=r" (errorflag) : "b" (handle), "a" (method), "c" (chunklen), "d" (len))
//Supporting defines
//From the beginning of the file
#define FILE_SEEK_ABSOLUTE 0x00
//From the current file pointer position
#define FILE_SEEK_RELATIVE 0x01
//From the end of the file
#define FILE_SEEK_REVERSE 0x02
//INT 21 function 48 - Allocate Memory ('segsize' segments, start segment in 'allocseg', can return an error code)
#define int21_memalloc(segsize, allocseg) asm volatile ("movb $72, %%ah\n\tint $33" : "+b" (segsize), "=a" (allocseg) : )
//INT 21 function 49 - Free Memory ('segsize' segments, start segment in 'allocseg', can return an error code)
#define int21_memfree(allocseg, errorcode) asm volatile ("movw %w0, %%es\n\tmovb $73, %%ah\n\tint $33" : "+r" (allocseg), "=a" (errorcode) : )
//INT 21 function 4A - Reallocate Memory ('segsize' segments as new size, start segment in 'allocseg', can return an error code)
#define int21_memrealloc(segsize, allocseg, errorcode) asm volatile ("movw %w0, %%es\n\tmovb $74, %%ah\n\tint $33" : "+r" (allocseg), "+b" (segsize), "=a" (errorcode) : )
