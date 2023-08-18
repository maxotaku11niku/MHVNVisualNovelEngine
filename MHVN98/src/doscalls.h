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
#define int21_conReadChar(char) asm volatile ("movb $1, %%ah\n\tint $33\n\tmovb %%al, %0" : "=rm" (char) : : "ah", "al")
//INT 21 function 02 - Output Single Character (char) To Console
#define int21_conWriteChar(char) asm volatile ("movb $2, %%ah\n\tmovb %0, %%dl\n\tint $33" : : "rmi" (char) : "ah", "dl")
//INT 21 function 03 - Input Single Character From Aux Port (put into 'char')
#define int21_auxReadChar(char) asm volatile ("movb $3, %%ah\n\tint $33\n\tmovb %%al, %0" : "=rm" (char) : : "ah", "al")
//INT 21 function 04 - Output Single Character (char) To Aux Port
#define int21_auxWriteChar(char) asm volatile ("movb $4, %%ah\n\tmovb %0, %%dl\n\tint $33" : : "rmi" (char) : "ah", "dl")
//INT 21 function 05 - Output Single Character (char) To Printer Port
#define int21_printerWriteChar(char) asm volatile ("movb $5, %%ah\n\tmovb %0, %%dl\n\tint $33" : : "rmi" (char) : "ah", "dl")
//INT 21 function 09 - Output String (pointed to by strptr) To Console (must be terminated by '$')
#define int21_conWriteString(strptr) asm volatile ("movb $9, %%ah\n\tmovw %0, %%dx\n\tint $33" : : "rmi" (strptr) : "ah", "dx")

//INT 21 function 3C - Create File (pathname pointed to by 'path' (null-terminated), attributes in 'attrs', returns a handle in 'handle', can return an error code)
#define int21_createfile(pathseg, path, attrs, handle, errorflag) asm inline ("movb $60, %%ah\n\tmovw %w2, %%dx\n\tmovw %w3, %%cx\n\tmovw %w4, %%ds\n\tint $33\n\tmovw %%cs, %%cx\n\tmovw %%cx, %%ds\n\tmovw %%ax, %w0" : "=rm" (handle), "=@ccc" (errorflag) : "rmi" (path), "rmi" (attrs), "rmi" (pathseg) : "ah", "ax", "cx", "dx")
//Supporting defines
#define FILE_ATTRIBUTE_READONLY 0x0001
#define FILE_ATTRIBUTE_HIDDEN 0x0002
#define FILE_ATTRIBUTE_SYSTEM 0x0004
#define FILE_ATTRIBUTE_VOLUMELABEL 0x0008
#define FILE_ATTRIBUTE_SUBDIR 0x0010
#define FILE_ATTRIBUTE_ARCHIVE 0x0020
//INT 21 function 3D - Open File (pathname pointed to by 'path' (null-terminated), attribute in 'attr', returns a handle in 'handle', can return an error code)
#define int21_openfile(pathseg, path, attr, handle, errorflag) asm inline ("movb $61, %%ah\n\tmovw %w2, %%dx\n\tmovb %b3, %%al\n\tmovw %w4, %%ds\n\tint $33\n\tmovw %%cs, %%dx\n\tmovw %%dx, %%ds\n\tmovw %%ax, %w0" : "=rm" (handle), "=@ccc" (errorflag) : "rmi" (path), "rmi" (attr), "rmi" (pathseg) : "ah", "ax", "al", "dx")
//Supporting defines
#define FILE_OPEN_READ 0x00
#define FILE_OPEN_WRITE 0x01
#define FILE_OPEN_READWRITE 0x02
//INT 21 function 3E - Close File (given by handle in 'handle', can return an error code)
#define int21_closefile(handle, errorcode, errorflag) asm inline ("movb $62, %%ah\n\tmovw %w2, %%bx\n\tint $33\n\tmovw %%ax, %w0\n\t" : "=rm" (errorcode), "=@ccc" (errorflag) : "rmi" (handle) : "ah", "bx", "ax")
//INT 21 function 3F - Read From a File (given by handle in 'handle', puts 'len' bytes into array pointed to by 'buffer', returns the number of bytes actually read in 'readbytes', can return an error code)
#define int21_readfile(handle, len, bufseg, buffer, readbytes, errorflag) asm inline ("movb $63, %%ah\n\tmovw %w2, %%bx\n\tmovw %w3, %%cx\n\tmovw %w4, %%dx\n\tmovw %w5, %%ds\n\tint $33\n\tmovw %%cs, %%dx\n\tmovw %%dx, %%ds\n\tmovw %%ax, %w0" : "=rm" (readbytes), "=@ccc" (errorflag) : "rmi" (handle), "rmi" (len), "rmi" (buffer), "rmi" (bufseg) : "ah", "ax", "bx", "cx", "dx")
//INT 21 function 40 - Write To a File (given by handle in 'handle', puts 'len' bytes into the file from the array pointed to by 'buffer', returns the number of bytes actually written in 'writebytes', can return an error code)
#define int21_writefile(handle, len, bufseg, buffer, writebytes, errorflag) asm inline ("movb $64, %%ah\n\tmovw %w2, %%bx\n\tmovw %w3, %%cx\n\tmovw %w4, %%dx\n\tmovw %w5, %%ds\n\tint $33\n\tmovw %%cs, %%dx\n\tmovw %%dx, %%ds\n\tmovw %%ax, %w0" : "=rm" (writebytes), "=@ccc" (errorflag) : "rmi" (handle), "rmi" (len), "rmi" (buffer), "rmi" (bufseg) : "ah", "ax", "bx", "cx", "dx")
//INT 21 function 41 - Delete File (pathname pointed to by 'path' (null-terminated), can return an error code)
#define int21_deletefile(pathseg, path, errorcode, errorflag) asm inline ("movb $65, %%ah\n\tmovw %w2, %%dx\n\tmovw %w3, %%ds\n\tint $33\n\tmovw %%cs, %%dx\n\tmovw %%dx, %%ds\n\tmovw %%ax, %w0" : "=rm" (errorcode), "=@ccc" (errorflag) : "rmi" (path), "rmi" (pathseg) : "ah", "ax", "dx")
//INT 21 function 42 - Seek In File (given by handle in 'handle', moves the file pointer by chunklen:len bytes, according to the method in 'method', returns the new file position in poschunk:pos, can return an error code)
#define int21_seekfile(handle, method, chunklen, len, poschunk, pos, errorflag) asm inline ("movb $66, %%ah\n\tmovw %w3, %%bx\n\tmovb %b4, %%al\n\tmovw %w5, %%cx\n\tmovw %w6, %%dx\n\tint $33\n\tmovw %%dx, %w0\n\tmovw %%ax, %w1" : "=rm" (poschunk), "=rm" (pos), "=@ccc" (errorflag) : "rmi" (handle), "rmi" (method), "rmi" (chunklen), "rmi" (len) : "ah", "al", "ax", "bx", "cx", "dx")