//x86 interrupt management
#pragma once

#define INTFUNC __attribute__((interrupt))

//Trigger software interrupt (please use specific wrappers instead of a raw swint call)
#define swint(num) asm ("int %0" : : "i" (num))

//Pointer to the first element of the interrupt vector table, you can use this to straightforwardly index the table
#define INTERRUPT_VECTOR_TABLE ((unsigned long*)0x0)

//Set the interrupt vector for interrupt 'num' to be fnseg:fnptr
#define setinterrupt(num, fnseg, fnptr) asm ("xor %%ax, %%ax\nmov %%ax, %%ds\nmov %w0, %%bx\nshl %%bx\nshl %%bx\nmov %w2, (%%bx)\nmov %w1, 2(%%bx)\nmov %%cs, %%ax\nmov %%ax, %%ds" : : "rmi" (num), "rmi" (fnseg), "rmi" (fnptr) : "ax", "bx")

//Get the interrupt vector for interrupt 'num' and put its segment into 'fnseg' and its offset into 'fnptr'
#define getinterrupt(num, fnseg, fnptr) asm ("xor %%ax, %%ax\nmov %%ax, %%ds\nmov %w0, %%bx\nshl %%bx\nshl %%bx\nmov (%%bx), %w2\nmov 2(%%bx), %w1\nmov %%cs, %%ax\nmov %%ax, %%ds" : : "rmi" (num), "rmi" (fnseg), "rmi" (fnptr) : "ax", "bx")

//Use these to turn interrupts off and on
#define intsoff() asm ("cli")
#define intson() asm ("sti")

typedef void interruptFunc(void*);
typedef interruptFunc* interruptFuncPtr;