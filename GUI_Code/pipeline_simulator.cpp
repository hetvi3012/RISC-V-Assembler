#include <pipeline_simulator.h>

#include <iostream>
#include "pipeline_registers.h"
#include "knobs.h"
#include "branchprediction.h"
#include "control_unit.h"

namespace pipeline{

int data_transfer_instructions = 0;
int ALU_instructions = 0;
int control_instructions = 0;

unsigned int R[32] = {0, 0, 0x7FFFFFDC, 0x10000000, 0, 0, 0, 0,
                      0, 0, 0x00000001, 0x7FFFFFDC, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0};

std::unordered_map<unsigned int, unsigned char> MEM;
unsigned int PC = 0;
unsigned int nextPC;
bool Halt = false;
int clock_cycles = 0;

void print_registers() {
    std::cout << "\n===== Register File =====\n";
    for (int i = 0; i < 32; i++) {
        std::cout << "x" << std::dec << i << ": " << to_hex(R[i]) << "\n";
    }
}

void print_memory() {
    std::cout << "\n===== Memory (non-zero) =====\n";
    for (auto& entry : MEM) {
        if (entry.second != 0)
            std::cout << to_hex(entry.first) << ": " << to_hex(entry.second) << "\n";
    }
}

void load_program_memory(const std::string& filename) {
    std::ifstream infile(filename);
    std::string line;
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string addr_str, data_str;
        ss >> addr_str >> data_str;
        unsigned int addr = std::stoul(addr_str, nullptr, 16);
        unsigned int data = std::stoul(data_str, nullptr, 16);
        for (int i = 0; i < 4; i++) {
            MEM[addr + i] = (data >> (8 * i)) & 0xFF;
        }
    }
}

void write_registers_to_file(const std::string& filename) {
    std::ofstream outfile(filename);
    for (int i = 0; i < 32; i++) {
        outfile << to_hex(R[i]) << "\n";
    }
}

void write_memory_to_file(const std::string& filename) {
    std::ofstream outfile(filename);
    for (unsigned int i = 0; i < 4096; i++) {
        outfile << to_hex(MEM[i]) << "\n";
    }
}

// flushing and stalling control signals
PipelineControl pipelineControl;

// Global pipeline registers
IF_ID_Register if_id;
ID_EX_Register id_ex;
EX_MEM_Register ex_mem;
MEM_WB_Register mem_wb;

std::ofstream knob5File;

bool stopFetching = false;  // Prevents fetching new instructions after HALT

// Stats
int totalCycles = 0;
int dataStalls = 0;
int controlStalls = 0;
int dataHazards = 0;
int controlHazards = 0;
int branchMispredictions = 0;

// Global branch predictor
BranchPredictor BPU;

// Helper: Check RAW hazard
int checkRAWStalls(uint8_t rs, const KnobManager& knobs) {
    int stalls = 0;
    if (rs == 0) return 0;

    // Case 1: Data forwarding is OFF — fallback to old logic
    if (!knobs.dataForwarding) {
        if (ex_mem.valid && ex_mem.RegWrite && ex_mem.rd != 0 && ex_mem.rd == rs) {
            stalls = max(stalls, 2);
        }
        if (mem_wb.valid && mem_wb.RegWrite && mem_wb.rd != 0 && mem_wb.rd == rs) {
            stalls = max(stalls, 1);
        }
    }

    // Case 2: Data forwarding is ON — check for Load-Use hazard
    else {
        if (ex_mem.valid && ex_mem.MemRead && ex_mem.rd != 0 && ex_mem.rd == rs) {
            // Load-use hazard: even with forwarding, we need 1 stall
            stalls = max(stalls, 1);
        }
    }


    if (stalls > 0 && pipelineControl.stallIFCount == 0) dataHazards++;
    return stalls;
}

// Function declarations
int instructionFetch(const KnobManager& knobs);
pair<int, vector<vector<int>>> instructionDecode(const KnobManager& knobs);
int executeStage(const KnobManager& knobs);
int memoryAccess(const KnobManager& knobs);
int writeBack(const KnobManager& knobs);
void printPipelineRegisters(const KnobManager& knobs);
void flushPipeline();
bool isPipelineEmpty();

