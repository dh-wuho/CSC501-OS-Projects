.section	.data
.section	.text
.globl zfunction

zfunction:
	pushl %ebp
	movl  %esp, %ebp

	andl $0xFFFC01FF, 8(%ebp)
	sarl $0x8, 8(%ebp)
	orl $0xFF000000, 8(%ebp)
	movl 8(%ebp), %eax

	leave
	ret
