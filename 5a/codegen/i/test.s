.text

.globl	Main

foo:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	8(%ebp)
	pushl	12(%ebp)
	subl	$0, %esp
	pushl	-4(%ebp)
	pushl	-8(%ebp)
	popl	%eax
	popl	%ebx
	addl	%ebx, %eax
	pushl	%eax
	popl	%eax
	movl	%ebp, %esp
	popl	%ebx
	movl	%ebx, %ebp
	ret
Main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$12, %esp
	pushl	$2
	popl	%eax
	movl	%eax, -8(%ebp)
	pushl	$4
	popl	%eax
	movl	%eax, -12(%ebp)
	pushl	$5
	pushl	$1
	call	foo
	movl	%eax, -4(%ebx)
	pushl	-4(%ebp)
	popl	%eax
	movl	%ebp, %esp
	popl	%ebx
	movl	%ebx, %ebp
	ret
