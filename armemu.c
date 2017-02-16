#include <stdbool.h>
#include <stdio.h>

#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

int find_max_a(int *a, int len);
int sum_array_a(int *a, int n);
int fib_rec_a(int n);
int fib_iter_a(int n);
int find_str_a(char *s, char *sub);

struct arm_state {
    unsigned int regs[NREGS];
    unsigned int cpsr;
    unsigned char stack[STACK_SIZE];
    int datainst;
    int memoryinst;
    int binst;
    int numinstr;
};

void init_arm_state(struct arm_state *as, unsigned int *func,
                   unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
    int i;

    /* zero out all arm state */
    for (i = 0; i < NREGS; i++) {
        as->regs[i] = 0;
    }

    as->cpsr = 0;

    for (i = 0; i < STACK_SIZE; i++) {
        as->stack[i] = 0;
    }

    as->regs[PC] = (unsigned int) func;
    as->regs[SP] = (unsigned int) &as->stack[STACK_SIZE];
    as->regs[LR] = 0;
    as->datainst = 0;
    as->memoryinst = 0;
    as->binst = 0;
    as->numinstr = 0;
    as->regs[0] = arg0;
    as->regs[1] = arg1;
    as->regs[2] = arg2;
    as->regs[3] = arg3;
}

bool is_bx_inst(unsigned int iw)
{
    unsigned int bx_code;

    bx_code = (iw >> 4) & 0x00FFFFFF;
    return (bx_code == 0b000100101111111111110001);
}

bool is_add_inst(unsigned int iw)
{
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == 0b0100);
}

bool is_b_inst(unsigned int iw)
{
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 25) & 0b111; 

    return (op == 0b101);
}
bool is_sub_inst(unsigned int iw)
{
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == 0b0010);
}

bool is_mov_inst(unsigned int iw)
{
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == 0b1101);
}

bool is_ldr_inst(unsigned int iw)
{
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b1;
    opcode = (iw >> 20) & 0b1;

    return (op == 1) && (opcode == 1);
}

bool is_str_inst(unsigned int iw)
{
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b1;
    opcode = (iw >> 20) & 0b1;

    return (op == 1) && (opcode == 0);
}

bool is_cmp_inst(unsigned int iw)
{
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == 0b1010);
}

void armemu_b(struct arm_state *state) 
{
    unsigned int iw;
    int offset;
    iw = *((unsigned int *) state->regs[PC]);
    if ((iw >>  24) & 0b1 == 1) {
        state->regs[LR] = state->regs[PC] + 4;
    }
    if((iw >> 23) & 0b1 == 1) { // if negative offset
        offset = (iw & 0b00000000111111111111111111111111);
        offset += 0xFF000000;
        offset = ((~offset) +1) * -4;
        state->regs[PC] = state->regs[PC] + 8 + offset;       
        } else {     
            offset = (iw & 0b00000000111111111111111111111111) * 4;
            state->regs[PC] = state->regs[PC] + 8 + offset;
        }
    state->numinstr++;
    state->binst++;
}

void armemu_add(struct arm_state *state)
{
    unsigned int iw;
    unsigned int rd, rn ;
    int rm;

    iw = *((unsigned int *) state->regs[PC]);
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if (iw >> 25 & 0b1 == 1) { //immediate
        rm = iw & 0xFF;
        state->regs[rd] = state->regs[rn] + rm;
    } else {
        rm = iw & 0xF;
        state->regs[rd] = state->regs[rn] + state->regs[rm];
    }

    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
    state->numinstr++;
    state->datainst++;
}

void armemu_sub(struct arm_state *state)
{
    unsigned int iw;
    unsigned int rd, rn;
    int rm;

    iw = *((unsigned int *) state->regs[PC]);
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if (iw >> 25 & 0b1 == 1) { //immediate
        rm = iw & 0xFF;
        state->regs[rd] = state->regs[rn] - rm;
    } else {
        rm = iw & 0xF;
        state->regs[rd] = state->regs[rn] - state->regs[rm];
    }
    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
    state->numinstr++;
    state->datainst++;
}

int check_cond(struct arm_state *state) {

    unsigned int iw;
    unsigned int rd, op2;
    iw = *((unsigned int *) state->regs[PC]);
    if (iw >> 28 == 0b1011) {
        if ((state->cpsr >> 31) == 1) { // less than cond
            return 1;
        } else {
            return 0;
        }
    } else if (iw >> 28 == 0b0000) { //equal to cond
        if ((state->cpsr >> 30)  == 0b01) {
            return 1;
        } else {
            return 0;
        }
    } else if (iw >> 28 == 0b1100) { // greater than cond
        if ((state->cpsr >> 30)  == 0b00) {
            return 1;
        } else {
            return 0;
        }
    }
}

