#include <bits/stdc++.h>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;

class DualStreamBuf : public std::streambuf {
    public:
        DualStreamBuf(std::streambuf* buf1, std::streambuf* buf2)
            : buffer1(buf1), buffer2(buf2) {}
    
    protected:
        virtual int overflow(int c) override {
            if (c == EOF) return !EOF;
            if (buffer1) buffer1->sputc(c);
            if (buffer2) buffer2->sputc(c);
            return c;
        }
    
        virtual int sync() override {
            int result1 = buffer1 ? buffer1->pubsync() : 0;
            int result2 = buffer2 ? buffer2->pubsync() : 0;
            return result1 == 0 && result2 == 0 ? 0 : -1;
        }
    
    private:
        std::streambuf* buffer1;
        std::streambuf* buffer2;
};    

// ------------------------
// Helper: Format 32-bit Hexadecimal String
// ------------------------
inline string to_hex(unsigned int value) {
    stringstream ss;
    ss << "0x" << setw(8) << setfill('0') << hex << uppercase << value;
    return ss.str();
}

// ------------------------
// Global Registers & Memory
// ------------------------
unsigned int R[32] = {0, 0, 0x7FFFFFDC, 0x10000000, 0, 0, 0, 0,
                      0, 0, 0x00000001, 0x7FFFFFDC, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0}; // x0 is always 0.
unsigned int PC = 0x0;               // Program Counter
unsigned int tempPC = 0x0;           // Holds PC value for current instruction
unsigned int instruction_word = 0;   // Current fetched instruction
unsigned int imm = 0x0;              // Immediate value
unordered_map<unsigned int, unsigned char> MEM;  // Memory modeled as address->byte
int clock_cycles = 0;                // Clock cycle counter

// ------------------------
// Global Control Signals
// ------------------------
bool RegWrite = false;   // Should write back to a register?
bool MemRead  = false;   // Memory read operation?
bool MemWrite = false;   // Memory write operation?
bool MemToReg = false;   // Write-back data comes from memory?
bool ALUSrc   = false;   // Second ALU operand comes from immediate?
bool Branch   = false;   // Branch instruction?
bool Jump     = false;   // Jump instruction?

// ------------------------
// Global ALU & Memory Stage Values
// ------------------------
unsigned int ALU_result = 0; // Result from ALU (or effective address)
unsigned int memory_data = 0; // Data loaded from memory

// ------------------------
// Instruction Decoded Fields & Immediate Values
// ------------------------
unsigned int decoded_opcode   = 0;
unsigned int decoded_rd       = 0;
unsigned int decoded_funct3   = 0;
unsigned int decoded_rs1      = 0;
unsigned int decoded_rs2      = 0;
unsigned int decoded_funct7   = 0;

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

InstructionType currInst = INST_UNKNOWN;

// ------------------------
// Halt Control Signal
// ------------------------
bool Halt = false;  // When true, the simulator will exit after the current write-back.

// ------------------------
// Structure to Save Simulator State (for stepping/backtracking)
// ------------------------
struct SimState {
    unsigned int PC;
    unsigned int R[32];
    unordered_map<unsigned int, unsigned char> MEM;
    int clock_cycles;
};

vector<SimState> simHistory;  // History of simulation states

// ------------------------
// Helper: Sign Extension Function
// ------------------------
int sign_extend(unsigned int value, int bits) {
    if (value & (1u << (bits - 1))) {
        value |= ~((1u << bits) - 1);
    }
    return (int)value;
}

// ------------------------
// Load Program Memory
// ------------------------
void load_program_memory() {
    ifstream file("output.mc");
    if (!file) {
        cerr << "Error opening file " << "output.mc" << "!" << endl;
        exit(1);
    }
    unsigned int address, data;
    string line;
    while (getline(file, line)) {
        if(line.empty()) continue;
        istringstream iss(line);
        if (iss >> hex >> address >> data) {
            for (int i = 0; i < 4; i++) {
                MEM[address] = data & 0xFF;
                data >>= 8;
                address++;
            }
        }
    }
    file.close();
}

