	.file	"generator.c"
	.text
.globl my_generator

my_generator:
	pushl	%ebp
	movl	%esp, %ebp
	movl	staticVar.1250, %eax
	addl	$1, %eax
	movl	%eax, staticVar.1250
	movl	staticVar.1250, %eax
	popl	%ebp
	ret

.globl main


main:
	pushl	%ebp
	movl	%esp, %ebp
	call	my_generator
	movl	$0, %eax
	popl	%ebp
	ret

.local	staticVar.1250
.comm	staticVar.1250,4,4

