	.text
	.align 4
	.global _start
	.type _start, @function
_start:
	lui x1,38
	nop
	nop
	addi x3,x1,-1023
	lui x5,524287
	nop
	nop
	add x5,x5,x5
	nop
	nop
	add x5,x5,x5
	nop
	nop
	addiw x6,x5,22
	subw x7,x5,x3
	nop
	nop
	nop
	nop
	nop
	.word 0xddffccff # test end marker
	.size _start, .-_start
# Post conditions:
#  x1 contains 155648 (0x26000)
#  x3 contains 154625 (0x25c01)
#  x5 contains 8589918208 (0x1ffffc000)
#  x6 contains 4294950934 (0xffffffffffffc016)
#  x7 contains 4294796287 (0xfffffffffffd63ff)
