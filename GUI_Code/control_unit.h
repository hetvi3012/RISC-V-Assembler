#ifndef CONTROL_UNIT_H
#define CONTROL_UNIT_H

#include <cstdint>
#include "pipeline_registers.h" // For ALUOpType

// ----------------------------------------------
// Define ALU NOP operation for no-ops or invalids
// ----------------------------------------------
#ifndef ALU_NOP
#define ALU_NOP ALU_PASS  // Or define separately in ALUOpType if needed
#endif

// ----------------------------------------------------------
// Structure: ControlSignals to be passed from Decode to EX
// ----------------------------------------------------------
struct ControlSignals {
    bool RegWrite = false;
    bool ALUSrc   = false;
    bool MemRead  = false;
    bool MemWrite = false;
    bool MemToReg = false;
    bool Branch   = false;
    bool Jump     = false;
    bool Halt     = false;

    ALUOpType aluOp = ALU_NOP;
};

// ----------------------------------------------------------
// Function: Get control signals based on opcode, funct3/7
// ----------------------------------------------------------
inline ControlSignals getControlSignals(uint8_t opcode, uint8_t funct3, uint8_t funct7) {
    ControlSignals ctrl = {};  // Safe zero-initialization

    switch (opcode) {
    // --------------------------
    // R-type (e.g. add, sub, xor)
    // --------------------------
    case 0x33:
        ctrl.RegWrite = true;
        ctrl.ALUSrc   = false;
        switch (funct3) {
        case 0x0: ctrl.aluOp = (funct7 == 0x20) ? ALU_SUB : ALU_ADD; break;
        case 0x1: ctrl.aluOp = ALU_SLL;  break;
        case 0x2: ctrl.aluOp = ALU_SLT;  break;
        case 0x3: ctrl.aluOp = ALU_SLTU; break;
        case 0x4: ctrl.aluOp = ALU_XOR;  break;
        case 0x5: ctrl.aluOp = (funct7 == 0x20) ? ALU_SRA : ALU_SRL; break;
        case 0x6: ctrl.aluOp = ALU_OR;   break;
        case 0x7: ctrl.aluOp = ALU_AND;  break;
        }
        break;

        // --------------------------
        // I-type (immediate ALU ops)
        // --------------------------
    case 0x13:
        ctrl.RegWrite = true;
        ctrl.ALUSrc   = true;
        switch (funct3) {
        case 0x0: ctrl.aluOp = ALU_ADD; break;
        case 0x1: ctrl.aluOp = ALU_SLL; break;
        case 0x2: ctrl.aluOp = ALU_SLT; break;
        case 0x3: ctrl.aluOp = ALU_SLTU; break;
        case 0x4: ctrl.aluOp = ALU_XOR; break;
        case 0x5: ctrl.aluOp = (funct7 == 0x20) ? ALU_SRA : ALU_SRL; break;
        case 0x6: ctrl.aluOp = ALU_OR; break;
        case 0x7: ctrl.aluOp = ALU_AND; break;
        }
        break;

        // --------------------------
        // Load instructions
        // --------------------------
    case 0x03:
        ctrl.RegWrite = true;
        ctrl.ALUSrc   = true;
        ctrl.MemRead  = true;
        ctrl.MemToReg = true;
        ctrl.aluOp    = ALU_ADD;
        break;

        // --------------------------
        // Store instructions
        // --------------------------
    case 0x23:
        ctrl.ALUSrc   = true;
        ctrl.MemWrite = true;
        ctrl.aluOp    = ALU_ADD;
        break;

        // --------------------------
        // Branch instructions
        // --------------------------
    case 0x63:
        ctrl.Branch = true;
        switch (funct3) {
        case 0x0: ctrl.aluOp = ALU_EQ;  break;  // BEQ
        case 0x1: ctrl.aluOp = ALU_NE;  break;  // BNE
        case 0x4: ctrl.aluOp = ALU_LT;  break;  // BLT
        case 0x5: ctrl.aluOp = ALU_GE;  break;  // BGE
        case 0x6: ctrl.aluOp = ALU_LTU; break;  // BLTU
        case 0x7: ctrl.aluOp = ALU_GEU; break;  // BGEU
        }
        break;

        // --------------------------
        // Jumps
        // --------------------------
    case 0x6F:  // JAL
        ctrl.RegWrite = true;
        ctrl.Jump     = true;
        ctrl.aluOp    = ALU_PASS;
        break;

    case 0x67:  // JALR
        ctrl.RegWrite = true;
        ctrl.Jump     = true;
        ctrl.ALUSrc   = true;
        ctrl.aluOp    = ALU_PASS;
        break;

        // --------------------------
        // LUI / AUIPC
        // --------------------------
    case 0x37:  // LUI
        ctrl.RegWrite = true;
        ctrl.aluOp    = ALU_PASS;
        break;

    case 0x17:  // AUIPC
        ctrl.RegWrite = true;
        ctrl.aluOp    = ALU_ADD;
        break;

        // --------------------------
        // HALT
        // --------------------------
    case 0x00:
        ctrl.Halt = true;
        break;

    default:
        ctrl.aluOp = ALU_NOP;
        break;
    }

    return ctrl;
}

#endif // CONTROL_UNIT_H
