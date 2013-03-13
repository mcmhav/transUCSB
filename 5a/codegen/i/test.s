.text

.globl	Main

Main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	pushl	$0
	popl	%eax
	movl	%eax, -8(%ebp)
	pushl	$2
	popl	%eax
	fabs	, %eax
	pushl	%eax
	popl	%eax
	movl	%eax, -4(%ebp)
	pushl	-8(%ebp)
	popl	%ecx
	jecxz	O1
	movl	$0, %eax
	jmp	D2
O1:	movl	$1, %eax
D2:	pushl	%eax
	popl	%eax
	movl	%eax, -8(%ebp)
	pushl	-4(%ebp)
	pushl	$5
	popl	%eax
	popl	%ecx
	subl	%eax, %ecx
	jecxz	F4
	shr	$31, %ecx
	jecxz	T3
F4:	movl	$0, %eax
	jmp	D5
T3:	movl	$1, %eax
D5:	pushl	%eax
	popl	%ecx
	movl $0, %ebx
	cmp %eax, %ebx
	je D6
	pushl	$10
	popl	%eax
	movl	%eax, -4(%ebp)
D6:	pushl	-4(%ebp)
	popl	%eax
	movl	%ebp, %esp
	popl	%ebx
	movl	%ebx, %ebp
	ret
