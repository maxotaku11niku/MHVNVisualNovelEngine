;# MHVN98 - PC98 executable for the MHVN Visual Novel Engine
;# Copyright (c) 2023-2024 Maxim Hoxha

;# Permission is hereby granted, free of charge, to any person obtaining a copy
;# of this software and associated documentation files (the "Software"), to deal
;# in the Software without restriction, including without limitation the rights
;# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;# copies of the Software, and to permit persons to whom the Software is
;# furnished to do so, subject to the following conditions:

;# The above copyright notice and this permission notice shall be included in all
;# copies or substantial portions of the Software.

;# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;# SOFTWARE.

;# Interrupt functions

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

