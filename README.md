# RISC-V Simulator & Assembler

This project encompasses a **32-bit RISC-V assembler (Phase 1)** and two modes of simulation:
1. **Non-pipelined (Phase 2)** — straight-line execution without pipelining.
2. **Pipelined (Phase 3)** — five-stage in-order pipeline with hazard control, data forwarding, one-bit branch prediction, and stall support.

A CLI and a GUI front-end let you explore both modes. The GUI also visualizes the Pattern History Table (PHT), Branch Target Buffer (BTB), cycle logs, and execution stats.

---

## Table of Contents

- [Project Overview](#project-overview)
- [Features & Knobs](#features--knobs)
- [Input & Output Formats](#input--output-formats)
- [Instruction Set Support](#instruction-set-support)
- [Project Structure](#project-structure)
- [Building & Running](#building--running)
- [Implementation Details](#implementation-details)
- [Statistics (stats.txt)](#statistics-statstxt)
<<<<<<< HEAD
=======
>>>>>>> b16fed7765b52712c6f2fe68476f264cb2d169c3
- [Screenshots](#screenshots)

---

## Project Overview

1. **Assembler (Phase 1)**
   - Parses `input.asm`, builds symbol table, encodes to `output.mc`.

2. **Simulator (Phase 2 & 3)**
   - **Phase 2**: Non-pipelined, executes each instruction end-to-end per cycle.
   - **Phase 3**: Five-stage pipeline (IF, ID, EX, MEM, WB) with:
     - **Hazard Detection & Stalling**
     - **Data Forwarding**
     - **One-Bit Branch Predictor (PHT & BTB)**
   - CLI outputs:
     - `cycle_log.txt` (detailed cycle-by-cycle events)
     - `stats.txt` (aggregate performance metrics)
     - `reg_out.mem`, `data_out.mem` (final state dumps)

3. **GUI Mode**
   - Toggle knobs for pipelining, forwarding, branch prediction.
   - Visual PHT table & BTB entries.
   - Live cycle log and stats display.

---

## Features & Knobs

### Global Knobs (CLI & GUI)
1. **Enable Pipelining** — switch between non-pipelined and pipelined modes.
2. **Enable Data Forwarding** — bypass ALU results or insert stalls when disabled.
3. **Trace Registers** — print entire register file at end of each cycle.
4. **Trace Pipeline Registers** — dump pipeline-latch contents per cycle.
5. **Instruction Trace** — trace pipeline registers for a specific instruction index.
6. **Branch Predictor Trace** — dump PC, PHT entries, and BTB state each cycle.

---

## Input & Output Formats

| File             | Description                                    |
|------------------|------------------------------------------------|
| `input.asm`      | Assembly source                                |
| `output.mc`      | Binary machine code                            |
| `cycle_log.txt`  | Per-cycle simulation events                    |
| `stats.txt`      | Summary simulation statistics                  |
| `data_out.mem`   | Final data memory snapshot                     |
| `reg_out.mem`    | Final register file snapshot                   |

---

## Instruction Set Support

Supports RV32I base instructions (R, I, S, B, U, J formats) and directives: `.text`, `.data`, `.byte`, `.half`, `.word`, `.dword`, `.asciz`.

---

## Project Structure

```
RISC-V/
├── .vscode/
├── GUI_Code/              # GUI source
├── GUI_Exe/               # GUI executable (RiscVSim.exe)
├── assembler.cpp          # Two-pass assembler
├── assembler.exe
├── pipeline_simulator.cpp # Core simulator (Phase 2 & 3)
├── pipeline_simulator.exe
├── branchprediction.h     # One-bit predictor (PHT & BTB)
├── control_unit.h         # Hazard detection & stall logic
├── knobs.h                # Knob definitions & logging
├── pipeline_registers.h   # Pipeline latch structures
├── stats.txt              # Aggregate stats output
├── cycle_log.txt          # Detailed cycle logs
├── data_out.mem
├── reg_out.mem
├── input.asm
└── README.md
```

---

## Building & Running

### CLI
```sh
# Compile
g++ -o assembler assembler.cpp
g++ -o pipeline_simulator pipeline_simulator.cpp

# Run Assembler
./assembler

# Run Simulator (Phase 2 or 3 according to knob)
./pipeline_simulator
```

### GUI
```sh
cd GUI_Exe
./RiscVSim.exe
```
Use GUI toggles for knobs, view PHT, BTB, stats, and cycle log live.

---

## Implementation Details

### Assembler (Two-Pass)
- **Pass 1**: Build symbol table for labels and data allocations.
- **Pass 2**: Encode instructions, resolve immediates and labels.

### Simulator
- **Non-Pipelined**: Phase 2 model.
  - **IF:** Fetches the instruction from memory.
  - **ID:** Decodes the instruction and reads the necessary registers.
  - **EX:** Executes the arithmetic/logical operation.
  - **MEM:** Accesses memory for load and store instructions.
  - **WB:** Writes results back to the register file.

- **Pipelined**:
  - **Pipeline Registers**: Structures in `pipeline_registers.h` carry control/data signals.
  - **Hazard Detection Unit**: Inserts stalls on RAW/control hazards.
  - **Forwarding Unit**: Routes EX/MEM or MEM/WB results to reduce stalls.
  - **Branch Predictor**: 1-bit per branch PC with PHT & BTB arrays.

---

## Statistics (stats.txt)

At simulation end, prints:
1. Total cycles
2. Total instructions executed
3. CPI
4. Data-transfer (load/store) count
5. ALU instruction count
6. Control instruction count
7. Total stalls/bubbles
8. Data hazards encountered
9. Control hazards encountered
10. Branch mispredictions
11. Stalls due to data hazards
12. Stalls due to control hazards



---

## Screenshots

*To be added: GUI pipeline view, PHT/BTB table, cycle logs, forwarding highlights.*
=======
## Screenshots

*To be added: GUI pipeline view, PHT/BTB table, cycle logs, forwarding highlights.*
>>>>>>> b16fed7765b52712c6f2fe68476f264cb2d169c3