// ------------------------
// Write Register File to a File (for GUI)
// ------------------------
void write_registers_to_file() {
    ofstream regFile("reg_out.mem");
    if (!regFile) {
        cerr << "Error opening reg_out.mem for writing!" << endl;
        return;
    }
    for (int i = 0; i < 32; i++) {
        int reg_val = static_cast<int>(R[i]);
        regFile << "x" << i << ": " << to_hex(R[i])
                << " (" << reg_val << ")" << endl;
    }
    regFile.close();
}

// ------------------------
// Write Memory Contents to a File (for GUI)
// ------------------------
void write_memory_to_file() {
    ofstream memFile("data_out.mem");
    if (!memFile) {
        cerr << "Error opening data_out.mem for writing!" << endl;
        return;
    }
    vector<unsigned int> addresses;
    for (auto &entry : MEM) {
        if(entry.second != 0)
            addresses.push_back(entry.first);
    }
    sort(addresses.begin(), addresses.end());
    for (auto addr : addresses) {
        if (addr < 0x10000000) continue;  // Skip code segment
        int mem_val = static_cast<signed char>(MEM[addr]);
        memFile << to_hex(addr) << " : "
                << "0x" << setw(2) << setfill('0') << hex << uppercase << (unsigned int)MEM[addr]
                << " (" << dec << mem_val << ")" << endl;
    }
    memFile << "Other: 0" << endl;
    memFile.close();
}

// ------------------------
// Print Register File (Only Non-Zero Registers)
// ------------------------
void print_registers() {
    cout << "\n================ Non-Zero Register File State ================\n";
    bool printed = false;
    for (int i = 0; i < 32; i++) {
        if (R[i] != 0) {
            int reg_val = static_cast<int>(R[i]);
            cout << "x" << setw(2) << i << ": " << to_hex(R[i])
                 << " (" << reg_val << ")" << endl;
            printed = true;
        }
    }
    if (!printed)
        cout << "None" << endl;
    cout << "==============================================================\n";
}

// ------------------------
// Print Memory Contents (Only Non-Zero Memory)
// ------------------------
void print_memory() {
    cout << "\n================ Non-Zero Memory Contents ================\n";
    vector<unsigned int> addresses;
    for (auto &entry : MEM) {
        if (entry.second != 0)
            addresses.push_back(entry.first);
    }
    sort(addresses.begin(), addresses.end());
    bool printed = false;
    for (auto addr : addresses) {
        if (addr < 0x10000000) continue;
        int mem_val = static_cast<signed char>(MEM[addr]);
        cout << to_hex(addr) << " : "
             << "0x" << setw(2) << setfill('0') << hex << uppercase << (unsigned int)MEM[addr]
             << " (" << dec << mem_val << ")" << endl;
        printed = true;
    }
    if (!printed)
        cout << "None" << endl;
    cout << "==========================================================\n";
}

// ------------------------
// Simulation Exit: Write Non-Zero Memory & Registers to File and Exit
// ------------------------
void swi_exit() {
    cout << "\nSimulation terminated after " << clock_cycles << " clock cycles." << endl;
    write_memory_to_file();
    write_registers_to_file();
    print_registers();
    print_memory();
    exit(0);
}

// ------------------------
// Save the Current Simulator State (for stepping/backtracking)
// ------------------------
void saveState() {
    SimState state;
    state.PC = PC;
    state.clock_cycles = clock_cycles;
    for (int i = 0; i < 32; i++)
        state.R[i] = R[i];
    state.MEM = MEM;
    simHistory.push_back(state);
}

// ------------------------
// Fetch Stage
// ------------------------
void fetch() {
    instruction_word = 0;
    for (int i = 3; i >= 0; i--) {
        instruction_word <<= 8;
        instruction_word |= MEM[PC + i];
    }
    cout << "\n---------- Fetch Stage ----------" << endl;
    cout << "PC = " << to_hex(PC)
         << " | Fetched Instruction = " << to_hex(instruction_word) << endl;
    tempPC = PC;
    if (instruction_word == 0) {
        Halt = true;
        return;
    }
    PC += 4;
}