void armemu_mov(struct arm_state *state)
{
    unsigned int iw;
    unsigned int rd, op2;

    iw = *((unsigned int *) state->regs[PC]);
    rd = (iw >> 12) & 0xF;
    
    if (iw >> 25 & 0b1 == 1) { //immediate
        op2 = iw & 0xFF;
        state->regs[rd] = op2;

    } else {
        op2 = iw & 0xF;
        state->regs[rd] = state->regs[op2];
    }   
    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
    state->numinstr++;
    state->datainst++;
}

void armemu_str(struct arm_state *state) 
{
    unsigned int iw;

    iw = *((unsigned int *) state->regs[PC]);
    unsigned int rd = ((iw >> 12) & 0xF);
    unsigned int val = state->regs[rd];
    unsigned int rn = ((iw >> 16) & 0xF);
    *(unsigned int*)state->regs[rn] = val;

    if (rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
    state->numinstr++;
    state->memoryinst++;
}

void armemu_ldr(struct arm_state *state) 
{
    unsigned int iw;

    iw = *((unsigned int *) state->regs[PC]);
    unsigned int rn = ((iw >> 16) & 0xF);
    unsigned int rd = ((iw >> 12) & 0xF);

    if ((iw >> 22) & 0b1 == 1) { //ldrb
        unsigned int val = (*(unsigned char*)state->regs[rn]) | 0b00000000000000000000000000000000;
        state->regs[rd] = val;
    } else {
        unsigned int* val = (unsigned int*)state->regs[rn];
        state->regs[rd] = *val;
    }
    if (rd != PC) {
            state->regs[PC] = state->regs[PC] + 4;
        }
    state->numinstr++;
    state->memoryinst++;
}

void armemu_cmp(struct arm_state *state)
{
    unsigned int iw;
    unsigned int rd, rn, rm;

    iw = *((unsigned int *) state->regs[PC]);
    rd = (iw >> 12) & 0xF;
    rn = (iw >> 16) & 0xF;

    if (iw >> 25 & 0b1 == 1) { //immediate 
        rm = iw & 0xFF;
        if ((int) (state->regs[rn] - rm) == 0) {  // equal
            state->cpsr |= 0b01000000000000000000000000000000;
            state->cpsr &= 0b01111111111111111111111111111111;
        } else if ((int) (state->regs[rn] - rm) < 0) { //less than
            state->cpsr |= 0b10000000000000000000000000000000;
            state->cpsr &= 0b10111111111111111111111111111111;
        } else {
            state->cpsr &= 0b00111111111111111111111111111111;
        }
        state->regs[PC] = state->regs[PC] + 4;
    } else {
        rm = iw & 0xF;
        if ((int) (state->regs[rn] - state->regs[rm]) == 0) { // equal
            state->cpsr |= 0b01000000000000000000000000000000;
            state->cpsr &= 0b01111111111111111111111111111111;
        } else if ((int) (state->regs[rn] - state->regs[rm]) < 0) { // less than
            state->cpsr |= 0b10000000000000000000000000000000;
            state->cpsr &= 0b10111111111111111111111111111111;
        } else {                            
            state->cpsr &= 0b00111111111111111111111111111111;
        }
        state->regs[PC] = state->regs[PC] + 4;
        state->numinstr++;
        state->datainst++;
    }   
}

void armemu_bx(struct arm_state *state)
{
    unsigned int iw;
    unsigned int rn;

    iw = *((unsigned int *) state->regs[PC]);
    rn = iw & 0b1111;

    state->regs[PC] = state->regs[rn];
    state->binst++;
    state->numinstr++;
}

void armemu_one(struct arm_state *state)
{
    unsigned int iw;
    
    iw = *((unsigned int *) state->regs[PC]);

    //branch instructions
    if (is_bx_inst(iw)) {
        armemu_bx(state);
    } else if (is_b_inst(iw)) {
        if (iw >> 28 == 0b1110) {
            armemu_b(state);
        } else {
            if (check_cond(state) == 1) {
                armemu_b(state);
            } else {
                state->regs[PC] = state->regs[PC] + 4;
            }
        }
    }
    // data processing instructions
      else if (is_add_inst(iw)) {
        armemu_add(state);
    } else if (is_mov_inst(iw)) {
        if (iw >> 28 == 0b1110) {
            armemu_mov(state);
        } else {
            if (check_cond(state) == 1) {
                armemu_mov(state);
            } else {
                state->regs[PC] = state->regs[PC] + 4;
            }
        }
    } else if (is_cmp_inst(iw)) {
        armemu_cmp(state);
    } else if (is_sub_inst(iw)) {
        armemu_sub(state);
    } 
    // memory instructions
    else if (is_ldr_inst(iw)) {
        armemu_ldr(state);
    } else if (is_str_inst(iw)) {
        armemu_str(state);
    } 
}

unsigned int armemu(struct arm_state *state)
{
    while (state->regs[PC] != 0) {
        armemu_one(state);
    }

    return state->regs[0];
}
                    
int main(int argc, char **argv)
{
    struct arm_state state;
    unsigned int r, s;
    int big[1000];
    int i;

    char k[10] = {'k','p','l','k','l','c','e','p','e','i'}; 
    char l[2] = {'e', 'p'};

    for (i = 0; i < 1000; i++) {
        big[i] = i;
    }

    init_arm_state(&state, (unsigned int *) fib_rec_a, 20, 0, 0, 0);
    s = fib_rec_a(20);
    printf("arm version of fib_rec of 20 : %d\n\n", s);
    r = armemu(&state);
    printf("fib recursion of 20 on armemu = %d\n", r);
    printf("number of total instructions is : %d\n", state.numinstr);
    printf("number of branch instructions is : %d\n", state.binst);
    printf("number of data processing instructions is : %d\n", state.datainst);
    printf("number of memory instructions is : %d\n\n", state.memoryinst);

    init_arm_state(&state, (unsigned int *) fib_rec_a, 10, 0, 0, 0);
    s = fib_rec_a(10);
    printf("arm version of fib_rec of 10: %d\n\n", s);
    r = armemu(&state);
    printf("fib recursion of 10 on armemu = %d\n", r);
    printf("number of total instructions is : %d\n", state.numinstr);
    printf("number of branch instructions is : %d\n", state.binst);
    printf("number of data processing instructions is : %d\n", state.datainst);
    printf("number of memory instructions is : %d\n\n", state.memoryinst);

    init_arm_state(&state, (unsigned int *) fib_iter_a, 20, 0, 0, 0);
    s = fib_iter_a(20);
    printf("arm version of fib_iter : %d\n\n", s);
    r = armemu(&state);
    printf("fib iterative of 20 on armemu = %d\n", r);
    printf("number of total instructions is : %d\n", state.numinstr);
    printf("number of branch instructions is : %d\n", state.binst);
    printf("number of data processing instructions is : %d\n", state.datainst);
    printf("number of memory instructions is : %d\n\n", state.memoryinst);

    init_arm_state(&state, (unsigned int *) fib_iter_a, 10, 0, 0, 0);
    s = fib_iter_a(10);
    printf("arm version of fib_iter of 10 : %d\n\n", s);
    r = armemu(&state);
    printf("fib iterative of 10 on armemu = %d\n", r);
    printf("number of total instructions is : %d\n", state.numinstr);
    printf("number of branch instructions is : %d\n", state.binst);
    printf("number of data processing instructions is : %d\n", state.datainst);
    printf("number of memory instructions is : %d\n\n", state.memoryinst);


    init_arm_state(&state, (unsigned int *) find_str_a, (unsigned int) &k[0], (unsigned int) &l[0], 0, 0);
    s = find_str_a(k,l);
    printf("arm version of find str is : %d\n\n", s);
    r = armemu(&state);
    printf("location of string on armemu is = %d\n", r);
    printf("number of total instructions is : %d\n", state.numinstr);
    printf("number of branch instructions is : %d\n", state.binst);
    printf("number of data processing instructions is : %d\n", state.datainst);
    printf("number of memory instructions is : %d\n\n", state.memoryinst);

    init_arm_state(&state, (unsigned int *) find_max_a, (unsigned int) &big[0], 1000, 0, 0);
    s = find_max_a(big, 1000);
    printf("arm version of find_max is : %d\n\n", s);
    r = armemu(&state);
    printf("max number on armemu is = %d\n", r);
    printf("number of total instructions is : %d\n", state.numinstr);
    printf("number of branch instructions is : %d\n", state.binst);
    printf("number of data processing instructions is : %d\n", state.datainst);
    printf("number of memory instructions is : %d\n\n", state.memoryinst);

    init_arm_state(&state, (unsigned int *) sum_array_a, (unsigned int) &big[0], 1000, 0, 0);
    s = sum_array_a(big, 1000);
    printf("arm version of sum of numbers 1 to 999 : %d\n\n", s);
    r = armemu(&state);
    printf("sum of numbers 1 to 999 on armemu is = %d\n", r);
    printf("number of total instructions is : %d\n", state.numinstr);
    printf("number of branch instructions is : %d\n", state.binst);
    printf("number of data processing instructions is : %d\n", state.datainst);
    printf("number of memory instructions is : %d\n\n", state.memoryinst);

    return 0;
}
