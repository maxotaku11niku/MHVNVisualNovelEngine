//x86 interrupt management
#pragma once

#define INTFUNC __attribute__((interrupt))

//Trigger software interrupt (please use specific wrappers instead of a raw swint call)
#define swint(num) asm ("int %0" : : "i" (num))

//Pointer to the first element of the interrupt vector table, you can use this to straightforwardly index the table
#define INTERRUPT_VECTOR_TABLE ((unsigned long*)0x0)

//Set the interrupt vector for interrupt 'num' to be fnseg:fnptr
#define setinterrupt(num, fnseg, fnptr) asm ("xor %%ax, %%ax\n\tmov %%ax, %%ds\n\tshl %%bx\n\tshl %%bx\n\tmov %w1, (%%bx)\n\tmov %w0, 2(%%bx)\n\tmov %%cs, %%ax\n\tmov %%ax, %%ds" : : "b" (num), "ri" (fnseg), "ri" (fnptr) : "ax")

//Get the interrupt vector for interrupt 'num' and put its segment into 'fnseg' and its offset into 'fnptr'
#define getinterrupt(num, fnseg, fnptr) asm ("xor %%ax, %%ax\n\tmov %%ax, %%ds\n\tshl %%bx\n\tshl %%bx\n\tmov (%%bx), %w1\n\tmov 2(%%bx), %w0\n\tmov %%cs, %%ax\n\tmov %%ax, %%ds" : : "b" (num), "ri" (fnseg), "ri" (fnptr) : "ax")

//Use these to turn interrupts off and on
#define intsoff() asm ("cli")
#define intson() asm ("sti")

typedef void interruptFunc(void*);
typedef interruptFunc* interruptFuncPtr;