// ------------------------
// Decode Stage
// ------------------------
void decode() {
    decoded_opcode  = instruction_word & 0x7F;
    decoded_rd      = (instruction_word >> 7) & 0x1F;
    decoded_funct3  = (instruction_word >> 12) & 0x7;
    decoded_rs1     = (instruction_word >> 15) & 0x1F;
    decoded_rs2     = (instruction_word >> 20) & 0x1F;
    decoded_funct7  = (instruction_word >> 25) & 0x7F;

    RegWrite = false; MemRead = false; MemWrite = false; MemToReg = false;
    ALUSrc = false; Branch = false; Jump = false;
    imm = 0;
    currInst = INST_UNKNOWN;

    cout << "\n---------- Decode Stage ----------" << endl;
    cout << "Opcode = " << to_hex(decoded_opcode)
         << " | rd = x" << dec << decoded_rd
         << " | rs1 = x" << decoded_rs1
         << " | rs2 = x" << decoded_rs2
         << " | funct3 = " << to_hex(decoded_funct3)
         << " | funct7 = " << to_hex(decoded_funct7) << endl;

    switch(decoded_opcode) {
    case 0x33:
        RegWrite = true; ALUSrc = false;
        if(decoded_funct7 == 0x00) {
            switch(decoded_funct3) {
            case 0x0: currInst = INST_ADD;  cout << "Instruction: ADD" << endl; break;
            case 0x1: currInst = INST_SLL;  cout << "Instruction: SLL" << endl; break;
            case 0x2: currInst = INST_SLT;  cout << "Instruction: SLT" << endl; break;
            case 0x3: currInst = INST_SLTU; cout << "Instruction: SLTU" << endl; break;
            case 0x4: currInst = INST_XOR;  cout << "Instruction: XOR" << endl; break;
            case 0x5: currInst = INST_SRL;  cout << "Instruction: SRL" << endl; break;
            case 0x6: currInst = INST_OR;   cout << "Instruction: OR" << endl; break;
            case 0x7: currInst = INST_AND;  cout << "Instruction: AND" << endl; break;
            default:  currInst = INST_UNKNOWN; cout << "Unknown R-type instruction" << endl; break;
            }
        } else if(decoded_funct7 == 0x20) {
            if(decoded_funct3 == 0x0) { currInst = INST_SUB; cout << "Instruction: SUB" << endl; }
            else if(decoded_funct3 == 0x5) { currInst = INST_SRA; cout << "Instruction: SRA" << endl; }
            else { currInst = INST_UNKNOWN; cout << "Unknown R-type instruction" << endl; }
        }
        break;
    case 0x13:
        RegWrite = true; ALUSrc = true;
        imm = sign_extend((instruction_word >> 20) & 0xFFF, 12);
        switch(decoded_funct3) {
        case 0x0: currInst = INST_ADDI;  cout << "Instruction: ADDI" << endl; break;
        case 0x1:
            currInst = INST_SLLI;
            imm = (instruction_word >> 20) & 0x1F;
            cout << "Instruction: SLLI" << endl; break;
        case 0x2: currInst = INST_SLTI;  cout << "Instruction: SLTI" << endl; break;
        case 0x3: currInst = INST_SLTIU; cout << "Instruction: SLTIU" << endl; break;
        case 0x4: currInst = INST_XORI;  cout << "Instruction: XORI" << endl; break;
        case 0x5:
            if ((instruction_word >> 25) == 0x00) {
                currInst = INST_SRLI;
                imm = (instruction_word >> 20) & 0x1F;
                cout << "Instruction: SRLI" << endl;
            } else if ((instruction_word >> 25) == 0x20) {
                currInst = INST_SRAI;
                imm = (instruction_word >> 20) & 0x1F;
                cout << "Instruction: SRAI" << endl;
            } else { currInst = INST_UNKNOWN; cout << "Unknown I-type shift" << endl; }
            break;
        case 0x6: currInst = INST_ORI;   cout << "Instruction: ORI" << endl; break;
        case 0x7: currInst = INST_ANDI;  cout << "Instruction: ANDI" << endl; break;
        default:  currInst = INST_UNKNOWN; cout << "Unknown I-type instruction" << endl; break;
        }
        break;
    case 0x03:
        RegWrite = true; ALUSrc = true; MemRead = true; MemToReg = true;
        imm = sign_extend((instruction_word >> 20) & 0xFFF, 12);
        switch(decoded_funct3) {
        case 0x0: currInst = INST_LB;  cout << "Instruction: LB" << endl; break;
        case 0x1: currInst = INST_LH;  cout << "Instruction: LH" << endl; break;
        case 0x2: currInst = INST_LW;  cout << "Instruction: LW" << endl; break;
        case 0x4: currInst = INST_LBU; cout << "Instruction: LBU" << endl; break;
        case 0x5: currInst = INST_LHU; cout << "Instruction: LHU" << endl; break;
        default:  currInst = INST_UNKNOWN; cout << "Unknown load instruction" << endl; break;
        }
        break;
    case 0x67:
        RegWrite = true; ALUSrc = true; Jump = true;
        imm = sign_extend((instruction_word >> 20) & 0xFFF, 12);
        currInst = INST_JALR;
        cout << "Instruction: JALR" << endl;
        break;
    case 0x23:
        RegWrite = false; ALUSrc = true; MemWrite = true;
        imm = sign_extend(((instruction_word >> 25) << 5) | ((instruction_word >> 7) & 0x1F), 12);
        switch(decoded_funct3) {
        case 0x0: currInst = INST_SB; cout << "Instruction: SB" << endl; break;
        case 0x1: currInst = INST_SH; cout << "Instruction: SH" << endl; break;
        case 0x2: currInst = INST_SW; cout << "Instruction: SW" << endl; break;
        default:  currInst = INST_UNKNOWN; cout << "Unknown store instruction" << endl; break;
        }
        break;
    case 0x63:
        RegWrite = false; ALUSrc = false; Branch = true;
        {
            unsigned int imm_raw = (((instruction_word >> 31) & 0x1) << 12) |
                                   (((instruction_word >> 7)  & 0x1) << 11) |
                                   (((instruction_word >> 25) & 0x3F) << 5) |
                                   (((instruction_word >> 8)  & 0xF) << 1);
            imm = sign_extend(imm_raw, 13);
        }
        switch(decoded_funct3) {
        case 0x0: currInst = INST_BEQ;  cout << "Instruction: BEQ" << endl; break;
        case 0x1: currInst = INST_BNE;  cout << "Instruction: BNE" << endl; break;
        case 0x4: currInst = INST_BLT;  cout << "Instruction: BLT" << endl; break;
        case 0x5: currInst = INST_BGE;  cout << "Instruction: BGE" << endl; break;
        case 0x6: currInst = INST_BLTU; cout << "Instruction: BLTU" << endl; break;
        case 0x7: currInst = INST_BGEU; cout << "Instruction: BGEU" << endl; break;
        default:  currInst = INST_UNKNOWN; cout << "Unknown branch instruction" << endl; break;
        }
        break;
    case 0x37:
        RegWrite = true;
        imm = instruction_word & 0xFFFFF000;
        currInst = INST_LUI;
        cout << "Instruction: LUI" << endl;
        break;
    case 0x17:
        RegWrite = true;
        imm = instruction_word & 0xFFFFF000;
        currInst = INST_AUIPC;
        cout << "Instruction: AUIPC" << endl;
        break;
    case 0x6F:
        RegWrite = true; Jump = true;
        {
            unsigned int imm_raw = (((instruction_word >> 31) & 0x1) << 20) |
                                   (((instruction_word >> 21) & 0x3FF) << 1) |
                                   (((instruction_word >> 20) & 0x1) << 11) |
                                   (((instruction_word >> 12) & 0xFF) << 12);
            imm = sign_extend(imm_raw, 21);
        }
        currInst = INST_JAL;
        cout << "Instruction: JAL" << endl;
        break;
    case 0:
        currInst = INST_EXIT;
        cout << "Instruction: EXIT" << endl;
    default:
        currInst = INST_UNKNOWN;
        cout << "Unknown Instruction" << endl;
        break;
    }

    cout << "Control Signals: "
         << "RegWrite=" << boolalpha << RegWrite << ", ALUSrc=" << ALUSrc
         << ", MemRead=" << MemRead << ", MemWrite=" << MemWrite
         << ", MemToReg=" << MemToReg << ", Branch=" << Branch
         << ", Jump=" << Jump << ", Halt=" << Halt << endl;
}