void printRegisterFile() {
    print_registers();
}

void resetPipelinedSimulator(){
    // Reset pipeline control signals
    pipelineControl = PipelineControl();  // assumes default constructor resets values

    // Reset pipeline registers
    if_id = IF_ID_Register();
    id_ex = ID_EX_Register();
    ex_mem = EX_MEM_Register();
    mem_wb = MEM_WB_Register();

    // Reset simulation flags
    stopFetching = false;

    // Reset statistics
    totalCycles = 0;
    dataStalls = 0;
    controlStalls = 0;
    dataHazards = 0;
    controlHazards = 0;
    branchMispredictions = 0;
    data_transfer_instructions = 0;
    ALU_instructions = 0;
    control_instructions = 0;

    // Reset branch predictor
    BPU = BranchPredictor();  // again, assuming default constructor

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
    load_program_memory("output.mc");

    // Clear log file.
    ofstream ofs("cycle_log.txt", ios::out | ios::trunc);
    if (!ofs)
        cerr << "Error clearing cycle_log.txt" << endl;
    ofs.close();

    // Clear stats file.
    ofstream ofs2("stats.txt", ios::out | ios::trunc);
    if (!ofs2)
        cerr << "Error clearing stats.txt" << endl;
    ofs2.close();

    cout << "Simulation reset." << endl;
}

pair<vector<int>, vector<vector<int>>> stepPipelinedSimulator(KnobManager& knobs){
    vector<int> instructionStage = {-1, -1, -1, -1, -1};
    vector<vector<int>> forwardingDetails;
    if (!(stopFetching && isPipelineEmpty())) {
        if (pipelineControl.flushIF) {
            PC = nextPC;
            flushPipeline();
            pipelineControl.flushIF = false;
        }

        totalCycles++;

        // Redirect cout to a buffer for this cycle
        std::ostringstream cycleOut;
        std::streambuf* origBuf = std::cout.rdbuf();
        std::cout.rdbuf(cycleOut.rdbuf());

        // flags for which stages to run
        bool runIF = true;
        bool runID = if_id.valid;
        bool runEX = id_ex.valid;
        bool runMEM = ex_mem.valid;
        bool runWB = mem_wb.valid;

        // Run stages, fetch always happens
        if (runWB) {
            instructionStage[4] = writeBack(knobs);
            if (mem_wb.Halt) stopFetching = true;
        }
        if (runMEM) instructionStage[3] = memoryAccess(knobs);
        if (runEX) instructionStage[2] = executeStage(knobs);
        if (runID){
            auto details = instructionDecode(knobs);
            instructionStage[1] = details.first;
            forwardingDetails = details.second;
        }
        if (runIF) instructionStage[0] = instructionFetch(knobs);

        // Restore cout
        std::cout.rdbuf(origBuf);

        // Add to output file
        std::ofstream logFile("cycle_log.txt", std::ios::out | std::ios::trunc);

        logFile << "=========== Cycle " << totalCycles << " ===========\n";
        logFile << cycleOut.str();

        if (knobs.printPipelineRegisters) {
            logFile << "\n--- Pipeline Snapshot ---\n";
            std::ostringstream snap;
            std::cout.rdbuf(snap.rdbuf());
            printPipelineRegisters(knobs);
            std::cout.rdbuf(origBuf);
            logFile << snap.str();
        }

        if (knobs.printRegistersEveryCycle) {
            std::ostringstream regSnap;
            std::cout.rdbuf(regSnap.rdbuf());
            printRegisterFile();
            std::cout.rdbuf(origBuf);
            logFile << "\n--- Register File ---\n" << regSnap.str();
        }

        if (knobs.traceBranchUnit) {
            logFile << "\n--- Branch Prediction Unit ---\n";
            BPU.printState(logFile);
        }

        logFile.close();
    }

    // Write stats to a separate file
    if (knobs.enableStats) {
        int totalInstructions = data_transfer_instructions + ALU_instructions + control_instructions;
        int totalStalls = dataStalls + controlStalls;

        std::ofstream stats("stats.txt", std::ios::out | std::ios::trunc);
        float cpi = totalInstructions ? totalCycles / (float)totalInstructions : 0.0f;
        stats << "Total Cycles: " << totalCycles << "\n";
        stats << "Instructions Executed: " << totalInstructions << "\n";
        stats << "CPI: " << cpi << "\n";
        stats << "Data Instructions: " << data_transfer_instructions << "\n";
        stats << "ALU Instructions: " << ALU_instructions << "\n";
        stats << "Control Instructions: " << control_instructions << "\n";
        stats << "Data Hazards: " << dataHazards << "\n";
        stats << "Control Hazards: " << controlHazards << "\n";
        stats << "Branch Mispredictions: " << branchMispredictions << "\n";
        stats << "Total Stalls: " << totalStalls << "\n";
        stats << "Stalls due to data hazards: " << dataStalls << "\n";
        stats << "Stalls due to control hazards: " << controlStalls << "\n";
        stats.close();
    }

    return {instructionStage, forwardingDetails};
}

