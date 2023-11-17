//PC-98 graphic charger interface

#include "pc98_grcg.h"
#include "x86strops.h"
#include "x86segments.h"
#include "unrealhwaddr.h"

void ClearScreenGRCG(unsigned char clearCol)
{
    SetGRCGMode(GRCG_ENABLE | GRCG_MODE_TDW | GRCG_PLANEMASK(0x0F));
    SetGRCGTilesToColour(clearCol);
    Memset32Flat(0x00000000, gdcPlane0_relptr, 8000);
}

void ClearLinesGRCG(unsigned char clearCol, unsigned long startLine, unsigned long numLines)
{
    SetGRCGMode(GRCG_ENABLE | GRCG_MODE_TDW | GRCG_PLANEMASK(0x0F));
    SetGRCGTilesToColour(clearCol);
    Memset32Flat(0x00000000, gdcPlane0_relptr + startLine * 80, 20 * numLines);
}