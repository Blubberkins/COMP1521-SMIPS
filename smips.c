// Accepts 32-bit instruction codes for MIPS and shows MIPS code and output
// Aaron Wang z5308498
// 7/8/2020

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define MAX_INSTRUCTION_CODES 1000

int instruction_printer(uint32_t code, int line, int *reg);
int instruction_processor(uint32_t code, int line, int *reg,
int *vValues, int *aValues, int *syscalls);

int main(int argc, char *argv[]) {

    // Initialise an array to store register values
    // reg[2] = $v0
    // reg[4] = $a0
    int *reg = malloc(32 * sizeof(int));

    // Initialises all registers to value '0'
    for (int i = 0; i < 32; i++) {

        reg[i] = 0;

    }

    // Initialise an array to store values of $v0
    int *vValues = malloc(MAX_INSTRUCTION_CODES * sizeof(int));

    // Initialises all vValues to value '0'
    for (int i = 0; i < MAX_INSTRUCTION_CODES; i++) {

        vValues[i] = 0;

    }  

    // Initialise an array to store values of $a0
    int *aValues = malloc(MAX_INSTRUCTION_CODES * sizeof(int));

    // Initialises all aValues to value '0'
    for (int i = 0; i < MAX_INSTRUCTION_CODES; i++) {

        aValues[i] = 0;

    }

    // Initialises an array to store the number of syscalls and invalid syscalls
    // syscall[0] = number of syscalls
    // syscall[1] = 0 for valid syscalls, 1 for invalid syscalls
    // syscall[2] = value of the invalid syscall instruction
    int *syscalls = malloc(3 * sizeof(int));  
    
    // Initialises all syscall array values to '0'
    for (int i = 0; i < 3; i++) {

        syscalls[i] = 0;

    }

    // Read instruction codes from given file into an array

    FILE *read_file = fopen(argv[1], "r");

    int instructionCodes[MAX_INSTRUCTION_CODES];
    int num_lines = 0;
    // scans the hexvalues into an array called instructionCodes
    while (num_lines < MAX_INSTRUCTION_CODES && 
    fscanf(read_file, "%x", &instructionCodes[num_lines]) == 1) {

        num_lines++;
        
    }

    // Determine the instruction for each hexvalue and call its
    // print function, also checks for invalid instructions

    printf("Program\n");            // Prints first line of program output

    int print_return = 0;
    for (int line = 0; line < num_lines; line++) {

        if (print_return == 1) {    // Exit instruction is called

            break;

        } else if (print_return == 2) { // Invalid instruction code is given

            return 1;

        } else {

            print_return = 
            instruction_printer(instructionCodes[line], line, reg);

        }
        
    }

    // Processes the instructions in order, accounting for jumps backwards

    for (int line = 0; line < num_lines; line++) {

        reg[0] = 0; // Sets register 0 to value 0 every loop as its value
                    // should always remain 0

        // instruction_processor returns the same line value unless it
        // processes a jump instruction, then it returns the line the jump
        // has moved the execution to
        line = instruction_processor(instructionCodes[line], line, reg, 
                                        vValues, aValues, syscalls);

        if (line == -1) {   // Exit instruction is called

            line = num_lines;   // Exit loop

        }

    }

    fclose(read_file);  // Closes the file being read from

    // Prints the output of the program instructions

    printf("Output\n");

    if (syscalls[1] == 1) { // syscalls[1] = 1 indicates an invalid syscall

        printf("Unknown system call: %d\n", syscalls[2]);

    } else {

        for (int i = 0; vValues[i] != 0; i++) {

            if (vValues[i] == 1) {  // $v0 = 1 (print integer)

                printf("%d", aValues[i]);

            } else if (vValues[i] == 11) {  // $v0 = 11 (print char)

                printf("%c", aValues[i]);

            }

        }

    }

    // Prints the non-zero register values after program execution

    printf("Registers After Execution\n");

    if (syscalls[1] == 0) { // syscalls[1] = 0 indicates all syscalls were valid

        for (int i = 0; i < 32; i++) {

            if (reg[i] != 0) {  // if register value != 0

                if (i < 10) {   // if line < 10 add extra space before '='

                    printf("$%d  = %d\n", i, reg[i]);

                } else {

                    printf("$%d = %d\n", i, reg[i]);

                }

            }

        }

    } else if (syscalls[1] == 1) {  // syscalls[1] = 1 indicates an invalid
                                    // syscall

        printf("$2  = %d\n", syscalls[2]);

    }

    // Free all malloc'ed arrays

    free(reg);
    free(vValues);
    free(aValues);
    free(syscalls);

    return 0;

}

