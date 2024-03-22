//x86 segment management
//Use with extreme caution
//Maxim Hoxha 2023-2024

#pragma once

//Get segment registers
#define getcs(seg) __asm ("movw %%cs, %w0" : "=rm" (seg))
#define getds(seg) __asm ("movw %%ds, %w0" : "=rm" (seg))
#define getes(seg) __asm ("movw %%es, %w0" : "=rm" (seg))
#define getss(seg) __asm ("movw %%ss, %w0" : "=rm" (seg))
#define getfs(seg) __asm ("movw %%fs, %w0" : "=rm" (seg))
#define getgs(seg) __asm ("movw %%gs, %w0" : "=rm" (seg))
//Set segment registers
//EXTREMELY DANGEROUS!!!!
#define setcs(seg) __asm volatile ("movw %w0, %%cs" : : "rm" (seg) : "%cs")
//VERY DANGEROUS!!
#define setds(seg) __asm volatile ("movw %w0, %%ds" : : "rm" (seg) : "%ds")
#define setes(seg) __asm volatile ("movw %w0, %%es" : : "rm" (seg) : "%es")
//VERY DANGEROUS!!
#define setss(seg) __asm volatile ("movw %w0, %%ss" : : "rm" (seg) : "%ss")
#define setfs(seg) __asm volatile ("movw %w0, %%fs" : : "rm" (seg) : "%fs")
#define setgs(seg) __asm volatile ("movw %w0, %%gs" : : "rm" (seg) : "%gs")
//Reset segment registers to cs
#define resetdstocs() __asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%ds" : : : "ax")
#define resetestocs() __asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%es" : : : "ax")
#define resetsstocs() __asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%ss" : : : "ax")
#define resetfstocs() __asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%fs" : : : "ax")
#define resetgstocs() __asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%gs" : : : "ax")
//Reset segment registers to ds
#define resetestods() __asm volatile ("movw %%ds, %%ax\n\tmovw %%ax, %%es" : : : "ax")
#define resetsstods() __asm volatile ("movw %%ds, %%ax\n\tmovw %%ax, %%ss" : : : "ax")
#define resetfstods() __asm volatile ("movw %%ds, %%ax\n\tmovw %%ax, %%fs" : : : "ax")
#define resetgstods() __asm volatile ("movw %%ds, %%ax\n\tmovw %%ax, %%gs" : : : "ax")
//Reset segment registers to ss
#define resetdstoss() __asm volatile ("movw %%ss, %%ax\n\tmovw %%ax, %%ds" : : : "ax")
#define resetestoss() __asm volatile ("movw %%ss, %%ax\n\tmovw %%ax, %%es" : : : "ax")
#define resetfstoss() __asm volatile ("movw %%ss, %%ax\n\tmovw %%ax, %%fs" : : : "ax")
#define resetgstoss() __asm volatile ("movw %%ss, %%ax\n\tmovw %%ax, %%gs" : : : "ax")
//Reset ds and es (the 8086 normal data segment registers) to cs
#define reset8086datasegtocs() __asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%ds\n\tmovw %%ax, %%es" : : : "ax")
//Reset fs and gs (the 80386 extra data segment registers) to cs
#define reset386datasegtocs() __asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%fs\n\tmovw %%ax, %%gs" : : : "ax")
//Reset ds, es, fs and gs to cs
#define resetalldatasegtocs() __asm volatile ("movw %%cs, %%ax\n\tmovw %%ax, %%ds\n\tmovw %%ax, %%es\n\tmovw %%ax, %%fs\n\tmovw %%ax, %%gs" : : : "ax")

