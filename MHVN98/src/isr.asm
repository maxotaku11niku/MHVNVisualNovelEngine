;# Interrupt service routines

.intel_syntax noprefix
.code16
.text

	.p2align 2
	.globl	_vsyncInterrupt
	.def	_vsyncInterrupt;	.scl	2;	.type	32;	.endef
_vsyncInterrupt:
	push ax
	movb _vsynced, 1
	out 0x64, al	;#PC-98 GDC I/O: CRT interrupt reset
	mov al, 0x20
	out 0x00, al	;#PC-98 interrupt controller: signal end of interrupt
	pop ax
	iret
	
.data
	.globl	_vsynced