int instruction_printer(uint32_t code, int line, int *reg) {

    // Prints additional space if the line number < 10 (formatting)
    if (line < 10) {

        printf(" ");

    }

    if (code == 12) {   // code = 1100

        printf(" %d: syscall\n", line);

        if (reg[2] == 10) {   // if $v0 = 10 (exit instruction is called)

            return 1;

        }

    } else {

        // isolate opcode, s, t into their separate values
        uint32_t opcode = code >> 26;
        uint32_t s = (code << 6) >> 27;
        uint32_t t = (code << 11) >> 27;

        if (opcode == 0 || opcode == 28) {  // opcode = 000000 or 011100

            // isolate d, functcode into their separate values
            uint32_t d = (code << 16) >> 27;
            uint32_t functcode = (code << 26) >> 26;

            if (opcode == 28) { // opcode = 011100

                printf(" %d: mul  $%d, $%d, $%d\n", line, d, s, t);

            } else if (functcode == 32) {   // functcode = 100000

                printf(" %d: add  $%d, $%d, $%d\n", line, d, s, t);

            } else if (functcode == 34) {   // functcode = 100010

                printf(" %d: sub  $%d, $%d, $%d\n", line, d, s, t);

            } else if (functcode == 36) {   // functcode = 100100

                printf(" %d: and  $%d, $%d, $%d\n", line, d, s, t);

            } else if (functcode == 37) {   // functcode = 100101

                printf(" %d: or  $%d, $%d, $%d\n", line, d, s, t);

            } else if (functcode == 42) {   // functcode = 101010

                printf(" %d: slt  $%d, $%d, $%d\n", line, d, s, t);

            }

        } else {

            int16_t immediate = (code << 16) >> 16;

            if (opcode == 4) {      // opcode = 000100
            
                printf(" %d: beq  $%d, $%d, %d\n", line, s, t, immediate);

            } else if (opcode == 5) {   // opcode = 000101

                printf(" %d: bne  $%d, $%d, %d\n", line, s, t, immediate);

            } else if (opcode == 8) {   // opcode = 001000

                printf(" %d: addi $%d, $%d, %d\n", line, t, s, immediate);

            } else if (opcode == 10) {   // opcode = 001010

                printf(" %d: slti $%d, $%d, %d\n", line, t, s, immediate);

            } else if (opcode == 12) {   // opcode = 001100

                printf(" %d: andi $%d, $%d, %d\n", line, t, s, immediate);

            } else if (opcode == 13) {   // opcode = 001101

                printf(" %d: ori  $%d, $%d, %d\n", line, t, s, immediate);

            } else if (opcode == 15) {   // opcode = 001111

                printf(" %d: lui  $%d, %d\n", line, t, immediate);

            } else {

                printf("Invalid instruction code: %08X", code);
                return 2;

            }
        
        }

    }

    return 0;
}

int instruction_processor(uint32_t code, int line, int *reg,
int *vValues, int *aValues, int *syscalls) {

    if (code == 12) {   // if instruction is syscall

        if (reg[2] == 1) {  // if $v0 = 1 (print integer)

            vValues[syscalls[0]] = 1;   // store $v0 (1) in vValues
            aValues[syscalls[0]] = reg[4];  // store $a0 in aValues
            syscalls[0]++;  // increment number of syscalls called

        } else if (reg[2] == 10) {  // if $v0 = 10 (exit)

            return -1;  // line is set to -1 to indicate the exit request

        } else if (reg[2] == 11) {  // if $v0 = 11 (print char)

            vValues[syscalls[0]] = 11;   // store $v0 (11) in vValues
            aValues[syscalls[0]] = reg[4];  // store $a0 in aValues
            syscalls[0]++;  // increment number of syscalls called

        } else {    // else invalid $v0

            syscalls[1] = 1;    // syscalls[1] = 1 indicates invalid syscall
            syscalls[2] = reg[2];   // syscalls[2] stores invalid $v0 value

        }

    } else {

        // isolate opcode, s, t into their separate values
        uint32_t opcode = code >> 26;
        uint32_t s = (code << 6) >> 27;
        uint32_t t = (code << 11) >> 27;

        if (opcode == 0 || opcode == 28) {  // opcode = 000000 or 011100

            // isolate d, functcode into their separate values
            uint32_t d = (code << 16) >> 27;
            uint32_t functcode = (code << 26) >> 26;

            if (opcode == 28) { // mul: $d = $s * $t

                reg[d] = reg[s] * reg[t];

            } else if (functcode == 32) {   // add: $d = $s + $t

                reg[d] = reg[s] + reg[t];

            } else if (functcode == 34) {   // sub: $d = $s - $t

                reg[d] = reg[s] - reg[t];

            } else if (functcode == 36) {   // sub: $d = $s & $t

                reg[d] = reg[s] & reg[t];

            } else if (functcode == 37) {   // or: $d = $s | $t

                reg[d] = reg[s] | reg[t];

            } else if (functcode == 42) {   // slt: $d = 1 if s < t else 0

                if (reg[s] < reg[t]) {

                    reg[d] = 1;

                } else {

                    reg[d] = 0;

                }

            }

        } else {

            int16_t immediate = (code << 16) >> 16;

            if (opcode == 4) {      // beq: if ($s == $t) PC += I
            
                if (reg[s] == reg[t]) {

                    return line + (immediate - 1);  // -1 to account for line++
                                                    // in main function

                }

            } else if (opcode == 5) {   // bne: if ($s != $t) PC += I

                if (reg[s] != reg[t]) {

                    return line + (immediate - 1);  // -1 to account for line++
                                                    // in main function

                }

            } else if (opcode == 8) {   // addi: $t = $s + I

                reg[t] = reg[s] + immediate;

            } else if (opcode == 10) {   // slti: $t = ($s < I)

                reg[t] = (reg[s] < immediate);

            } else if (opcode == 12) {   // andi: $t = $s & I

                reg[t] = reg[s] & immediate;

            } else if (opcode == 13) {   // ori: $t = $s | I

                reg[t] = reg[s] | immediate;

            } else if (opcode == 15) {   // lui: $t = I << 16

                reg[t] = immediate << 16;

            }
        
        }

    }

    return line;
}