pair<vector<int>, map<uint32_t, vector<pair<bool, bool>>>> runPipelinedSimulator(KnobManager& knobs, const set<int>& traceCounters) {
    vector<int> instructionStage = {-1, -1, -1, -1, -1};
    while (!(stopFetching && isPipelineEmpty())) {
        instructionStage = stepPipelinedSimulator(knobs).first;
        bool flag = true;
        for(int pc: instructionStage){
            if ((traceCounters.find(pc) != traceCounters.end())
                || (traceCounters.find(pc-0x10000000) != traceCounters.end())
                || (traceCounters.find(pc-0x20000000) != traceCounters.end())){
                flag = false;
                break;
            }
        }
        if (!flag) break;
    }

    std::cout << "\n✅ Simulation complete. Output written to 'cycle_log.txt' and 'stats.txt'\n";

    return {instructionStage, BPU.PHT_table};
}

int instructionFetch(const KnobManager& knobs) {
    // 1. If we're stalling this cycle, return without fetching
    if (pipelineControl.stallIFCount > 0) {
        pipelineControl.stallIFCount--;
        if (if_id.valid) return 0x10000000+if_id.pc;
        return -1;
    }

    // 2. Don't do anything if already HALTed
    if (stopFetching) {
        return -1;
    }

    // 3. Check if PC is outside memory (end of program)
    if (MEM.find(PC) == MEM.end() || MEM[PC] == 0) {
        stopFetching = true;
        if_id.valid = false;
        return -1;
    }

    // 4. Fetch 4-byte instruction (little-endian)
    uint32_t fetchedInstruction = 0;
    for (int i = 3; i >= 0; i--) {
        fetchedInstruction <<= 8;
        fetchedInstruction |= MEM[PC + i];
    }

    // 5. Store in IF/ID register
    if_id.instruction = fetchedInstruction;
    if_id.pc = PC;
    if_id.valid = true;

    if (knobs.traceInstructionIndex == PC) {
        knob5File << "Cycle " << totalCycles << ": Fetched instruction at PC: " << to_hex(PC) << "\n";
        knob5File << "  Instruction: " << to_hex(fetchedInstruction) << "\n";
    }

    // 6. Branch prediction (only if opcode is branch/jump)
    if (knobs.traceBranchUnit){
        bool predictedTaken = BPU.predict(PC);
        uint32_t predictedTarget = BPU.getTarget(PC);

        if (predictedTaken) {
            PC = predictedTarget;
        } else {
            PC += 4;
        }
    }
    else{
        PC += 4;
    }

    // 7. Check again if PC now points to something invalid
    if (MEM.find(PC) == MEM.end()) {
        stopFetching = true;
    }

    if (pipelineControl.flushIF) return 0x20000000+if_id.pc;
    return if_id.pc;
}

