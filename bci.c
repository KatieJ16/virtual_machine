/*
 * CS 11, C track, lab 8
 *
 * FILE: bci.c
 *       Implementation of the bytecode interpreter.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bci.h"


/* Define the virtual machine. */
vm_type vm;


/* Initialize the virtual machine. */
void init_vm(void)
{
    int i;

    /*
     * Initialize the stack.  It grows to the right i.e.
     * to higher memory.
     */

    vm.sp = 0;

    for (i = 0; i < STACK_SIZE; i++)
    {
        vm.stack[i] = 0;
    }

    /*
     * Initialize the registers to all zeroes.
     */

    for (i = 0; i < NREGS; i++)
    {
        vm.reg[i] = 0;
    }

    /*
     * Initialize the instruction buffer to all zeroes.
     */

    for (i = 0; i < MAX_INSTS; i++)
    {
        vm.inst[i] = 0;
    }

    vm.ip = 0;
}


/*
 * Helper function to read in integer values which take up varying
 * numbers of bytes from the instruction array 'vm.inst'.
 *
 * NOTES:
 * 1) This function moves 'vm.ip' past the integer's location
 *    in memory.
 * 2) This function assumes that integers take up 4 bytes and are
 *    arranged in a little-endian order (low-order bytes at the
 *    beginning).  This should hold for any pentium-based microprocessor.
 * 3) This function only works for n = 1, 2, or 4 bytes.
 *
 */

int read_n_byte_integer(int n)
{
    int i;
    unsigned char *val_ptr;
    int val = 0;

    /* This only works for 1, 2, or 4 byte integers. */
    assert((n == 1) || (n == 2) || (n == 4));

    val_ptr = (unsigned char *)(&val);

    for (i = 0; i < n; i++)
    {
        *val_ptr = vm.inst[vm.ip];
        val_ptr++;
        vm.ip++;
    }

    return val;
}


/*
 * Machine operations.
 */

void do_push(int n) {
    /* Check that there is space in the stack*/
    if(vm.sp > 255) {
        fprintf(stderr, "Stack full, cannot push");
        exit(0);
    }
    /* make top of stack n and increment stack pointer. */
    vm.stack[vm.sp] = n;
    vm.sp ++;
    printf("%d pushed to stack\n", n);
}

void do_pop(void) {
    /* Check that the stack has something to pop. */
    if(vm.sp < 1) {
        fprintf(stderr, "Stack empty, cannot pop.");
        exit(0);
    }

    /* Pop from stack */
    vm.sp --;
    printf("pop\n");
}

void do_load(int n) {
    /* Check that an value can be added to the stack. */
    if(vm.sp > 255) {
        fprintf(stderr, "Stack ful, cannot load.");
        exit(0);
    }

    /* load value. */
    vm.stack[vm.sp] = vm.reg[n];
    vm.sp ++;
    printf("%d loaded to stack.\n", vm.reg[n]);
}

void do_store(int n) {
    /* make sure there is at least one value in the stack. */
    if(vm.sp < 1) {
        fprintf(stderr, "Stack empty, cannot store.");
        exit(0);
    }

    /* store and pop */
    vm.reg[n] = vm.stack[vm.sp];
    do_pop();
    printf("%d stored.\n", vm.reg[n]);
}

void do_jmp(int n) {
    /* make sure n is a valid instruction. */
    if(n > MAX_INSTS) {
        fprintf(stderr, "Invalid instruction, cannot jump.");
        exit(0);
    }

    /* Jump */
    vm.ip = n;
    printf(" Jump to %d\n", n);
}

void do_jz(int n) {
    /* Check if TOS is zero. */
    if(vm.stack[vm.sp] == 0) {
        do_pop();
        do_jmp(n);
    } else { /* just pop if TOS isn't zero. */
        do_pop();
    }
}

void do_jnz(int n) {
    /* Check if TOS is zero. */
    if(vm.stack[vm.sp] == 0) {
        do_pop();
    } else { /* when not zero */
        do_pop();
        do_jmp(n);
    }
}

void do_add(void) {
    int s1;
    int s2;

    /* check that there are two elements to pop. */
    if(vm.sp < 2) {
        fprintf(stderr, "There are not two elements in the stack, cannot add.");
        exit(0);
    }

    /* valid, now to the adding. */
    s1 = vm.stack[vm.sp];
    s2 = vm.stack[vm.sp - 1];
    do_pop();
    do_pop();
    do_push(s1 + s2);
}