// ------------------------
// Execute Stage
// ------------------------
void execute() {
    cout << "\n---------- Execute Stage ----------" << endl;
    unsigned int operand1 = R[decoded_rs1];
    unsigned int operand2 = ALUSrc ? imm : R[decoded_rs2];

    switch(currInst) {
    case INST_ADD:
        ALU_result = operand1 + R[decoded_rs2];
        cout << "ADD: " << to_hex(operand1) << " + " << to_hex(R[decoded_rs2])
             << " = " << to_hex(ALU_result) << " (" << static_cast<int>(ALU_result) << ")" << endl;
        break;
    case INST_SLL:
        ALU_result = operand1 << (R[decoded_rs2] & 0x1F);
        cout << "SLL: " << to_hex(operand1) << " << " << (R[decoded_rs2] & 0x1F)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_SLT:
        ALU_result = ((int)operand1 < (int)R[decoded_rs2]) ? 1 : 0;
        cout << "SLT: (" << to_hex(operand1) << " < " << to_hex(R[decoded_rs2])
             << ") = " << to_hex(ALU_result) << endl;
        break;
    case INST_SLTU:
        ALU_result = (operand1 < R[decoded_rs2]) ? 1 : 0;
        cout << "SLTU: (" << to_hex(operand1) << " < " << to_hex(R[decoded_rs2])
             << ") = " << to_hex(ALU_result) << endl;
        break;
    case INST_XOR:
        ALU_result = operand1 ^ R[decoded_rs2];
        cout << "XOR: " << to_hex(operand1) << " ^ " << to_hex(R[decoded_rs2])
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_SRL:
        ALU_result = operand1 >> (R[decoded_rs2] & 0x1F);
        cout << "SRL: " << to_hex(operand1) << " >> " << (R[decoded_rs2] & 0x1F)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_OR:
        ALU_result = operand1 | R[decoded_rs2];
        cout << "OR: " << to_hex(operand1) << " | " << to_hex(R[decoded_rs2])
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_AND:
        ALU_result = operand1 & R[decoded_rs2];
        cout << "AND: " << to_hex(operand1) << " & " << to_hex(R[decoded_rs2])
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_SUB:
        ALU_result = operand1 - R[decoded_rs2];
        cout << "SUB: " << to_hex(operand1) << " - " << to_hex(R[decoded_rs2])
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_SRA: {
        int s_operand1 = static_cast<int>(operand1);
        ALU_result = static_cast<unsigned int>(s_operand1 >> (R[decoded_rs2] & 0x1F));
        cout << "SRA: " << to_hex(operand1) << " (signed " << s_operand1 << ") >> "
             << (R[decoded_rs2] & 0x1F) << " = " << to_hex(ALU_result) << endl;
        break; }
    case INST_ADDI:
        ALU_result = operand1 + imm;
        cout << "ADDI: " << to_hex(operand1) << " + " << to_hex(imm)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_SLLI:
        ALU_result = operand1 << (imm & 0x1F);
        cout << "SLLI: " << to_hex(operand1) << " << " << (imm & 0x1F)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_SLTI:
        ALU_result = ((int)operand1 < imm) ? 1 : 0;
        cout << "SLTI: (" << to_hex(operand1) << " < " << to_hex(imm)
             << ") = " << to_hex(ALU_result) << endl;
        break;
    case INST_SLTIU:
        ALU_result = (operand1 < imm) ? 1 : 0;
        cout << "SLTIU: (" << to_hex(operand1) << " < " << to_hex(imm)
             << ") = " << to_hex(ALU_result) << endl;
        break;
    case INST_XORI:
        ALU_result = operand1 ^ imm;
        cout << "XORI: " << to_hex(operand1) << " ^ " << to_hex(imm)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_SRLI:
        ALU_result = operand1 >> (imm & 0x1F);
        cout << "SRLI: " << to_hex(operand1) << " >> " << (imm & 0x1F)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_SRAI: {
        int s_operand1 = static_cast<int>(operand1);
        ALU_result = static_cast<unsigned int>(s_operand1 >> (imm & 0x1F));
        cout << "SRAI: " << to_hex(operand1) << " (signed " << s_operand1 << ") >> "
             << (imm & 0x1F) << " = " << to_hex(ALU_result) << endl;
        break; }
    case INST_ORI:
        ALU_result = operand1 | imm;
        cout << "ORI: " << to_hex(operand1) << " | " << to_hex(imm)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_ANDI:
        ALU_result = operand1 & imm;
        cout << "ANDI: " << to_hex(operand1) << " & " << to_hex(imm)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_LB: case INST_LH: case INST_LW:
    case INST_LBU: case INST_LHU:
        ALU_result = operand1 + imm;
        cout << "Load Effective Address: " << to_hex(operand1) << " + " << to_hex(imm)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_JALR: {
        unsigned int link = tempPC + 4;
        ALU_result = (operand1 + imm) & ~1;
        cout << "JALR: Jumping to " << to_hex(ALU_result)
             << ", Link Address = " << to_hex(link) << endl;
        PC = ALU_result;
        break; }
    case INST_SB: case INST_SH: case INST_SW:
        ALU_result = operand1 + imm;
        cout << "Store Effective Address: " << to_hex(operand1) << " + " << to_hex(imm)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_BEQ:
        cout << "BEQ: if (" << to_hex(R[decoded_rs1]) << " == " << to_hex(R[decoded_rs2]) << ") ";
        if (R[decoded_rs1] == R[decoded_rs2]) {
            PC = tempPC + imm;
            cout << "branch taken to " << to_hex(PC) << endl;
        } else {
            cout << "branch not taken." << endl;
        }
        break;
    case INST_BNE:
        cout << "BNE: if (" << to_hex(R[decoded_rs1]) << " != " << to_hex(R[decoded_rs2]) << ") ";
        if (R[decoded_rs1] != R[decoded_rs2]) {
            PC = tempPC + imm;
            cout << "branch taken to " << to_hex(PC) << endl;
        } else {
            cout << "branch not taken." << endl;
        }
        break;
    case INST_BLT:
        cout << "BLT: if (" << to_hex(R[decoded_rs1]) << " < " << to_hex(R[decoded_rs2]) << ") ";
        if ((int)R[decoded_rs1] < (int)R[decoded_rs2]) {
            PC = tempPC + imm;
            cout << "branch taken to " << to_hex(PC) << endl;
        } else {
            cout << "branch not taken." << endl;
        }
        break;
    case INST_BGE:
        cout << "BGE: if (" << to_hex(R[decoded_rs1]) << " >= " << to_hex(R[decoded_rs2]) << ") ";
        if ((int)R[decoded_rs1] >= (int)R[decoded_rs2]) {
            PC = tempPC + imm;
            cout << "branch taken to " << to_hex(PC) << endl;
        } else {
            cout << "branch not taken." << endl;
        }
        break;
    case INST_BLTU:
        cout << "BLTU: if (" << to_hex(R[decoded_rs1]) << " < " << to_hex(R[decoded_rs2]) << ") ";
        if (R[decoded_rs1] < R[decoded_rs2]) {
            PC = tempPC + imm;
            cout << "branch taken to " << to_hex(PC) << endl;
        } else {
            cout << "branch not taken." << endl;
        }
        break;
    case INST_BGEU:
        cout << "BGEU: if (" << to_hex(R[decoded_rs1]) << " >= " << to_hex(R[decoded_rs2]) << ") ";
        if (R[decoded_rs1] >= R[decoded_rs2]) {
            PC = tempPC + imm;
            cout << "branch taken to " << to_hex(PC) << endl;
        } else {
            cout << "branch not taken." << endl;
        }
        break;
    case INST_LUI:
        ALU_result = imm;
        cout << "LUI: Result = " << to_hex(ALU_result) << endl;
        break;
    case INST_AUIPC:
        ALU_result = tempPC + imm;
        cout << "AUIPC: " << to_hex(tempPC) << " + " << to_hex(imm)
             << " = " << to_hex(ALU_result) << endl;
        break;
    case INST_JAL:
        cout << "JAL: Jumping to " << to_hex(tempPC + imm)
             << ", Link Address = " << to_hex(tempPC + 4) << endl;
        ALU_result = tempPC + 4;
        PC = tempPC + imm;
        break;
    default:
        cout << "No operation executed." << endl;
        break;
    }
}

