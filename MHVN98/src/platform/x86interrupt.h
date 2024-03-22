//x86 interrupt management
//Maxim Hoxha 2023-2024

#pragma once

#define INTFUNC __attribute__((interrupt)) __far

//Trigger software interrupt (please use specific wrappers instead of a raw swint call)
#define swint(num) asm ("int %0" : : "i" (num))

//Pointer to the first element of the interrupt vector table, you can use this to straightforwardly index the table
#define INTERRUPT_VECTOR_TABLE ((__far InterruptFuncPtr*)0x0)

//Set the interrupt vector for interrupt 'num' to be fnseg:fnptr
#define setinterrupt(num, fnseg, fnptr) __asm ("xor %%ax, %%ax\n\tmov %%ax, %%ds\n\tshl %%bx\n\tshl %%bx\n\tmov %w1, (%%bx)\n\tmov %w0, 2(%%bx)\n\tmov %%cs, %%ax\n\tmov %%ax, %%ds" : : "b" (num), "ri" (fnseg), "ri" (fnptr) : "ax")

//Get the interrupt vector for interrupt 'num' and put its segment into 'fnseg' and its offset into 'fnptr'
#define getinterrupt(num, fnseg, fnptr) __asm ("xor %%ax, %%ax\n\tmov %%ax, %%ds\n\tshl %%bx\n\tshl %%bx\n\tmov (%%bx), %w1\n\tmov 2(%%bx), %w0\n\tmov %%cs, %%ax\n\tmov %%ax, %%ds" : : "b" (num), "ri" (fnseg), "ri" (fnptr) : "ax")

//Use these to turn interrupts off and on
#define intsoff() __asm ("cli")
#define intson() __asm ("sti")

typedef INTFUNC void InterruptFunc();
typedef InterruptFunc* InterruptFuncPtr;

//Get the raw entry of an interrupt vector
__attribute__((always_inline)) InterruptFuncPtr GetInterruptFunctionRaw(unsigned char num)
{
    return INTERRUPT_VECTOR_TABLE[num];
}

//Set the raw entry of an interrupt vector
__attribute__((always_inline)) inline void SetInterruptFunctionRaw(unsigned char num, InterruptFuncPtr ptr)
{
    INTERRUPT_VECTOR_TABLE[num] = ptr;
}

//Set the entry of an interrupt vector with a function pointer in the same segment
__attribute__((always_inline)) inline void SetInterruptFunction(unsigned char num, InterruptFuncPtr ptr)
{
    unsigned short codeSeg;
    getcs(codeSeg);
    INTERRUPT_VECTOR_TABLE[num] = ((unsigned long)ptr & 0x0000FFFF) | ((unsigned long)codeSeg << 16);
}
