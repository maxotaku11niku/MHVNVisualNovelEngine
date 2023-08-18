//Unreal mode flat addressing compensation for hardware addresses
#pragma once

extern unsigned long* ivt_relptr;
extern unsigned char* gdcPlane0_relptr;
extern unsigned char* gdcPlane1_relptr;
extern unsigned char* gdcPlane2_relptr;
extern unsigned char* gdcPlane3_relptr;
extern unsigned short* key_buffer_relptr;
extern unsigned short* key_conversiontable_relptr;
extern unsigned short* key_bufferhead_relptr;
extern unsigned short* key_buffertail_relptr;
extern unsigned short* key_buffercount_relptr;
extern unsigned char* key_errorretry_relptr;
extern unsigned char* key_status_relptr;
extern unsigned char* key_mod_relptr;