pair<int, vector<vector<int>>> instructionDecode(const KnobManager& knobs) {
    if (!if_id.valid) {
        id_ex=ID_EX_Register{};
        id_ex.valid = false;
        return {-1, {}};
    }

    uint32_t instruction = if_id.instruction;
    uint8_t opcode = instruction & 0x7F;
    uint8_t rd = (instruction >> 7) & 0x1F;
    uint8_t funct3 = (instruction >> 12) & 0x7;
    uint8_t rs1 = (instruction >> 15) & 0x1F;
    uint8_t rs2 = (instruction >> 20) & 0x1F;
    uint8_t funct7 = (instruction >> 25) & 0x7F;

    // Immediate
    uint32_t imm = 0;
    switch (opcode) {
    case 0x03: case 0x13: case 0x67:
        imm = static_cast<int32_t>(instruction) >> 20;
        rs2 = 0;
        break;
    case 0x23:
        imm = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);
        imm = static_cast<int32_t>(imm << 20) >> 20;
        break;
    case 0x63:
        imm = (((instruction >> 31) & 0x1) << 12) |
              (((instruction >> 7) & 0x1) << 11) |
              (((instruction >> 25) & 0x3F) << 5) |
              (((instruction >> 8) & 0xF) << 1);
        imm = static_cast<int32_t>(imm << 19) >> 19;
        break;
    case 0x37: case 0x17:
        imm = instruction & 0xFFFFF000;
        rs1 = 0;
        rs2 = 0;
        break;
    case 0x6F:
        rs1 = 0;
        rs2 = 0;
        imm = (((instruction >> 31) & 0x1) << 20) |
              (((instruction >> 21) & 0x3FF) << 1) |
              (((instruction >> 20) & 0x1) << 11) |
              (((instruction >> 12) & 0xFF) << 12);
        imm = static_cast<int32_t>(imm << 11) >> 11;
        break;
    }

    // RAW hazard check
    int stallRS1 = checkRAWStalls(rs1, knobs);
    int stallRS2 = checkRAWStalls(rs2, knobs);

    if (pipelineControl.stallIFCount > 0) return {-1, {}};

    //check data hazards for rs1 and rs2
    int maxStalls = std::max(stallRS1, stallRS2);
    if (maxStalls > 0) {
        pipelineControl.stallIFCount = maxStalls;
        dataStalls += maxStalls;

        id_ex.valid = false;
        return {-1, {}};
    }

    // Use Control Unit to get control signals
    ControlSignals ctrl = getControlSignals(opcode, funct3, funct7);

    // Populate ID/EX pipeline register
    id_ex.pc = if_id.pc;
    id_ex.instruction = instruction;
    id_ex.rs1 = rs1;
    id_ex.rs2 = rs2;
    id_ex.rd = rd;
    id_ex.funct3 = funct3;
    id_ex.funct7 = funct7;
    id_ex.opcode = opcode;
    id_ex.imm = imm;

    // Forwarded values
    uint32_t rs1Val = R[rs1], rs2Val = R[rs2];

    vector<vector<int>> forwardingDetails;

    if (knobs.dataForwarding) {
        bool forwarded_rs1 = false, forwarded_rs2 = false;

        if (ex_mem.valid && ex_mem.RegWrite && ex_mem.rd != 0) {
            if (ex_mem.rd == rs1) {
                rs1Val = ex_mem.aluResult;
                forwarded_rs1 = true;
                forwardingDetails.push_back({ex_mem.pc/4, 3, id_ex.pc/4, 2});
            }
            if (ex_mem.rd == rs2) {
                rs2Val = ex_mem.aluResult;
                forwarded_rs2 = true;
                forwardingDetails.push_back({ex_mem.pc/4, 3, id_ex.pc/4, 2});
            }
        }
        if (mem_wb.valid && mem_wb.RegWrite && mem_wb.rd != 0) {
            if (!forwarded_rs1 && mem_wb.rd == rs1) {
                rs1Val = mem_wb.MemToReg ? mem_wb.memData : mem_wb.aluResult;
                forwardingDetails.push_back({mem_wb.pc/4, 4, id_ex.pc/4, 2});
            }
            if (!forwarded_rs2 && mem_wb.rd == rs2) {
                rs2Val = mem_wb.MemToReg ? mem_wb.memData : mem_wb.aluResult;
                forwardingDetails.push_back({mem_wb.pc/4, 4, id_ex.pc/4, 2});
            }
        }
    }

    //knob5 logging
    if (knobs.traceInstructionIndex != -1 && if_id.pc == (uint32_t)knobs.traceInstructionIndex) {
        knob5File << "Cycle " << totalCycles << ": [ID Stage]\n";
        knob5File << "ID/EX Register:\n";
        knob5File << "  PC: " << to_hex(id_ex.pc) << "\n";
        knob5File << "  Instruction: " << to_hex(id_ex.instruction) << "\n";
        knob5File << "  rs1: x" << +id_ex.rs1 << " = " << to_hex(id_ex.rs1Val) << "\n";
        knob5File << "  rs2: x" << +id_ex.rs2 << " = " << to_hex(id_ex.rs2Val) << "\n";
        knob5File << "  rd: x" << +id_ex.rd << "\n";
        knob5File << "  imm: " << to_hex(id_ex.imm) << "\n";
        knob5File << "  Control Signals -> ALUSrc: " << id_ex.ALUSrc
                  << ", RegWrite: " << id_ex.RegWrite
                  << ", MemRead: " << id_ex.MemRead
                  << ", MemWrite: " << id_ex.MemWrite
                  << ", MemToReg: " << id_ex.MemToReg
                  << ", Branch: " << id_ex.Branch
                  << ", Jump: " << id_ex.Jump
                  << ", Halt: " << id_ex.Halt << "\n\n";
    }

    // Set register values in ID/EX register
    id_ex.rs1Val = rs1Val;
    id_ex.rs2Val = rs2Val;

    // Set control signals using control unit
    id_ex.RegWrite = ctrl.RegWrite;
    id_ex.ALUSrc   = ctrl.ALUSrc;
    id_ex.MemRead  = ctrl.MemRead;
    id_ex.MemWrite = ctrl.MemWrite;
    id_ex.MemToReg = ctrl.MemToReg;
    id_ex.Branch   = ctrl.Branch;
    id_ex.Jump     = ctrl.Jump;
    id_ex.Halt     = ctrl.Halt;
    if (ctrl.Halt) {
        stopFetching = true;
    }

    id_ex.valid    = true;

    if_id.valid = false;
    id_ex.aluOp = ctrl.aluOp; // New: ALU control signal
    if_id = IF_ID_Register{}; // clears IF/ID

    if (pipelineControl.flushIF) return {0x20000000+id_ex.pc, forwardingDetails};
    return {id_ex.pc, forwardingDetails};
}