// ------------------------
// Memory Access Stage
// ------------------------
void mem_access() {
    cout << "\n---------- Memory Access Stage ----------" << endl;
    if (MemRead) {
        switch(currInst) {
        case INST_LB: {
            int data = static_cast<int>(static_cast<signed char>(MEM[ALU_result]));
            memory_data = static_cast<unsigned int>(data);
            cout << "LB: Loaded byte " << to_hex(static_cast<unsigned int>(data))
                 << " (" << data << ") from address " << to_hex(ALU_result) << endl;
            break; }
        case INST_LH: {
            int data = static_cast<int>(static_cast<short>(MEM[ALU_result] | (MEM[ALU_result+1] << 8)));
            memory_data = static_cast<unsigned int>(data);
            cout << "LH: Loaded halfword " << to_hex(static_cast<unsigned int>(data))
                 << " (" << data << ") from address " << to_hex(ALU_result) << endl;
            break; }
        case INST_LW: {
            int data = 0;
            for (int i = 0; i < 4; i++) {
                data |= (MEM[ALU_result + i] << (8 * i));
            }
            memory_data = static_cast<unsigned int>(data);
            cout << "LW: Loaded word " << to_hex(static_cast<unsigned int>(data))
                 << " (" << data << ") from address " << to_hex(ALU_result) << endl;
            break; }
        case INST_LBU: {
            unsigned int data = MEM[ALU_result];
            memory_data = data;
            cout << "LBU: Loaded unsigned byte " << to_hex(data)
                 << " from address " << to_hex(ALU_result) << endl;
            break; }
        case INST_LHU: {
            unsigned int data = MEM[ALU_result] | (MEM[ALU_result+1] << 8);
            memory_data = data;
            cout << "LHU: Loaded unsigned halfword " << to_hex(data)
                 << " from address " << to_hex(ALU_result) << endl;
            break; }
        default:
            cout << "No memory access performed." << endl;
            break;
        }
    }
    else if (MemWrite) {
        switch(currInst) {
        case INST_SB: {
            unsigned char data = R[decoded_rs2] & 0xFF;
            MEM[ALU_result] = data;
            cout << "SB: Stored byte " << to_hex(static_cast<unsigned int>(data))
                 << " to address " << to_hex(ALU_result) << endl;
            break; }
        case INST_SH: {
            unsigned int data = R[decoded_rs2];
            MEM[ALU_result]   = data & 0xFF;
            MEM[ALU_result+1] = (data >> 8) & 0xFF;
            cout << "SH: Stored halfword " << to_hex(data & 0xFFFF)
                 << " to address " << to_hex(ALU_result) << endl;
            break; }
        case INST_SW: {
            unsigned int data = R[decoded_rs2];
            for (int i = 0; i < 4; i++) {
                MEM[ALU_result + i] = data & 0xFF;
                data >>= 8;
            }
            cout << "SW: Stored word from register x" << decoded_rs2
                 << " to address " << to_hex(ALU_result) << endl;
            break; }
        default:
            cout << "No memory access performed." << endl;
            break;
        }
    }
    else{
        cout << "No memory access performed." << endl;
    }
}

