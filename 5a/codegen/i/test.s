.text


.globl	poo

poo:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	$5
	popl	%eax
	popl	%ebx
	movl	%ebx, %ebp
	ret

.globl	foo

foo:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	8(%ebp)
	pushl	12(%ebp)
	subl	$4, %esp
	pushl	-8(%ebp)
	pushl	-4(%ebp)
	call	poo
	movl	%eax, -4(%ebx)
	pushl	-4(%ebp)
	popl	%eax
	movl	%ebp, %esp
	popl	%ebx
	movl	%ebx, %ebp
	ret

.globl	Main

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
	call	poo
	movl	%eax, -4(%ebx)
	pushl	$6
	pushl	$20
	popl	%eax
	popl	%ebx
	addl	%ebx, %eax
	pushl	%eax
	pushl	-4(%ebp)
	popl	%eax
	popl	%ebx
	addl	%ebx, %eax
	pushl	%eax
	popl	%eax
	movl	%eax, -12(%ebp)
	pushl	$4
	popl	%eax
	movl	%eax, -4(%ebp)
	pushl	-12(%ebp)
	pushl	$5
	popl	%eax
	popl	%ecx
	cmp	%eax, %ecx
	je	T0
	movl	$0, %eax
	jmp	D1
T0:	movl	$1, %eax
D1:	pushl	%eax
	popl	%ecx
	movl $0, %ebx
	cmp %eax, %ebx
	je E2
	pushl	$7
	popl	%eax
	movl	%eax, -4(%ebp)
	jmp	D3
E2:	pushl	$6
	popl	%eax
	movl	%eax, -4(%ebp)
D3:	pushl	-4(%ebp)
	popl	%eax
	movl	%ebp, %esp
	popl	%ebx
	movl	%ebx, %ebp
	ret
