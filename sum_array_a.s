.text

.balign 4

.global sum_array_a
.func sum_array_a

sum_array_a:
	mov r2, #0

loop:	
	cmp r1, #1
	blt endloop
	ldr r3, [r0]
	add r0, r0, #4
	add r2, r2, r3
	sub r1, r1, #1
	b loop
	
endloop:
	mov r0, r2
	bx lr

