.text

.balign 4

.global fib_rec_a
.func fib_rec_a

fib_rec_a:
	sub sp, sp, #4
	str lr, [sp]
	sub sp, sp, #4
	str r4, [sp]
	sub sp, sp, #4
	str r0, [sp]
	cmp r0, #2
	blt exit
	sub r0, r0, #1
	bl fib_rec_a
	mov r4, r0
	ldr r0, [sp]
	add sp, sp, #4
	sub r0, r0, #2
	sub sp, sp, #4
	str r0, [sp]
	bl fib_rec_a
	add r0, r4, r0

exit:
	add sp, sp, #4
	ldr r4, [sp]
	add sp, sp, #4
	ldr lr, [sp]
	add sp, sp, #4
	bx lr
