//PC-98 graphic charger interface
//Maxim Hoxha 2023-2024

#include "pc98_grcg.h"
#include "x86strops.h"

void GRCGClearScreen(unsigned char clearCol)
{
    GRCGWriteMode(GRCG_ENABLE | GRCG_MODE_TDW | GRCG_PLANEMASK(0x0F));
    GRCGSetTilesToColour(clearCol);
    Memset16Far(0x0000, GDC_PLANES, 16000);
}

void GRCGClearLines(unsigned char clearCol, unsigned short startLine, unsigned short numLines)
{
    GRCGWriteMode(GRCG_ENABLE | GRCG_MODE_TDW | GRCG_PLANEMASK(0x0F));
    GRCGSetTilesToColour(clearCol);
    Memset16Far(0x0000, GDC_PLANES + startLine * 80, 40 * numLines);
}
