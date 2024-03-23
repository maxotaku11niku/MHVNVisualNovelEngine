//PC-98 EGC interface
//Maxim Hoxha 2023-2024

#include "pc98_grcg.h"
#include "pc98_egc.h"
#include "x86strops.h"

void EGCSetToBackgroundClearMode()
{
    EGCSetPatternAndReadSource(EGC_PATTERNSOURCE_BGCOLOUR);
    EGCSetReadWriteMode(EGC_WRITE_PATSHIFT | EGC_SOURCE_CPU);
    EGCSetBitAddressTransferDirection(EGC_BLOCKTRANSFER_FORWARD);
}

void EGCSetToMonochromeDrawMode()
{
    EGCSetPatternAndReadSource(EGC_PATTERNSOURCE_FGCOLOUR);
    EGCSetReadWriteMode(EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP((EGC_ROP_SRC & EGC_ROP_PAT) | ((~EGC_ROP_SRC) & EGC_ROP_DST)));
    EGCSetBitAddressTransferDirection(EGC_BLOCKTRANSFER_FORWARD);
}

void EGCSetToVRAMBlit()
{
    EGCSetPatternAndReadSource(EGC_PATTERNSOURCE_PATREG);
    EGCSetReadWriteMode(EGC_WRITE_PATSHIFT | EGC_SOURCE_VRAM | EGC_PATSET_SOURCE);
    EGCSetBitAddressTransferDirection(EGC_BLOCKTRANSFER_FORWARD);
}



void EGCClearScreen()
{
    EGCSetPlaneAccess(0xF);
    EGCSetMask(0xFFFF);
    EGCSetToBackgroundClearMode();
    EGCSetBitLength(2048);
    Memset16Far(0xFFFF, GDC_PLANES, 16000);
}

void EGCClearLines(unsigned short startLine, unsigned short numLines)
{
    EGCSetPlaneAccess(0xF);
    EGCSetMask(0xFFFF);
    EGCSetToBackgroundClearMode();
    EGCSetBitLength(2048);
    Memset16Far(0xFFFF, GDC_PLANES + startLine * 80, 40 * numLines);
}

void EGCEnable()
{
    GRCGEnable();
    GDCSetMode2(GDC_MODE2_MODIFY);
    GDCSetMode2(GDC_MODE2_EGC);
    GDCSetMode2(GDC_MODE2_NOMODIFY);
}

void EGCDisable()
{
    GDCSetMode2(GDC_MODE2_MODIFY);
    GDCSetMode2(GDC_MODE2_GRCG);
    GDCSetMode2(GDC_MODE2_NOMODIFY);
    GRCGDisable();
}
