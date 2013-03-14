.text

.globl	Main

Main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	pushl	$0
	popl	%eax
	movl	%eax, -12(%ebp)
	pushl	-12(%ebp)
	popl	%ecx
	jecxz	O0
	movl	$0, %eax
	jmp	D1
O0:	movl	$1, %eax
D1:	pushl	%eax
	popl	%eax
	movl	%eax, -12(%ebp)
	pushl	$10
	popl	%eax
	imull	$-1, %eax
	pushl	%eax
	popl	%eax
	movl	%eax, -8(%ebp)
	pushl	$3
	popl	%eax
	movl	%eax, -4(%ebp)
	pushl	-4(%ebp)
	pushl	$5
	popl	%eax
	popl	%ecx
	subl	%eax, %ecx
	jecxz	F3
	shr	$31, %ecx
	jecxz	T2
F3:	movl	$0, %eax
	jmp	D4
T2:	movl	$1, %eax
D4:	pushl	%eax
	popl	%ecx
	movl $0, %ebx
	cmp %eax, %ebx
	je D5
	pushl	$10
	popl	%eax
	movl	%eax, -4(%ebp)
D5:	pushl	-4(%ebp)
	popl	%eax
	movl	%ebp, %esp
	popl	%ebx
	movl	%ebx, %ebp
	ret
