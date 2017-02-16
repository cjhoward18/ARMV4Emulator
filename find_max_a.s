.text

.balign 4

.global find_max_a
.func find_max_a

find_max_a:
	ldr r2, [r0]
	add r0, r0, #4

loop:
	cmp r1, #2
	blt endloop
	ldr r3, [r0]
	cmp r2, r3
	movlt r2,r3
	sub r1, r1, #1
	add r0, r0, #4
	b loop

endloop:
	mov r0, r2
	bx lr
	