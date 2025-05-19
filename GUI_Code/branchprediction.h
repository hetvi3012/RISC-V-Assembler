// branch_predictor.h
#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <iomanip>

using namespace std;

inline std::string to_hex(unsigned int value) {
    std::stringstream stream;
    stream << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
    return stream.str();
}

// ----------------------------
// Branch Prediction Unit (1-bit Predictor)
// ----------------------------
class BranchPredictor {
private:
    unordered_map<uint32_t, bool> PHT;  // PC -> 1-bit prediction state (true = taken, false = not taken)
    unordered_map<uint32_t, uint32_t> BTB; // PC -> target address

public:
    map<uint32_t, vector<pair<bool, bool>>> PHT_table; // predicted, actual

    // Predict whether branch at 'pc' will be taken (1-bit predictor)
    bool predict(uint32_t pc) {
        // If PC not seen before, default prediction is taken (false)
        if (PHT.find(pc) == PHT.end()) return false;
        return PHT[pc];
    }

    // Get the predicted target address from BTB if exists; otherwise fallback to PC+4
    uint32_t getTarget(uint32_t pc) {
        if (BTB.find(pc) != BTB.end()) {
            return BTB[pc];
        }
        return pc + 4; // fallback default
    }

    // Update prediction outcome and BTB based on actual branch result
    void update(bool enabled, uint32_t pc, bool actualTaken, uint32_t actualTarget) {
        if (enabled){
            bool predicted = predict(pc);

            // Update prediction state
            if (actualTaken) {
                PHT[pc] = true;
                BTB[pc] = actualTarget; // Update target if taken
            } else {
                PHT[pc] = false;
                BTB.erase(pc); // Remove target if not taken
            }

            PHT_table[pc].push_back({predicted, actualTaken});
        }
        else{
            PHT_table[pc].push_back({false, actualTaken});
        }
    }

    // Debug output of the predictor state
    void printState(std::ostream& out = std::cout) {
        out << "======== Branch Prediction Unit ========\n";
        out << "BTB (Branch Target Buffer):\n";
        for (const auto& entry : BTB) {
            out << "  PC: " << to_hex(entry.first)
            << " → Target: " << to_hex(entry.second) << "\n";
        }

        out << "\n1-bit Prediction State (Taken = 1, Not Taken = 0):\n";
        for (const auto& state : PHT) {
            out << "  PC: " << to_hex(state.first)
            << " → Predicted: " << (state.second ? "Taken" : "Not Taken") << "\n";
        }
        out << "========================================\n";
    }

};

// Global BPU instance
extern BranchPredictor BPU;

#endif // BRANCH_PREDICTOR_H
