/ ls
	.=4096

start:
	cmp	$2, cx
	ja	error
	jne	1f
	mov	2(si), 0f
1:	sys	handle; error
	sys	brk; end
	mov	$0x2020, buf+16
	sys	open; 0: dot; 0
	mov	ax, bx
1:	mov	bx, ax
	sys	read; buf; 16
	je	9f
	mov	$1, ax
	sys	write; buf+2; 16
	j	1b
9:	sys	exit; 0

error:
	mov	$2, bx
	sys	write; 1f; dot-1f
	sys	exit; 1

1:	<ls error\n>
dot:	<.\0>
buf:	.=.+18
end:
