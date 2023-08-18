//PC-98 EGC interface
#pragma once

#include "x86ports.h"

//OUTPORT 6A - Write EGC/GRCG Mode
#define egc_writegrcgmode(mode) portOutB(0x6A, mode)
//Supporting defines
#define EGC_MODE_GRCG 0x04
#define EGC_MODE_EGC 0x05
#define EGC_MODE_NOMODIFY 0x06
#define EGC_MODE_MODIFY 0x07
//OUTPORT 04A0 - EGC Plane Access Register
//For convenience, as hardware uses bit = 0 to indicate access, we NOT a mask where bit = 1 indicates access
#define egc_planeaccess(planemask) portOutWExt(0x04A0, ~(planemask))
//OUTPORT 04A2 - EGC Pattern Data And Read Setting Register
//Apparently the lower 8 bits must be 1
#define egc_patdatandreadmode(mode) portOutWExt(0x04A2, (mode) | 0x00FF)
//Supporting defines
#define EGC_PATTERNSOURCE_PATREG	0x0000
#define EGC_PATTERNSOURCE_BGCOLOUR	0x2000
#define EGC_PATTERNSOURCE_FGCOLOUR	0x4000
#define EGC_READSOURCE_PLANE(n) (((n) & 0x7) << 8)
//OUTPORT 04A4 - EGC Read/Write Mode Register
#define egc_rwmode(mode) portOutWExt(0x04A4, mode)
//Supporting defines
#define EGC_READ_SINGLEPLANE	0x0000
#define EGC_READ_COMPARE    	0x2000
#define EGC_WRITE_NOMODIFY  	0x0000
#define EGC_WRITE_ROPSHIFT  	0x0800
#define EGC_WRITE_PATSHIFT  	0x1000
#define EGC_SOURCE_VRAM     	0x0000
#define EGC_SOURCE_CPU      	0x0400
#define EGC_PATSET_NONE     	0x0000
#define EGC_PATSET_SOURCE   	0x0100
#define EGC_PATSET_VRAM     	0x0200
//The raster operation part is such that we can take these defines and then apply bitwise operations, which should represent logically what we want to do!
#define EGC_ROP_0 0x0000
#define EGC_ROP_1 0x00FF
#define EGC_ROP_NOP 0x00CC
#define EGC_ROP_SRC 0x00F0
#define EGC_ROP_DST 0x00CC
#define EGC_ROP_PAT 0x00AA
#define EGC_ROP(rop) ((rop) & 0x00FF)
//OUTPORT 04A6 - EGC Foreground Colour Register
#define egc_fgcolour(col) portOutWExt(0x04A6, col)
//OUTPORT 04A8 - EGC Mask Register
#define egc_mask(mask) portOutWExt(0x04A8, mask)
//OUTPORT 04AA - EGC Background Colour Register
#define egc_bgcolour(col) portOutWExt(0x04AA, col)
//OUTPORT 04AC - EGC Bit Address And Block Transfer Register
#define egc_bitaddrbtmode(mode) portOutWExt(0x04AC, mode)
//Supporting defines
#define EGC_BLOCKTRANSFER_FORWARD 0x0000
#define EGC_BLOCKTRANSFER_BACKWARD 0x1000
#define EGC_BITADDRESS_DEST(n) (((n) & 0xF) << 4)
#define EGC_BITADDRESS_SRC(n) ((n) & 0xF)
//OUTPORT 04AE - EGC Bit Length Register
//There is an implicit +1 to the length, so we account for that
#define egc_bitlen(len) portOutWExt(0x4AE, (len)-1)

void clearScreenEGC();
void clearLinesEGC(int startLine, int numLines);
void egcEnable();
void egcDisable();