void do_sub(void) {
    int s1;
    int s2;

    /* check that there are two elements to pop. */
    if(vm.sp < 2) {
        fprintf(stderr, "There are not two elements in the stack, cannot subtraction.");
        exit(0);
    }

    /* valid, now to the adding. */
    s1 = vm.stack[vm.sp];
    s2 = vm.stack[vm.sp - 1];
    do_pop();
    do_pop();
    do_push(s2 - s1);

}

void do_mul(void) {
    int s1;
    int s2;
        
    /* check that there are two elements to pop. */
    if(vm.sp < 2) {
        fprintf(stderr, "There are not two elements in the stack, cannot multiply.");
        exit(0);
    }
            
    /* valid, now to the adding. */
    s1 = vm.stack[vm.sp];
    s2 = vm.stack[vm.sp - 1];
    do_pop();
    do_pop();
    do_push(s1 * s2);
}

void do_div(void) {
    int s1;
    int s2;
            
    /* check that there are two elements to pop. */
    if(vm.sp < 2) {
        fprintf(stderr, "There are not two elements in the stack, cannot add.");
        exit(0);
    }

    /* valid, now to the adding. */
    s1 = vm.stack[vm.sp];
    s2 = vm.stack[vm.sp - 1];
    do_pop();
    do_pop();
    do_push(s2 / s1);
}

void do_print(void) {
    int n;
    /* Check there is a number to print. */
    if(vm.sp < 0){
        fprintf(stderr, "There is no number on the stack, cannot print.");
        exit(0);
    }
    
    n = vm.stack[vm.sp];
    fprintf(stdout, "%d\n", n );
    do_pop();
}


/*
 * Stored program execution.
 */

/* Load the stored program into the VM. */
void load_program(FILE *fp)
{
    int nread;
    unsigned char *inst = vm.inst;

    do
    {
        /*
         * Read a single byte at a time and load it into the
         * 'vm.insts' array.  'fread' returns the number of bytes read,
         * or 0 if EOF is hit.
         */

        nread = fread(inst, 1, 1, fp);
        inst++;
    }
    while (nread > 0);
}



/* Execute the stored program in the VM. */
void execute_program(void)
{
    int val;

    vm.ip = 0;
    vm.sp = 0;

    while (1)
    {
        /*
         * Read each instruction and select what to do based on the
         * instruction.  For each instruction you may also have to
         * read in some number of bytes as the arguments to the
         * instruction.
         */

        switch (vm.inst[vm.ip])
        {
        case NOP:
            /* Skip to the next instruction. */
            vm.ip++;
            break;

        case PUSH:
            vm.ip++;

            /* Read in the next 4 bytes. */
            val = read_n_byte_integer(4);
            do_push(val);
            break;

        case POP:
            vm.ip++;
            do_pop();
            break;

        case LOAD:
            vm.ip++;

            /* Read in the next byte. */
            val = read_n_byte_integer(1);
            do_load(val);
            break;

        case STORE:
            vm.ip++;

            /* Read in the next byte. */
            val = read_n_byte_integer(1);
            do_store(val);
            break;

        case JMP:
            vm.ip++;

            /* Read in the next two bytes. */
            val = read_n_byte_integer(2);
            do_jmp(val);
            break;

        case JZ:
            vm.ip++;

            /* Read in the next two bytes. */
            val = read_n_byte_integer(2);
            do_jz(val);
            break;


        case JNZ:
            vm.ip++;

            /* Read in the next two bytes. */
            val = read_n_byte_integer(2);
            do_jnz(val);
            break;


        case ADD:
            vm.ip++;
            do_add();
            break;

        case SUB:
            vm.ip++;
            do_sub();
            break;

        case MUL:
            vm.ip++;
            do_mul();
            break;

        case DIV:
            vm.ip++;
            do_div();
            break;

        case PRINT:
            vm.ip++;
            do_print();
            break;

        case STOP:
            return;

        default:
            fprintf(stderr, "execute_program: invalid instruction: %x\n",
                    vm.inst[vm.ip]);
            fprintf(stderr, "\taborting program!\n");
            return;
        }
    }
}


/* Run the program given the file name in which it's stored. */
void run_program(char *filename)
{
    FILE *fp;

    /* Open the file containing the bytecode. */
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "bci.c: run_program: "
               "error opening file %s; aborting.\n", filename);
        exit(1);
    }

    /* Initialize the virtual machine. */
    init_vm();

    /* Read the bytecode into the instruction buffer. */
    load_program(fp);

    /* Execute the program. */
    execute_program();

    /* Clean up. */
    fclose(fp);
}


