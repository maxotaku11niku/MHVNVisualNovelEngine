//PC-98 EGC interface

#include "pc98_grcg.h"
#include "pc98_egc.h"
#include "x86strops.h"
#include "x86segments.h"
#include "unrealhwaddr.h"

void clearScreenEGC()
{
	egc_planeaccess(0xF);
	egc_patdatandreadmode(EGC_PATTERNSOURCE_BGCOLOUR);
	egc_rwmode(EGC_WRITE_PATSHIFT | EGC_SOURCE_CPU);
	egc_mask(0xFFFF);
	egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
	egc_bitlen(2048);
	memset32Flat(0xFFFFFFFF, gdcPlane0_relptr, 8000);
}

void clearLinesEGC(int startLine, int numLines)
{
	egc_planeaccess(0xF);
	egc_patdatandreadmode(EGC_PATTERNSOURCE_BGCOLOUR);
	egc_rwmode(EGC_WRITE_PATSHIFT | EGC_SOURCE_CPU);
	egc_mask(0xFFFF);
	egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
	egc_bitlen(2048);
	memset32Flat(0xFFFFFFFF, gdcPlane0_relptr + startLine * 80, 20 * numLines);
}

void egcEnable()
{
	grcgEnable();
	graphicsSetMode2(GDC_MODE2_MODIFY);
	graphicsSetMode2(GDC_MODE2_EGC);
	graphicsSetMode2(GDC_MODE2_NOMODIFY);
}

void egcDisable()
{
	graphicsSetMode2(GDC_MODE2_MODIFY);
	graphicsSetMode2(GDC_MODE2_GRCG);
	graphicsSetMode2(GDC_MODE2_NOMODIFY);
	grcgDisable();
}