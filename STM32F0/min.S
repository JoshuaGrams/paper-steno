.section .interrupt_vectors

	.word 0x20000000 + 16*1024  @ initial stack pointer
	.word _start
	.rept 46
	.word spinlock              @ interrupt vectors
	.endr


	.text

spinlock:
	b spinlock

	.global _start
_start:
	mov r0,#0
0:	add r0,#1
	b 0b