int executeStage(const KnobManager& knobs) {
    if (!id_ex.valid) {
        ex_mem=EX_MEM_Register{};
        ex_mem.valid = false;
        return -1;
    }

    ex_mem.valid = true;
    ex_mem.pc = id_ex.pc;
    ex_mem.rd = id_ex.rd;
    ex_mem.rs2Val = id_ex.rs2Val;
    ex_mem.funct3 = id_ex.funct3;


    // Transfer control signals
    ex_mem.RegWrite = id_ex.RegWrite;
    ex_mem.MemRead  = id_ex.MemRead;
    ex_mem.MemWrite = id_ex.MemWrite;
    ex_mem.MemToReg = id_ex.MemToReg;
    ex_mem.Branch   = id_ex.Branch;
    ex_mem.Jump     = id_ex.Jump;
    ex_mem.Halt     = id_ex.Halt;

    uint32_t operand1 = id_ex.rs1Val;
    uint32_t operand2 = id_ex.ALUSrc ? id_ex.imm : id_ex.rs2Val;

    // Compute ALU result
    switch (id_ex.aluOp) {
    case ALU_ADD:  ex_mem.aluResult = operand1 + operand2; break;
    case ALU_SUB:  ex_mem.aluResult = operand1 - operand2; break;
    case ALU_AND:  ex_mem.aluResult = operand1 & operand2; break;
    case ALU_OR:   ex_mem.aluResult = operand1 | operand2; break;
    case ALU_XOR:  ex_mem.aluResult = operand1 ^ operand2; break;
    case ALU_SLL:  ex_mem.aluResult = operand1 << (operand2 & 0x1F); break;
    case ALU_SRL:  ex_mem.aluResult = operand1 >> (operand2 & 0x1F); break;
    case ALU_SRA:  ex_mem.aluResult = static_cast<int32_t>(operand1) >> (operand2 & 0x1F); break;
    case ALU_SLT:  ex_mem.aluResult = ((int32_t)operand1 < (int32_t)operand2) ? 1 : 0; break;
    case ALU_SLTU: ex_mem.aluResult = (operand1 < operand2) ? 1 : 0; break;
    case ALU_PASS: ex_mem.aluResult = id_ex.opcode == 0x37 ? id_ex.imm : operand1 + id_ex.imm; break; // LUI / AUIPC
    default:       ex_mem.aluResult = 0; break;
    }

    // ----------------------
    // Branch Prediction
    // ----------------------
    if (id_ex.Branch) {
        bool branchTaken = false;
        switch (id_ex.funct3) {
        case 0x0: branchTaken = (id_ex.rs1Val == id_ex.rs2Val); break; // BEQ
        case 0x1: branchTaken = (id_ex.rs1Val != id_ex.rs2Val); break; // BNE
        case 0x4: branchTaken = ((int32_t)id_ex.rs1Val < (int32_t)id_ex.rs2Val); break; // BLT
        case 0x5: branchTaken = ((int32_t)id_ex.rs1Val >= (int32_t)id_ex.rs2Val); break; // BGE
        case 0x6: branchTaken = (id_ex.rs1Val < id_ex.rs2Val); break; // BLTU
        case 0x7: branchTaken = (id_ex.rs1Val >= id_ex.rs2Val); break; // BGEU
        }

        bool predicted = false;
        if (knobs.traceBranchUnit) predicted = BPU.predict(id_ex.pc);
        uint32_t actualTarget = id_ex.pc + id_ex.imm;

        if (branchTaken != predicted) {
            pipelineControl.flushIF = true;
            nextPC = branchTaken ? actualTarget : id_ex.pc + 4;
            branchMispredictions++;
        }

        BPU.update(knobs.traceBranchUnit, id_ex.pc, branchTaken, actualTarget);
    }

    // ----------------------
    // Jump Handling
    // ----------------------
    if (id_ex.Jump) {
        ex_mem.aluResult = (id_ex.opcode == 0x6F) ? id_ex.pc + 4 : (id_ex.rs1Val + id_ex.imm) & ~1;
        nextPC = (id_ex.opcode == 0x6F) ? id_ex.pc + id_ex.imm : (id_ex.rs1Val + id_ex.imm) & ~1;
        pipelineControl.flushIF = true;
    }

    // Count instructions
    if (id_ex.opcode == 0x33 || // "0110011" → R-type ALU: add, sub, etc.
        id_ex.opcode == 0x13)
    { // "0010011" → I-type ALU: addi, andi, etc.
        ALU_instructions++;
    }
    else if (id_ex.opcode == 0x03 || // "0000011" → I-type loads: lb, lw, etc.
             id_ex.opcode == 0x23 || // "0100011" → S-type stores: sb, sw, etc.
             id_ex.opcode == 0x37 || // "0110111" → U-type: lui
             id_ex.opcode == 0x17)
    { // "0010111" → U-type: auipc
        data_transfer_instructions++;
    }
    else if (id_ex.opcode == 0x63 || // "1100011" → branches: beq, etc.
             id_ex.opcode == 0x6F || // "1101111" → jal
             id_ex.opcode == 0x67)
    { // "1100111" → jalr
        control_instructions++;
    }

    //knob5 logging
    if (knobs.traceInstructionIndex != -1 && id_ex.pc == (uint32_t)knobs.traceInstructionIndex) {
        knob5File << "Cycle " << totalCycles << ": [EX Stage]\n";
        knob5File << "EX/MEM Register:\n";
        knob5File << "  PC: " << to_hex(ex_mem.pc) << "\n";
        knob5File << "  ALUResult: " << to_hex(ex_mem.aluResult) << "\n";
        knob5File << "  rs2Val: " << to_hex(ex_mem.rs2Val) << "\n";
        knob5File << "  rd: x" << +ex_mem.rd << "\n";
        knob5File << "  Control Signals -> RegWrite: " << ex_mem.RegWrite
                  << ", MemRead: " << ex_mem.MemRead
                  << ", MemWrite: " << ex_mem.MemWrite
                  << ", MemToReg: " << ex_mem.MemToReg
                  << ", Branch: " << ex_mem.Branch
                  << ", Jump: " << ex_mem.Jump
                  << ", Halt: " << ex_mem.Halt << "\n\n";
    }

    id_ex = ID_EX_Register{}; // clears ID/EX
    id_ex.valid = false; // clear ID/EX register

    return ex_mem.pc;
}

