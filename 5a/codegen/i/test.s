.text

.globl	Main

foo:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$0, %esp
	pushl	$2
	popl	%eax
	movl	%ebp, %esp
	popl	%ebx
	movl	%ebx, %ebp
	ret
Main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	call	foo
	movl	%eax, -4(%ebx)
	pushl	-4(%ebp)
	popl	%eax
	movl	%ebp, %esp
	popl	%ebx
	movl	%ebx, %ebp
	ret
