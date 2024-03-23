//x86 interrupt management
//Maxim Hoxha 2023-2024

#pragma once

#define INTFUNC __attribute__((interrupt)) __far

//Trigger software interrupt (please use specific wrappers instead of a raw swint call)
#define swint(num) asm ("int %0" : : "i" (num))

//Pointer to the first element of the interrupt vector table, you can use this to straightforwardly index the table
#define INTERRUPT_VECTOR_TABLE ((__far InterruptFuncPtr*)0x0)

//Use these to turn interrupts off and on
#define intsoff() __asm ("cli")
#define intson() __asm ("sti")

typedef INTFUNC void InterruptFunc();
typedef InterruptFunc* InterruptFuncPtr;

//Get the raw entry of an interrupt vector
inline InterruptFuncPtr GetInterruptFunctionRaw(unsigned char num)
{
    return INTERRUPT_VECTOR_TABLE[num];
}

//Set the raw entry of an interrupt vector
inline void SetInterruptFunctionRaw(unsigned char num, InterruptFuncPtr ptr)
{
    INTERRUPT_VECTOR_TABLE[num] = ptr;
}
