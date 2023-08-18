//x86 segment management
//Use with extreme caution
#pragma once

//Get segment registers
#define getcs(seg) asm ("movw %%cs, %w0" : "=rm" (seg))
#define getds(seg) asm ("movw %%ds, %w0" : "=rm" (seg))
#define getes(seg) asm ("movw %%es, %w0" : "=rm" (seg))
#define getss(seg) asm ("movw %%ss, %w0" : "=rm" (seg))
#define getfs(seg) asm ("movw %%fs, %w0" : "=rm" (seg))
#define getgs(seg) asm ("movw %%gs, %w0" : "=rm" (seg))
//Set segment registers
//EXTREMELY DANGEROUS!!!!
#define setcs(seg) asm volatile ("movw %w0, %%cs" : : "rm" (seg))
//VERY DANGEROUS!!
#define setds(seg) asm volatile ("movw %w0, %%ds" : : "rm" (seg))
#define setes(seg) asm volatile ("movw %w0, %%es" : : "rm" (seg))
//VERY DANGEROUS!!
#define setss(seg) asm volatile ("movw %w0, %%ss" : : "rm" (seg))
#define setfs(seg) asm volatile ("movw %w0, %%fs" : : "rm" (seg))
#define setgs(seg) asm volatile ("movw %w0, %%gs" : : "rm" (seg))
//Reset segment registers to cs
#define resetdstocs() asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%ds" : : : "ax")
#define resetestocs() asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%es" : : : "ax")
#define resetsstocs() asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%ss" : : : "ax")
#define resetfstocs() asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%fs" : : : "ax")
#define resetgstocs() asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%gs" : : : "ax")
//Reset segment registers to ds
#define resetestods() asm volatile ("movw %%ds, %%ax\n\tmovw %%ax, %%es" : : : "ax")
#define resetsstods() asm volatile ("movw %%ds, %%ax\n\tmovw %%ax, %%ss" : : : "ax")
#define resetfstods() asm volatile ("movw %%ds, %%ax\n\tmovw %%ax, %%fs" : : : "ax")
#define resetgstods() asm volatile ("movw %%ds, %%ax\n\tmovw %%ax, %%gs" : : : "ax")
//Reset ds and es (the 8086 normal data segment registers) to cs
#define reset8086datasegtocs() asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%ds\n\tmovw %%ax, %%es" : : : "ax")
//Reset fs and gs (the 80386 extra data segment registers) to cs
#define reset386datasegtocs() asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%fs\n\tmovw %%ax, %%gs" : : : "ax")
//Reset ds, es, fs and gs to cs
#define resetalldatasegtocs() asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%ds\n\tmovw %%ax, %%es\n\tmovw %%ax, %%fs\n\tmovw %%ax, %%gs" : : : "ax")