/* Explicitly overridden memory operations follow */
//Load 8-bit number
#define segovrcs_memload8(src, dst) __asm volatile ("movb %%cs:%1, %b0" : "=r" (dst) : "m" (src))
#define segovres_memload8(src, dst) __asm volatile ("movb %%es:%1, %b0" : "=r" (dst) : "m" (src))
#define segovrfs_memload8(src, dst) __asm volatile ("movb %%fs:%1, %b0" : "=r" (dst) : "m" (src))
#define segovrgs_memload8(src, dst) __asm volatile ("movb %%gs:%1, %b0" : "=r" (dst) : "m" (src))
//Load 16-bit number
#define segovrcs_memload16(src, dst) __asm volatile ("movw %%cs:%1, %w0" : "=r" (dst) : "m" (src))
#define segovres_memload16(src, dst) __asm volatile ("movw %%es:%1, %w0" : "=r" (dst) : "m" (src))
#define segovrfs_memload16(src, dst) __asm volatile ("movw %%fs:%1, %w0" : "=r" (dst) : "m" (src))
#define segovrgs_memload16(src, dst) __asm volatile ("movw %%gs:%1, %w0" : "=r" (dst) : "m" (src))
//Load 32-bit number
#define segovrcs_memload32(src, dst) __asm volatile ("movl %%cs:%1, %k0" : "=r" (dst) : "m" (src))
#define segovres_memload32(src, dst) __asm volatile ("movl %%es:%1, %k0" : "=r" (dst) : "m" (src))
#define segovrfs_memload32(src, dst) __asm volatile ("movl %%fs:%1, %k0" : "=r" (dst) : "m" (src))
#define segovrgs_memload32(src, dst) __asm volatile ("movl %%gs:%1, %k0" : "=r" (dst) : "m" (src))
//Store 8-bit number
#define segovrcs_memstore8(src, dst) __asm volatile ("movb %b1, %%cs:%0" : "=m" (dst) : "r" (src))
#define segovres_memstore8(src, dst) __asm volatile ("movb %b1, %%es:%0" : "=m" (dst) : "r" (src))
#define segovrfs_memstore8(src, dst) __asm volatile ("movb %b1, %%fs:%0" : "=m" (dst) : "r" (src))
#define segovrgs_memstore8(src, dst) __asm volatile ("movb %b1, %%gs:%0" : "=m" (dst) : "r" (src))
//Store 16-bit number
#define segovrcs_memstore16(src, dst) __asm volatile ("movw %w1, %%cs:%0" : "=m" (dst) : "r" (src))
#define segovres_memstore16(src, dst) __asm volatile ("movw %w1, %%es:%0" : "=m" (dst) : "r" (src))
#define segovrfs_memstore16(src, dst) __asm volatile ("movw %w1, %%fs:%0" : "=m" (dst) : "r" (src))
#define segovrgs_memstore16(src, dst) __asm volatile ("movw %w1, %%gs:%0" : "=m" (dst) : "r" (src))
//Store 32-bit number
#define segovrcs_memstore32(src, dst) __asm volatile ("movl %k1, %%cs:%0" : "=m" (dst) : "r" (src))
#define segovres_memstore32(src, dst) __asm volatile ("movl %k1, %%es:%0" : "=m" (dst) : "r" (src))
#define segovrfs_memstore32(src, dst) __asm volatile ("movl %k1, %%fs:%0" : "=m" (dst) : "r" (src))
#define segovrgs_memstore32(src, dst) __asm volatile ("movl %k1, %%gs:%0" : "=m" (dst) : "r" (src))
//Move 8-bit number (override on destination)
#define segovrdcs_memmove8(src, dst) __asm volatile ("movb %1, %%al\n\tmovb %%al, %%cs:%0" : "=m" (dst) : "m" (src) : "al")
#define segovrdes_memmove8(src, dst) __asm volatile ("movb %1, %%al\n\tmovb %%al, %%es:%0" : "=m" (dst) : "m" (src) : "al")
#define segovrdfs_memmove8(src, dst) __asm volatile ("movb %1, %%al\n\tmovb %%al, %%fs:%0" : "=m" (dst) : "m" (src) : "al")
#define segovrdgs_memmove8(src, dst) __asm volatile ("movb %1, %%al\n\tmovb %%al, %%gs:%0" : "=m" (dst) : "m" (src) : "al")
//Move 8-bit number (override on source)
#define segovrscs_memmove8(src, dst) __asm volatile ("movb %%cs:%1, %%al\n\tmovb %%al, %0" : "=m" (dst) : "m" (src) : "al")
#define segovrses_memmove8(src, dst) __asm volatile ("movb %%es:%1, %%al\n\tmovb %%al, %0" : "=m" (dst) : "m" (src) : "al")
#define segovrsfs_memmove8(src, dst) __asm volatile ("movb %%fs:%1, %%al\n\tmovb %%al, %0" : "=m" (dst) : "m" (src) : "al")
#define segovrsgs_memmove8(src, dst) __asm volatile ("movb %%gs:%1, %%al\n\tmovb %%al, %0" : "=m" (dst) : "m" (src) : "al")
//Move 16-bit number (override on destination)
#define segovrdcs_memmove16(src, dst) __asm volatile ("movw %1, %%ax\n\tmovw %%ax, %%cs:%0" : "=m" (dst) : "m" (src) : "ax")
#define segovrdes_memmove16(src, dst) __asm volatile ("movw %1, %%ax\n\tmovw %%ax, %%es:%0" : "=m" (dst) : "m" (src) : "ax")
#define segovrdfs_memmove16(src, dst) __asm volatile ("movw %1, %%ax\n\tmovw %%ax, %%fs:%0" : "=m" (dst) : "m" (src) : "ax")
#define segovrdgs_memmove16(src, dst) __asm volatile ("movw %1, %%ax\n\tmovw %%ax, %%gs:%0" : "=m" (dst) : "m" (src) : "ax")
//Move 16-bit number (override on source)
#define segovrscs_memmove16(src, dst) __asm volatile ("movw %%cs:%1, %%ax\n\tmovw %%ax, %0" : "=m" (dst) : "m" (src) : "ax")
#define segovrses_memmove16(src, dst) __asm volatile ("movw %%es:%1, %%ax\n\tmovw %%ax, %0" : "=m" (dst) : "m" (src) : "ax")
#define segovrsfs_memmove16(src, dst) __asm volatile ("movw %%fs:%1, %%ax\n\tmovw %%ax, %0" : "=m" (dst) : "m" (src) : "ax")
#define segovrsgs_memmove16(src, dst) __asm volatile ("movw %%gs:%1, %%ax\n\tmovw %%ax, %0" : "=m" (dst) : "m" (src) : "ax")
//Move 32-bit number (override on destination)
#define segovrdcs_memmove32(src, dst) __asm volatile ("movl %1, %%eax\n\tmovl %%eax, %%cs:%0" : "=m" (dst) : "m" (src) : "eax")
#define segovrdes_memmove32(src, dst) __asm volatile ("movl %1, %%eax\n\tmovl %%eax, %%es:%0" : "=m" (dst) : "m" (src) : "eax")
#define segovrdfs_memmove32(src, dst) __asm volatile ("movl %1, %%eax\n\tmovl %%eax, %%fs:%0" : "=m" (dst) : "m" (src) : "eax")
#define segovrdgs_memmove32(src, dst) __asm volatile ("movl %1, %%eax\n\tmovl %%eax, %%gs:%0" : "=m" (dst) : "m" (src) : "eax")
//Move 32-bit number (override on source)
#define segovrscs_memmove32(src, dst) __asm volatile ("movl %%cs:%1, %%eax\n\tmovl %%eax, %0" : "=m" (dst) : "m" (src) : "eax")
#define segovrses_memmove32(src, dst) __asm volatile ("movl %%es:%1, %%eax\n\tmovl %%eax, %0" : "=m" (dst) : "m" (src) : "eax")
#define segovrsfs_memmove32(src, dst) __asm volatile ("movl %%fs:%1, %%eax\n\tmovl %%eax, %0" : "=m" (dst) : "m" (src) : "eax")
#define segovrsgs_memmove32(src, dst) __asm volatile ("movl %%gs:%1, %%eax\n\tmovl %%eax, %0" : "=m" (dst) : "m" (src) : "eax")