// ------------------------
// Write Back Stage
// ------------------------
void write_back() {
    cout << "\n---------- Write Back Stage ----------" << endl;
    if (RegWrite && decoded_rd != 0) {
        switch(currInst) {
        case INST_ADD: case INST_SLL: case INST_SLT: case INST_SLTU:
        case INST_XOR: case INST_SRL: case INST_OR:  case INST_AND:
        case INST_SUB: case INST_SRA:
        case INST_ADDI: case INST_SLLI: case INST_SLTI: case INST_SLTIU:
        case INST_XORI: case INST_SRLI: case INST_SRAI: case INST_ORI:
        case INST_ANDI: case INST_LUI:  case INST_AUIPC:
            R[decoded_rd] = ALU_result;
            cout << "Writing ALU result " << to_hex(ALU_result)
                 << " to register x" << decoded_rd << " ("
                 << static_cast<int>(R[decoded_rd]) << ")" << endl;
            break;
        case INST_LB: case INST_LH: case INST_LW:
        case INST_LBU: case INST_LHU:
            R[decoded_rd] = memory_data;
            cout << "Writing loaded data " << to_hex(memory_data)
                 << " to register x" << decoded_rd << " ("
                 << static_cast<int>(R[decoded_rd]) << ")" << endl;
            break;
        case INST_JAL: case INST_JALR:
            R[decoded_rd] = ALU_result;
            cout << "Writing link address " << to_hex(ALU_result)
                 << " to register x" << decoded_rd << " ("
                 << static_cast<int>(R[decoded_rd]) << ")" << endl;
            break;
        default:
            break;
        }
    } else {
        cout << "No register write-back performed." << endl;
    }
}