// ------------------------
// Memory Access Stage
// ------------------------
int memoryAccess(const KnobManager& knobs) {
    if (!ex_mem.valid) {
        mem_wb=MEM_WB_Register{};
        mem_wb.valid = false;
        return -1;
    }

    mem_wb.valid = true;
    mem_wb.pc = ex_mem.pc;
    mem_wb.rd = ex_mem.rd;
    mem_wb.aluResult = ex_mem.aluResult;

    // Control signals
    mem_wb.RegWrite = ex_mem.RegWrite;
    mem_wb.MemToReg = ex_mem.MemToReg;
    mem_wb.Halt = ex_mem.Halt;

    if (ex_mem.MemRead) {
        uint32_t addr = ex_mem.aluResult;
        uint32_t data = 0;
        switch (ex_mem.funct3) {
        case 0x0: // LB
            data = static_cast<int32_t>(static_cast<int8_t>(MEM[addr]));
            break;
        case 0x1: // LH
            data = static_cast<int32_t>(static_cast<int16_t>(MEM[addr] | (MEM[addr + 1] << 8)));
            break;
        case 0x2: // LW
            for (int i = 0; i < 4; i++) {
                data |= (MEM[addr + i] << (8 * i));
            }
            break;
        case 0x4: // LBU
            data = MEM[addr];
            break;
        case 0x5: // LHU
            data = MEM[addr] | (MEM[addr + 1] << 8);
            break;
        }
        mem_wb.memData = data;
    }
    else if (ex_mem.MemWrite) {
        uint32_t addr = ex_mem.aluResult;
        uint32_t val = ex_mem.rs2Val;
        switch (ex_mem.funct3) {
        case 0x0: // SB
            MEM[addr] = val & 0xFF;
            break;
        case 0x1: // SH
            MEM[addr] = val & 0xFF;
            MEM[addr + 1] = (val >> 8) & 0xFF;
            break;
        case 0x2: // SW
            for (int i = 0; i < 4; i++) {
                MEM[addr + i] = (val >> (8 * i)) & 0xFF;
            }
            break;
        }
    }

    //knob5 logging
    if (knobs.traceInstructionIndex != -1 && ex_mem.pc == (uint32_t)knobs.traceInstructionIndex) {
        knob5File << "Cycle " << totalCycles << ": [MEM Stage]\n";
        knob5File << "MEM/WB Register:\n";
        knob5File << "  PC: " << to_hex(mem_wb.pc) << "\n";
        knob5File << "  memData: " << to_hex(mem_wb.memData) << "\n";
        knob5File << "  ALUResult: " << to_hex(mem_wb.aluResult) << "\n";
        knob5File << "  rd: x" << +mem_wb.rd << "\n";
        knob5File << "  Control Signals -> RegWrite: " << mem_wb.RegWrite
                  << ", MemToReg: " << mem_wb.MemToReg
                  << ", Halt: " << mem_wb.Halt << "\n\n";
    }

    ex_mem.valid = false; // clear EX/MEM register
    ex_mem = EX_MEM_Register{}; // clears EX/MEM

    return mem_wb.pc;
}

