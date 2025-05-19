#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <unordered_map>
#include <vector>
#include <string>
using namespace std;

// ------------------------
// Global Registers & Memory
// ------------------------
extern unsigned int R[32];               // Register file (x0 - x31; x0 is always 0)
extern unsigned int PC;                  // Program Counter
extern unsigned int tempPC;              // Holds PC value for the current instruction
extern unsigned int instruction_word;    // Current fetched instruction
extern unsigned int imm;                 // Immediate value
extern unordered_map<unsigned int, unsigned char> MEM; // Memory (address -> byte)
extern int clock_cycles;                 // Clock cycle counter

// ------------------------
// Global Control Signals
// ------------------------
extern bool RegWrite;   // Write-back to register file?
extern bool MemRead;    // Memory read operation?
extern bool MemWrite;   // Memory write operation?
extern bool MemToReg;   // Write-back from memory?
extern bool ALUSrc;     // ALU source (immediate or register)?
extern bool Branch;     // Branch instruction flag?
extern bool Jump;       // Jump instruction flag?

// ------------------------
// Global ALU & Memory Stage Values
// ------------------------
extern unsigned int ALU_result;   // ALU result (or effective address)
extern unsigned int memory_data;  // Data loaded from memory

// ------------------------
// Instruction Decoded Fields & Immediate Values
// ------------------------
extern unsigned int decoded_opcode;
extern unsigned int decoded_rd;
extern unsigned int decoded_funct3;
extern unsigned int decoded_rs1;
extern unsigned int decoded_rs2;
extern unsigned int decoded_funct7;

// ------------------------
// Instruction Type Enumeration
// ------------------------
enum InstructionType {
    INST_ADD, INST_SLL, INST_SLT, INST_SLTU, INST_XOR, INST_SRL, INST_OR, INST_AND,
    INST_SUB, INST_SRA,
    INST_ADDI, INST_SLLI, INST_SLTI, INST_SLTIU, INST_XORI, INST_SRLI, INST_SRAI, INST_ORI, INST_ANDI,
    INST_LB, INST_LH, INST_LW, INST_LBU, INST_LHU,
    INST_JALR,
    INST_SB, INST_SH, INST_SW,
    INST_BEQ, INST_BNE, INST_BLT, INST_BGE, INST_BLTU, INST_BGEU,
    INST_LUI, INST_AUIPC,
    INST_JAL,
    INST_EXIT, INST_UNKNOWN
};
extern InstructionType currInst;

// ------------------------
// Halt Control Signal
// ------------------------
extern bool Halt;  // When true, the simulator will exit after the current write-back.

// ------------------------
// Simulator State Structure (for stepping/backtracking)
// ------------------------
struct SimState {
    unsigned int PC;
    unsigned int R[32];
    unordered_map<unsigned int, unsigned char> MEM;
    int clock_cycles;
};
extern vector<SimState> simHistory;

// ------------------------
// Function Prototypes
// ------------------------

// Utility Functions
string to_hex(unsigned int value);
int sign_extend(unsigned int value, int bits);

// Program Memory and Debug Printing
void load_program_memory(const string &file_name);
void print_registers();
void print_memory();

// State Management
void saveState();

// Simulation Pipeline Stages
void fetch();
void decode();
void execute();
void mem_access();
void write_back();

// Initialize
void load_program_memory();

// Simulator Control Functions
// Executes all simulation cycles (steps) and saves the state.
void runRiscvSim();
// Executes one simulation cycle (step) and saves the state.
void stepRiscvSim();
// Reverts to the previous simulation state (prev).
void prevRiscvSim();
// Resets (clears) the simulator: clears history and reinitializes registers, memory, and PC.
void resetRiscvSim();

#endif // SIMULATOR_H
