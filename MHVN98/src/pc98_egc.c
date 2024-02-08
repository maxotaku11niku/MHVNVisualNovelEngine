//PC-98 EGC interface

#include "pc98_grcg.h"
#include "pc98_egc.h"
#include "x86strops.h"
#include "x86segments.h"
//#include "unrealhwaddr.h"

void ClearScreenEGC()
{
    egc_planeaccess(0xF);
    egc_patdatandreadmode(EGC_PATTERNSOURCE_BGCOLOUR);
    egc_rwmode(EGC_WRITE_PATSHIFT | EGC_SOURCE_CPU);
    egc_mask(0xFFFF);
    egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
    egc_bitlen(2048);
    setes(GDC_PLANES_SEGMENT);
    Memset16Seg(0xFFFF, 0, 16000);
}

void ClearLinesEGC(unsigned short startLine, unsigned short numLines)
{
    egc_planeaccess(0xF);
    egc_patdatandreadmode(EGC_PATTERNSOURCE_BGCOLOUR);
    egc_rwmode(EGC_WRITE_PATSHIFT | EGC_SOURCE_CPU);
    egc_mask(0xFFFF);
    egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
    egc_bitlen(2048);
    setes(GDC_PLANES_SEGMENT);
    Memset16Seg(0xFFFF, startLine * 80, 40 * numLines);
}

void EGCEnable()
{
    GRCGEnable();
    GraphicsSetMode2(GDC_MODE2_MODIFY);
    GraphicsSetMode2(GDC_MODE2_EGC);
    GraphicsSetMode2(GDC_MODE2_NOMODIFY);
}

void EGCDisable()
{
    GraphicsSetMode2(GDC_MODE2_MODIFY);
    GraphicsSetMode2(GDC_MODE2_GRCG);
    GraphicsSetMode2(GDC_MODE2_NOMODIFY);
    GRCGDisable();
}