// ------------------------
// Step the Simulator: Execute one cycle and update state files and log file.
// The cycle_log.txt file will contain only the output of the latest cycle.
// ------------------------
void stepRiscvSim() {
    if (Halt) {
        cout << "Simulation is halted. Please reset to start again." << endl;
        return;
    }
    // Save the current state so that we can revert if needed.
    saveState();

    // Prepare to capture output of this cycle.
    streambuf* orig_buf = cout.rdbuf();          // save original streambuf
    ostringstream cycleStream;                     // stream to capture output
    DualStreamBuf dualBuf(orig_buf, cycleStream.rdbuf());
    cout.rdbuf(&dualBuf);

    // Execute one simulation cycle.
    fetch();
    decode();
    execute();
    mem_access();
    write_back();
    clock_cycles++;

    // Restore the original stream buffer.
    cout.rdbuf(orig_buf);

    // Write captured output (only for this cycle) to cycle_log.txt (overwriting previous content).
    ofstream logFile("cycle_log.txt", ios::out | ios::trunc);
    if (!logFile) {
        cerr << "Error opening cycle_log.txt for writing!" << endl;
    } else {
        logFile << cycleStream.str();
        logFile.close();
    }

    // We don't need to save EOF
    if (Halt){
        simHistory.pop_back();
    }

    // Update the state files for registers and memory.
    print_registers();
    print_memory();
    write_registers_to_file();
    write_memory_to_file();
}