/* Explicitly overridden memory operations follow */
//Load 8-bit number
#define segovrcs_memload8(src, dst) asm volatile ("movb %%cs:%1, %b0" : "=r" (dst) : "m" (src))
#define segovres_memload8(src, dst) asm volatile ("movb %%es:%1, %b0" : "=r" (dst) : "m" (src))
#define segovrfs_memload8(src, dst) asm volatile ("movb %%fs:%1, %b0" : "=r" (dst) : "m" (src))
#define segovrgs_memload8(src, dst) asm volatile ("movb %%gs:%1, %b0" : "=r" (dst) : "m" (src))
//Load 16-bit number
#define segovrcs_memload16(src, dst) asm volatile ("movw %%cs:%1, %w0" : "=r" (dst) : "m" (src))
#define segovres_memload16(src, dst) asm volatile ("movw %%es:%1, %w0" : "=r" (dst) : "m" (src))
#define segovrfs_memload16(src, dst) asm volatile ("movw %%fs:%1, %w0" : "=r" (dst) : "m" (src))
#define segovrgs_memload16(src, dst) asm volatile ("movw %%gs:%1, %w0" : "=r" (dst) : "m" (src))
//Load 32-bit number
#define segovrcs_memload32(src, dst) asm volatile ("movl %%cs:%1, %k0" : "=r" (dst) : "m" (src))
#define segovres_memload32(src, dst) asm volatile ("movl %%es:%1, %k0" : "=r" (dst) : "m" (src))
#define segovrfs_memload32(src, dst) asm volatile ("movl %%fs:%1, %k0" : "=r" (dst) : "m" (src))
#define segovrgs_memload32(src, dst) asm volatile ("movl %%gs:%1, %k0" : "=r" (dst) : "m" (src))
//Store 8-bit number
#define segovrcs_memstore8(src, dst) asm volatile ("movb %b1, %%cs:%0" : "=m" (dst) : "r" (src))
#define segovres_memstore8(src, dst) asm volatile ("movb %b1, %%es:%0" : "=m" (dst) : "r" (src))
#define segovrfs_memstore8(src, dst) asm volatile ("movb %b1, %%fs:%0" : "=m" (dst) : "r" (src))
#define segovrgs_memstore8(src, dst) asm volatile ("movb %b1, %%gs:%0" : "=m" (dst) : "r" (src))
//Store 16-bit number
#define segovrcs_memstore16(src, dst) asm volatile ("movw %w1, %%cs:%0" : "=m" (dst) : "r" (src))
#define segovres_memstore16(src, dst) asm volatile ("movw %w1, %%es:%0" : "=m" (dst) : "r" (src))
#define segovrfs_memstore16(src, dst) asm volatile ("movw %w1, %%fs:%0" : "=m" (dst) : "r" (src))
#define segovrgs_memstore16(src, dst) asm volatile ("movw %w1, %%gs:%0" : "=m" (dst) : "r" (src))
//Store 32-bit number
#define segovrcs_memstore32(src, dst) asm volatile ("movl %k1, %%cs:%0" : "=m" (dst) : "r" (src))
#define segovres_memstore32(src, dst) asm volatile ("movl %k1, %%es:%0" : "=m" (dst) : "r" (src))
#define segovrfs_memstore32(src, dst) asm volatile ("movl %k1, %%fs:%0" : "=m" (dst) : "r" (src))
#define segovrgs_memstore32(src, dst) asm volatile ("movl %k1, %%gs:%0" : "=m" (dst) : "r" (src))
//Move 8-bit number (override on destination)
#define segovrdcs_memmove8(src, dst) asm volatile ("movb %1, %%al\n\tmovb %%al, %%cs:%0" : "=m" (dst) : "m" (src) : "al")
#define segovrdes_memmove8(src, dst) asm volatile ("movb %1, %%al\n\tmovb %%al, %%es:%0" : "=m" (dst) : "m" (src) : "al")
#define segovrdfs_memmove8(src, dst) asm volatile ("movb %1, %%al\n\tmovb %%al, %%fs:%0" : "=m" (dst) : "m" (src) : "al")
#define segovrdgs_memmove8(src, dst) asm volatile ("movb %1, %%al\n\tmovb %%al, %%gs:%0" : "=m" (dst) : "m" (src) : "al")
//Move 8-bit number (override on source)
#define segovrscs_memmove8(src, dst) asm volatile ("movb %%cs:%1, %%al\n\tmovb %%al, %0" : "=m" (dst) : "m" (src) : "al")
#define segovrses_memmove8(src, dst) asm volatile ("movb %%es:%1, %%al\n\tmovb %%al, %0" : "=m" (dst) : "m" (src) : "al")
#define segovrsfs_memmove8(src, dst) asm volatile ("movb %%fs:%1, %%al\n\tmovb %%al, %0" : "=m" (dst) : "m" (src) : "al")
#define segovrsgs_memmove8(src, dst) asm volatile ("movb %%gs:%1, %%al\n\tmovb %%al, %0" : "=m" (dst) : "m" (src) : "al")
//Move 16-bit number (override on destination)
#define segovrdcs_memmove16(src, dst) asm volatile ("movw %1, %%ax\n\tmovw %%ax, %%cs:%0" : "=m" (dst) : "m" (src) : "ax")
#define segovrdes_memmove16(src, dst) asm volatile ("movw %1, %%ax\n\tmovw %%ax, %%es:%0" : "=m" (dst) : "m" (src) : "ax")
#define segovrdfs_memmove16(src, dst) asm volatile ("movw %1, %%ax\n\tmovw %%ax, %%fs:%0" : "=m" (dst) : "m" (src) : "ax")
#define segovrdgs_memmove16(src, dst) asm volatile ("movw %1, %%ax\n\tmovw %%ax, %%gs:%0" : "=m" (dst) : "m" (src) : "ax")
//Move 16-bit number (override on source)
#define segovrscs_memmove16(src, dst) asm volatile ("movw %%cs:%1, %%ax\n\tmovw %%ax, %0" : "=m" (dst) : "m" (src) : "ax")
#define segovrses_memmove16(src, dst) asm volatile ("movw %%es:%1, %%ax\n\tmovw %%ax, %0" : "=m" (dst) : "m" (src) : "ax")
#define segovrsfs_memmove16(src, dst) asm volatile ("movw %%fs:%1, %%ax\n\tmovw %%ax, %0" : "=m" (dst) : "m" (src) : "ax")
#define segovrsgs_memmove16(src, dst) asm volatile ("movw %%gs:%1, %%ax\n\tmovw %%ax, %0" : "=m" (dst) : "m" (src) : "ax")
//Move 32-bit number (override on destination)
#define segovrdcs_memmove32(src, dst) asm volatile ("movl %1, %%eax\n\tmovl %%eax, %%cs:%0" : "=m" (dst) : "m" (src) : "eax")
#define segovrdes_memmove32(src, dst) asm volatile ("movl %1, %%eax\n\tmovl %%eax, %%es:%0" : "=m" (dst) : "m" (src) : "eax")
#define segovrdfs_memmove32(src, dst) asm volatile ("movl %1, %%eax\n\tmovl %%eax, %%fs:%0" : "=m" (dst) : "m" (src) : "eax")
#define segovrdgs_memmove32(src, dst) asm volatile ("movl %1, %%eax\n\tmovl %%eax, %%gs:%0" : "=m" (dst) : "m" (src) : "eax")
//Move 32-bit number (override on source)
#define segovrscs_memmove32(src, dst) asm volatile ("movl %%cs:%1, %%eax\n\tmovl %%eax, %0" : "=m" (dst) : "m" (src) : "eax")
#define segovrses_memmove32(src, dst) asm volatile ("movl %%es:%1, %%eax\n\tmovl %%eax, %0" : "=m" (dst) : "m" (src) : "eax")
#define segovrsfs_memmove32(src, dst) asm volatile ("movl %%fs:%1, %%eax\n\tmovl %%eax, %0" : "=m" (dst) : "m" (src) : "eax")
#define segovrsgs_memmove32(src, dst) asm volatile ("movl %%gs:%1, %%eax\n\tmovl %%eax, %0" : "=m" (dst) : "m" (src) : "eax")