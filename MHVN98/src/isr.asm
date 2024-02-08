;# Interrupt service routines

.intel_syntax noprefix
.code16
.text

	.p2align 2
	.globl	VsyncInterrupt
	;#.def	VsyncInterrupt;	.scl	2;	.type	32;	.endef
VsyncInterrupt:
	push ax
	movb ss:vsynced, 1
	out 0x64, al	;#PC-98 GDC I/O: CRT interrupt reset
	mov al, 0x20
	out 0x00, al	;#PC-98 interrupt controller: signal end of interrupt
	pop ax
	iret
	
.bss
	.globl	vsynced
	vsynced: .skip 1,0

