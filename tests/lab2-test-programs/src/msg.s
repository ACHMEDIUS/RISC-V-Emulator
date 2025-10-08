# This program is a little strange / obfuscated on purpose to test your
# instructions.
	.text
	.align	4
	.globl	_start
	.type	_start, @function
_start:
	# Address of serial output device
	li	a0,512
	# Write some secret message to serial out
	li	a1,0x6c
	li	a6,10
	li	a7,9
	sb	a1,0(a0)
	li	a1,0x33
	sb	a1,0(a0)
	sb	a1,0(a0)
	li	a2,0x74
	sb	a2,0(a0)
	addi	a1,a1,-0x13
	sb	a1,0(a0)
	li	a1,0x4c
	sb	a1,0(a0)
	lui	a3,%hi(Q)
	addi	a3,a3,%lo(Q)
	ld	a1,0(a3)
	sb	a1,0(a0)
	sub	a1,a1,a7
	sb	a1,0(a0)
	addi	a0,a0,-4
	li	a1,0x43
	sb	a1,4(a0)
	add	a1,a1,0x10
	sb	a1,4(a0)
	addi	a0,a0,-4
	sb	a6,8(a0)
	nop
	nop
	nop
	nop
	nop
	.word 0xddffccff # test end marker
	.size _start, .-_start
# data segment
	.data
	.align 4
	.comm Q,16,4
Q:
	.word 0x00000049
	.word 0x00000000