// ------------------------
// Revert to the Previous Simulation State (go back one cycle)
// ------------------------
void prevRiscvSim() {
    if (simHistory.empty()) {
        cout << "No previous state available." << endl;
        return;
    }
    SimState state = simHistory.back();
    simHistory.pop_back();
    PC = state.PC;
    clock_cycles = state.clock_cycles;
    for (int i = 0; i < 32; i++)
        R[i] = state.R[i];
    MEM = state.MEM;
    Halt = false;
    cout << "Reverted to previous cycle state. Cycle: " << clock_cycles << endl;

    // Write to cycle_log.txt.
    ofstream logFile("cycle_log.txt", ios::out | ios::trunc);
    if (!logFile) {
        cerr << "Error opening cycle_log.txt for writing!" << endl;
    } else {
        logFile << "Reverted to previous cycle state. Cycle: " + to_string(clock_cycles);
        logFile.close();
    }

    // Update files so that GUI shows the reverted state.
    write_registers_to_file();
    write_memory_to_file();
}

// ------------------------
// Reset the Simulator: Clear history, reinitialize state, and clear the log.
// ------------------------
void resetRiscvSim() {
    simHistory.clear();

    // Reset registers to initial values.
    memset(R, 0, sizeof(R));
    R[2] = 0x7FFFFFDC;
    R[3] = 0x10000000;
    R[10] = 0x00000001;
    R[11] = 0x7FFFFFDC;

    // Reset PC, clock cycles, and halt flag.
    PC = 0;
    clock_cycles = 0;
    Halt = false;

    // Clear memory and reload the program.
    MEM.clear();
    load_program_memory();

    // Clear log file.
    ofstream ofs("cycle_log.txt", ios::out | ios::trunc);
    if (!ofs)
        cerr << "Error clearing cycle_log.txt" << endl;
    ofs.close();

    cout << "Simulation reset." << endl;

    // Update state files.
    write_registers_to_file();
    write_memory_to_file();
}

// ------------------------
// Main Simulation Loop (for testing interactive stepping)
// ------------------------
void runRiscvSim() {
    while (!Halt){
        stepRiscvSim();
    }
}

int main() {
    // Load the program and start the interactive simulation.
    load_program_memory();
    char command;
    cout << "\nSimulation Control: (r)un, (s)tep, (p)rev, (c)lear, (q)uit\n";
    while (true) {
        cout << "Enter command: ";
        cin >> command;
        if (command == 'r'){
            runRiscvSim();
        } else if (command == 's') {
            stepRiscvSim();
        } else if (command == 'p') {
            prevRiscvSim();
        } else if (command == 'c') {
            resetRiscvSim();
        } else if (command == 'q') {
            swi_exit();
        } else {
            cout << "Unknown command. Use s, p, r, or q." << endl;
        }
    }
    return 0;
}