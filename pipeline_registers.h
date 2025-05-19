#ifndef PIPELINE_REGISTERS_H
#define PIPELINE_REGISTERS_H

#include <cstdint>
#include <string>

// -----------------------------------------
// ALU Operation Types
// -----------------------------------------
enum ALUOpType {
    ALU_ADD, ALU_SUB, ALU_AND, ALU_OR, ALU_XOR,
    ALU_SLL, ALU_SRL, ALU_SRA,
    ALU_SLT, ALU_SLTU,
    ALU_PASS,  // For LUI
    ALU_EQ,    // For BEQ
    ALU_NE,    // For BNE
    ALU_LT,    // For BLT
    ALU_GE,    // For BGE
    ALU_LTU,   // For BLTU
    ALU_GEU    // For BGEU
};

// -----------------------------------------
// Global Pipeline Control Signals
// -----------------------------------------
struct PipelineControl {
    bool stallIF = false;
    bool flushIF = false;
    bool stallID = false;
    bool flushID = false;
    int stallIFCount = 0;  // Count of stall cycles to hold IF stage
};

// -----------------------------------------
// IF/ID Pipeline Register
// -----------------------------------------
struct IF_ID_Register {
    uint32_t instruction = 0;
    uint32_t pc = 0;
    bool valid = false;
};

// -----------------------------------------
// ID/EX Pipeline Register
// -----------------------------------------
struct ID_EX_Register {
    uint32_t pc = 0;
    uint32_t instruction = 0;

    // Register values
    uint32_t rs1Val = 0;
    uint32_t rs2Val = 0;

    // Decoded fields
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t rd = 0;
    uint32_t imm = 0;
    uint8_t funct3 = 0;
    uint8_t funct7 = 0;
    uint8_t opcode = 0;

    // Control signals
    bool RegWrite = false;
    bool ALUSrc = false;
    bool MemRead = false;
    bool MemWrite = false;
    bool MemToReg = false;
    bool Branch = false;
    bool Jump = false;
    bool Halt = false;

    bool valid = false;

    ALUOpType aluOp = ALU_ADD; // New: ALU control signal
};

// -----------------------------------------
// EX/MEM Pipeline Register
// -----------------------------------------
struct EX_MEM_Register {
    uint32_t pc = 0;
    uint32_t aluResult = 0;
    uint32_t rs2Val = 0;
    uint8_t rd = 0;

    uint8_t funct3 = 0;  // Needed for memoryAccess stage

    // Control signals
    bool RegWrite = false;
    bool MemRead = false;
    bool MemWrite = false;
    bool MemToReg = false;
    bool Branch = false;
    bool Jump = false;
    bool Halt = false;

    bool valid = false;
};

// -----------------------------------------
// MEM/WB Pipeline Register
// -----------------------------------------
struct MEM_WB_Register {
    uint32_t pc = 0;
    uint32_t memData = 0;
    uint32_t aluResult = 0;
    uint8_t rd = 0;

    // Control signals
    bool RegWrite = false;
    bool MemToReg = false;
    bool Halt = false;

    bool valid = false;
};

#endif // PIPELINE_REGISTERS_H
