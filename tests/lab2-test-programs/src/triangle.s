# Some constants
	.set IO, 0x200			# Address of serial output device
	.set CHAR, 0x78			# Character to write
	.set NEWLINE, 10
	.set LINES, 8
#
	.text
	.align	4
	.globl	_start
	.type	_start, @function
_start:
	li	a0,IO
	li	a1,CHAR
#
	li	a2,1			# Line counter
print_line:
	li	a3,0			# Character counter
print_char:
	sb	a1,0(a0)
#
	addiw	a3,a3,1
	blt	a3,a2,print_char	# char_counter < line_counter ?
#
	li	a6,NEWLINE
	sb	a6,0(a0)
#
	li	a6,LINES
	addiw	a2,a2,1
	ble	a2,a6,print_line
# Done
	nop
	nop
	nop
	nop
	nop
	.word 0xddffccff # test end marker
	.size _start, .-_start
