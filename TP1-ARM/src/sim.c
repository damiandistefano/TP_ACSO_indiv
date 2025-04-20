#include <stdio.h>
#include <stdint.h>
#include "shell.h"

void execute_adds_inmediate(uint32_t instruction);
void execute_adds_extended_register(uint32_t instruction);

typedef void (*InstructionFunction)(uint32_t instruction);

typedef struct {
    uint32_t opcode;
    InstructionFunction execute;
} InstructionEntry;

// Tabla de instrucciones
InstructionEntry instruction_set[] = {
    {0b10110001, execute_adds_inmediate},  
    {0b10101011, execute_adds_extended_register},  // SUB
    {0, NULL} // Marca el final
};

void process_instruction() {
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint32_t opcode = (instruction >> 24) & 0x7FF;

    // Buscar el opcode en la tabla
    for (int i = 0; instruction_set[i].execute != NULL; i++) {
        if (instruction_set[i].opcode == opcode) {
            instruction_set[i].execute(instruction);  
            break;
        }
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


// uint32_t decode_instruction(uint32_t instruction){

// }

void execute_adds_inmediate(uint32_t instruction) {
    uint32_t rd = (instruction >> 0) & 0x1F;   // Bits 0-4: Rd
    uint32_t rn = (instruction >> 5) & 0x1F;   // Bits 5-9: Rn
    uint32_t imm12 = (instruction >> 10) & 0xFFF; // Bits 10-21: imm12
    uint32_t shift = (instruction >> 22) & 0x3; // Bits 22-23: shift

    uint64_t operand2 = imm12;

    if (shift == 0b01) {
        operand2 <<= 12;  // LSL #12
    } 

    // Realizar la suma y actualizar flags NZ
    uint64_t result = CURRENT_STATE.REGS[rn] + operand2;
    NEXT_STATE.REGS[rd] = result;

    NEXT_STATE.FLAG_N = (result >> 63) & 1;        // Negative
    NEXT_STATE.FLAG_Z = (result == 0);             // Zero
    
}

void execute_adds_extended_register(uint32_t instruction) {
    uint32_t rd = (instruction >> 0) & 0x1F;   // Bits 0-4: Rd
    uint32_t rn = (instruction >> 5) & 0x1F;   // Bits 5-9: Rn
    uint32_t rm = (instruction >> 16) & 0x1F;  // Bits 16-20: Rm
    uint32_t shift = (instruction >> 22) & 0x3; // Bits 22-23: shift

    uint64_t operand2 = CURRENT_STATE.REGS[rm];

    if (shift == 0b01) {
        operand2 <<= 12;  // LSL #12
    } 

    uint64_t result = CURRENT_STATE.REGS[rn] + operand2;
    NEXT_STATE.REGS[rd] = result;

    NEXT_STATE.FLAG_N = (result >> 63) & 1;        // Negative
    NEXT_STATE.FLAG_Z = (result == 0);             // Zero
}


void execute_adds_register(uint32_t instruction) {
    uint32_t rd = (instruction >> 0) & 0x1F;   // Bits 0-4: Rd
    uint32_t rn = (instruction >> 5) & 0x1F;   // Bits 5-9: Rn
    uint32_t rm = (instruction >> 16) & 0x1F;  // Bits 16-20: Rm

    uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];
    
    NEXT_STATE.REGS[rd] = result;

    NEXT_STATE.FLAG_N = (result >> 63) & 1;  
    NEXT_STATE.FLAG_Z = (result == 0);  
}
// void execute_sub(uint32_t instruction) {
//     uint32_t rd = (instruction >> 0) & 0x1F;
//     uint32_t rn = (instruction >> 5) & 0x1F;
//     uint32_t rm = (instruction >> 16) & 0x1F;

//     NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];
// }