// ------------------------
// Write Back Stage
// ------------------------
int writeBack(const KnobManager& knobs) {
    if (!mem_wb.valid) return -1;

    if (mem_wb.RegWrite && mem_wb.rd != 0) {
        uint32_t result = mem_wb.MemToReg ? mem_wb.memData : mem_wb.aluResult;
        R[mem_wb.rd] = result;
    }

    if (mem_wb.Halt) {
        stopFetching = true;
    }

    int pcToReturn = mem_wb.pc;
    mem_wb.valid = false; // clear MEM/WB register
    mem_wb = MEM_WB_Register{}; // clears MEM/WB

    if (knobs.traceInstructionIndex != -1 && mem_wb.pc == (uint32_t)knobs.traceInstructionIndex) {
        knob5File << "Cycle " << totalCycles << ": [WB Stage]\n";
        knob5File << "Writeback:\n";
        knob5File << "  Destination Register (x" << +mem_wb.rd << ") ";
        if (mem_wb.RegWrite && mem_wb.rd != 0) {
            uint32_t result = mem_wb.MemToReg ? mem_wb.memData : mem_wb.aluResult;
            knob5File << "written with value: " << to_hex(result) << "\n\n";
        } else {
            knob5File << "not written (RegWrite = 0)\n\n";
        }
    }

    return pcToReturn;
}

