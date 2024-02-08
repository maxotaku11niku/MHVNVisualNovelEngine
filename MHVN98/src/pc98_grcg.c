//PC-98 graphic charger interface

#include "pc98_grcg.h"
#include "x86strops.h"
#include "x86segments.h"
#include "unrealhwaddr.h"

void ClearScreenGRCG(unsigned char clearCol)
{
    SetGRCGMode(GRCG_ENABLE | GRCG_MODE_TDW | GRCG_PLANEMASK(0x0F));
    SetGRCGTilesToColour(clearCol);
    setes(GDC_PLANES_SEGMENT);
    Memset16Seg(0x0000, 0, 16000);
}

void ClearLinesGRCG(unsigned char clearCol, unsigned short startLine, unsigned short numLines)
{
    SetGRCGMode(GRCG_ENABLE | GRCG_MODE_TDW | GRCG_PLANEMASK(0x0F));
    SetGRCGTilesToColour(clearCol);
    setes(GDC_PLANES_SEGMENT);
    Memset16Seg(0x0000, startLine * 80, 40 * numLines);
}
