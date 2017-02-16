.text

.balign 4

.global fib_iter_a
.func fib_iter_a

fib_iter_a:
	cmp r0, #1
	beq ret1
	cmp r0, #0
	beq ret0
	mov r2, #0  
	mov r3, #1  
	mov r1, #2  
	mov r12, #0  

loop:
	cmp r1, r0
	bgt endloop
	add r12, r2, r3
	mov r2, r3
	mov r3, r12
	add r1, r1, #1
	b loop

endloop:
	mov r0, r12
	bx lr

ret0:
	mov r0, #0
	bx lr
ret1:
	mov r0, #1
	bx lr