void printPipelineRegisters(const KnobManager& knobs) {
    cout << "-------- Pipeline Register Snapshot --------" << endl;

    cout << "[IF/ID] ";
    if (if_id.valid) {
        cout << "PC: " << to_hex(if_id.pc)
        << ", Instruction: " << to_hex(if_id.instruction) << endl;
    } else {
        cout << "Invalid" << endl;
    }

    cout << "[ID/EX] ";
    if (id_ex.valid) {
        cout << "PC: " << to_hex(id_ex.pc)
        << ", rs1: x" << dec << (int)id_ex.rs1 << " = " << to_hex(id_ex.rs1Val)
        << ", rs2: x" << (int)id_ex.rs2 << " = " << to_hex(id_ex.rs2Val)
        << ", rd: x" << (int)id_ex.rd
        << ", imm: " << to_hex(id_ex.imm)
        << ", ALUSrc: " << id_ex.ALUSrc << ", MemRead: " << id_ex.MemRead
        << ", MemWrite: " << id_ex.MemWrite << ", MemToReg: " << id_ex.MemToReg
        << ", RegWrite: " << id_ex.RegWrite << ", Branch: " << id_ex.Branch
        << ", Jump: " << id_ex.Jump << ", Halt: " << id_ex.Halt << endl;
    } else {
        cout << "Invalid" << endl;
    }

    cout << "[EX/MEM] ";
    if (ex_mem.valid) {
        cout << "PC: " << to_hex(ex_mem.pc)
        << ", ALUResult: " << to_hex(ex_mem.aluResult)
        << ", rs2Val: " << to_hex(ex_mem.rs2Val)
        << ", rd: x" << (int)ex_mem.rd
        << ", MemRead: " << ex_mem.MemRead << ", MemWrite: " << ex_mem.MemWrite
        << ", MemToReg: " << ex_mem.MemToReg << ", RegWrite: " << ex_mem.RegWrite
        << ", Branch: " << ex_mem.Branch << ", Jump: " << ex_mem.Jump << ", Halt: " << ex_mem.Halt << endl;
    } else {
        cout << "Invalid" << endl;
    }

    cout << "[MEM/WB] ";
    if (mem_wb.valid) {
        cout << "PC: " << to_hex(mem_wb.pc)
        << ", memData: " << to_hex(mem_wb.memData)
        << ", ALUResult: " << to_hex(mem_wb.aluResult)
        << ", rd: x" << (int)mem_wb.rd
        << ", MemToReg: " << mem_wb.MemToReg
        << ", RegWrite: " << mem_wb.RegWrite
        << ", Halt: " << mem_wb.Halt << endl;
    } else {
        cout << "Invalid" << endl;
    }

    cout << "-------------------------------------------" << endl;
}


void flushPipeline() {
    int is_hazard = 0;
    int num_stalls = 0;
    stopFetching = false;
    if (if_id.valid){
        if_id.valid = false;
        is_hazard = 1;
        num_stalls++;
    }
    if (id_ex.valid){
        id_ex.valid = false;
        is_hazard = 1;
        num_stalls++;
    }
    controlHazards += is_hazard;
    controlStalls += num_stalls;
}

bool isPipelineEmpty() {
    return !if_id.valid && !id_ex.valid && !ex_mem.valid && !mem_wb.valid;
}

// ==============================
// Main function to run simulator
// ==============================

void loadMemoryFromMCFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Cannot open file: " << filename << endl;
        exit(1);
    }

    string line;
    while (getline(infile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string addr_str, val_str;
        ss >> addr_str >> val_str;

        // Remove "0x" and convert
        uint32_t address = stoi(addr_str, nullptr, 16);
        uint32_t instruction = stoi(val_str, nullptr, 16);

        for (int i = 0; i < 4; i++) {
            MEM[address + i] = (instruction >> (8 * i)) & 0xFF;
        }
    }

    infile.close();
}
}
