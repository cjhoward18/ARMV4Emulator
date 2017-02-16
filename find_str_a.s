.text

.balign 4

.global find_str_a
.func find_str_a

find_str_a:
	//r1 is substring address
	//r0 is whole string address
	//push {r4, r5, r6}
	sub sp, sp, #4
	str r4, [sp]
	sub sp, sp, #4
	str r5, [sp]
	sub sp, sp, #4
	str r6, [sp]
	mov r2, r1 //substring
	mov r3, #0 //whole
	mov r12, #0 //returnindex
	mov r6, #0
	//r4 is string value
	// r5 is substring value

loop:
	ldrb r4, [r0]
	cmp r4, #0
	beq returnneg
	ldrb r5, [r1]
	cmp r4, r5
	beq collect
	add r0, r0, #1
	mov r1, r2
	mov r6, #0
	add r3, r3, #1
	b loop

collect:
	cmp r6, #0 
	moveq r12, r3
	add r6, r3, #1
	add r3, r3, #1 
	add r0, r0, #1
	add r1, r1, #1
	ldrb r5, [r1]
	cmp r5, #0
	beq returnindex
	b loop

returnindex:
	add sp, sp, #4
	ldr r4, [sp]
	add sp, sp, #4
	ldr r5, [sp]
	add sp, sp, #4
	ldr r6, [sp]
	add sp, sp, #4
	mov r0, r12
	bx lr

returnneg:
	add sp, sp, #4
	ldr r4, [sp]
	add sp, sp, #4
	ldr r5, [sp]
	add sp, sp, #4
	ldr r6, [sp]
	add sp, sp, #4
	mov r0, #-1
	bx lr

