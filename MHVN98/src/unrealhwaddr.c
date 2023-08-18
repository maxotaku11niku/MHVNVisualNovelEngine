//Unreal mode flat addressing compensation for hardware addresses

#include "x86segments.h"
#include "x86interrupt.h"
#include "pc98_gdc.h"
#include "pc98_keyboard.h"

//x86 real mode interrupt vector table address
unsigned long* ivt_relptr;

//GDC plane addresses
unsigned char* gdcPlane0_relptr;
unsigned char* gdcPlane1_relptr;
unsigned char* gdcPlane2_relptr;
unsigned char* gdcPlane3_relptr;

//Keyboard BIOS work area addresses
unsigned short* key_buffer_relptr;
unsigned short* key_conversiontable_relptr;
unsigned short* key_bufferhead_relptr;
unsigned short* key_buffertail_relptr;
unsigned short* key_buffercount_relptr;
unsigned char* key_errorretry_relptr;
unsigned char* key_status_relptr;
unsigned char* key_mod_relptr;

//Sets up all pointers to memory-mapped hardware when using unreal mode flat addressing
void setupRelativeFlatPointers()
{
	long rootAddr;
	getcs(rootAddr);
	rootAddr <<= 4; //This will now have the full real address of the program load point
	
	//Every other step is now trivial
	ivt_relptr = (unsigned long*)((unsigned char*)INTERRUPT_VECTOR_TABLE - rootAddr); //This WILL lead to a pointer to 0xFFFF....something, but overflow will save the day!
	gdcPlane0_relptr = GDC_PLANE0 - rootAddr;
	gdcPlane1_relptr = GDC_PLANE1 - rootAddr;
	gdcPlane2_relptr = GDC_PLANE2 - rootAddr;
	gdcPlane3_relptr = GDC_PLANE3 - rootAddr;
	key_buffer_relptr = (unsigned short*)(((unsigned char*)KEY_BUFFER) - rootAddr);
	key_conversiontable_relptr = (unsigned short*)(((unsigned char*)(&KEY_CONVERSION_TABLE)) - rootAddr);
	key_bufferhead_relptr = (unsigned short*)(((unsigned char*)(&KEY_BUFFER_HEAD)) - rootAddr);
	key_buffertail_relptr = (unsigned short*)(((unsigned char*)(&KEY_BUFFER_TAIL)) - rootAddr);
	key_buffercount_relptr = (unsigned short*)(((unsigned char*)(&KEY_BUFFER_COUNT)) - rootAddr);
	key_errorretry_relptr = &KEY_ERRORRETRY - rootAddr;
	key_status_relptr = KEY_STATUS - rootAddr;
	key_mod_relptr = &KEY_MOD - rootAddr;
}