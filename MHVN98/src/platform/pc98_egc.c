//PC-98 EGC interface

#include "pc98_grcg.h"
#include "pc98_egc.h"
#include "x86strops.h"
#include "x86segments.h"
//#include "unrealhwaddr.h"

void SetEGCToBackgroundClearMode()
{
    __asm (
        "movw $0x04A2, %%dx\n\t"
        "movw %0, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        "movw $0x04A4, %%dx\n\t"
        "movw %1, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        "movw $0x04AC, %%dx\n\t"
        "movw %2, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        : : "i" (EGC_PATTERNSOURCE_BGCOLOUR), //Pattern data and read mode
           "i" (EGC_WRITE_PATSHIFT | EGC_SOURCE_CPU), //Read/write mode
           "i" (EGC_BLOCKTRANSFER_FORWARD) : "%ax", "%dx"); //Bit address and block transfer direction
}

void SetEGCToMonochromeDrawMode()
{
    __asm (
        "movw $0x04A2, %%dx\n\t"
        "movw %0, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        "movw $0x04A4, %%dx\n\t"
        "movw %1, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        "movw $0x04AC, %%dx\n\t"
        "movw %2, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        : : "i" (EGC_PATTERNSOURCE_FGCOLOUR), //Pattern data and read mode
           "i" (EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP((EGC_ROP_SRC & EGC_ROP_PAT) | ((~EGC_ROP_SRC) & EGC_ROP_DST))), //Read/write mode
           "i" (EGC_BLOCKTRANSFER_FORWARD) : "%ax", "%dx"); //Bit address and block transfer direction
}

void SetEGCToVRAMBlit()
{
    __asm (
        "movw $0x04A2, %%dx\n\t"
        "movw %0, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        "movw $0x04A4, %%dx\n\t"
        "movw %1, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        "movw $0x04AC, %%dx\n\t"
        "movw %2, %%ax\n\t"
        "outw %%ax, %%dx\n\t"
        : : "i" (EGC_PATTERNSOURCE_PATREG), //Pattern data and read mode
           "i" (EGC_WRITE_PATSHIFT | EGC_SOURCE_VRAM | EGC_PATSET_SOURCE), //Read/write mode
           "i" (EGC_BLOCKTRANSFER_FORWARD) : "%ax", "%dx"); //Bit address and block transfer direction
}



void ClearScreenEGC()
{
    egc_planeaccess(0xF);
    egc_mask(0xFFFF);
    SetEGCToBackgroundClearMode();
    egc_bitlen(2048);
    setes(GDC_PLANES_SEGMENT);
    Memset16Seg(0xFFFF, 0, 16000);
}

void ClearLinesEGC(unsigned short startLine, unsigned short numLines)
{
    egc_planeaccess(0xF);
    egc_mask(0xFFFF);
    SetEGCToBackgroundClearMode();
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
