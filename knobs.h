#ifndef KNOBS_H
#define KNOBS_H

#include <bits/stdc++.h>
using namespace std;

struct KnobManager {
    // Main knobs
    bool pipelining = false;               // Knob1
    bool dataForwarding = false;           // Knob2
    bool printRegistersEveryCycle = false; // Knob3
    bool printPipelineRegisters = true;   // Knob4
    int  traceInstructionIndex = -1;       // Knob5
    bool traceBranchUnit = false;          // Knob6
    bool enableStats = true;

    void readKnobInputs();
};

inline void KnobManager::readKnobInputs() {
    char choice;

    cout << "========== Simulator Knob Configuration ==========\n";

    // Ask for all knobs, always
    cout << "Knob1: Enable pipelining? (y/n): ";
    cin >> choice;
    pipelining = (choice == 'y' || choice == 'Y');

    cout << "Knob2: Enable data forwarding? (y/n): ";
    cin >> choice;
    dataForwarding = (choice == 'y' || choice == 'Y');

    cout << "Knob3: Print register file every cycle? (y/n): ";
    cin >> choice;
    printRegistersEveryCycle = (choice == 'y' || choice == 'Y');

    cout << "Knob4: Print pipeline registers every cycle? (y/n): ";
    cin >> choice;
    printPipelineRegisters = (choice == 'y' || choice == 'Y');

    // Knob5
    cout << "Knob5: Trace a specific instruction? (enter index or 0xPC address or -1 to disable): ";
    string traceInput;
    cin >> traceInput;
    try {
        if (traceInput.size() >= 2 && (traceInput.substr(0, 2) == "0x" || traceInput.substr(0, 2) == "0X")) {
            traceInstructionIndex = stoi(traceInput, nullptr, 16);
        } else {
            traceInstructionIndex = stoi(traceInput);
        }
    } catch (const exception& e) {
        cerr << "Invalid input for Knob5. Defaulting to -1.\n";
        traceInstructionIndex = -1;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Knob6: Enable Branch Prediction Unit trace? (y/n): ";
    cin >> choice;
    traceBranchUnit = (choice == 'y' || choice == 'Y');

    // If pipelining is ON, ask for the additional features (not tied to any knob)
    if (pipelining) {
        cout << "\nPipelining ENABLED. Additional pipelined features:\n";

        cout << "Enable full stats collection? (y/n): ";
        cin >> choice;
        enableStats = (choice == 'y' || choice == 'Y');
    }
    else {
        cout << "Pipelining DISABLED. Running in non-pipelined mode.\n";
    }

    cout << "==================================================\n\n";
}

#endif
