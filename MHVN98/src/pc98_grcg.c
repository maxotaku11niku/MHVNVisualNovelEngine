//PC-98 graphic charger interface

#include "pc98_grcg.h"
#include "x86strops.h"
#include "x86segments.h"
#include "unrealhwaddr.h"

void clearScreenGRCG(unsigned char clearCol)
{
    setGRCGMode(GRCG_ENABLE | GRCG_MODE_TDW | GRCG_PLANEMASK(0x0F));
    setGRCGTilesToColour(clearCol);
    memset32Flat(0x00000000, gdcPlane0_relptr, 8000);
}

void clearLinesGRCG(unsigned char clearCol, int startLine, int numLines)
{
    setGRCGMode(GRCG_ENABLE | GRCG_MODE_TDW | GRCG_PLANEMASK(0x0F));
    setGRCGTilesToColour(clearCol);
    memset32Flat(0x00000000, gdcPlane0_relptr + startLine * 80, 20 * numLines);
}