;# Unreal mode runtime loader

.intel_syntax noprefix
.code16gcc
.text
	.globl	_realMain
	.def	_realMain;	.scl	2;	.type	32;	.endef
	
	.globl	_setupRelativeFlatPointers
	.def	_setupRelativeFlatPointers;	.scl	2;	.type	32;	.endef

	.p2align 2
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
	mov al, 0x00
	out 0xF2, al ;#PC-98: Open A20 line
	
	mov eax, gdtoffset
	xor ebx, ebx
	mov bx, cs
	shl ebx, 4
	add eax, ebx
	mov gdtoffset, eax ;#Fix GDT data offset
	
	cli
	push ds
	lgdt [gdtinfo]
	mov eax, cr0
	or al, 0x01
	mov cr0, eax ;#Enter x86 protected mode
	jmp Lmain_nocrash ;#Prevent crash on 80486 and under
Lmain_nocrash:
	mov bx, 0x0008
	mov ds, bx ;#Fill segment descriptor cache
	and al, 0xFE
	mov cr0, eax ;#Enter x86 real mode
	pop ds
	sti
	
	;# From this point onwards the processor is now in 'unreal' mode
	;# Of course, this isn't a 'real' mode of the processor
	;# It now allows us to ignore segmentation in our code, except when we have to interface with DOS or the BIOS
	;# Interfacing directly with hardware we will just adjust our pointers appropriately
	
	call _setupRelativeFlatPointers ;#Obtain the proper relative pointers for unreal mode
	call _realMain
	
	mov ah, 0x4C
	int 0x21 ;#DOS return from program
	
	;# We don't need to 'turn off' unreal mode as pointers default to 16-bit and still wrap (?)

.data
gdtinfo:		.dc.w	gdt_end - gdt - 1
gdtoffset:		.dc.l	gdt
			
gdt:			.word	0x0000, 0x0000, 0x0000, 0x0000 ;#required to be empty
				.word	0xFFFF, 0x0000, 0x9200, 0x00CF ;#limit: 0xFFFFFFFF, offset: 0x00000000, data seg
gdt_end: