// assembler.h
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
using std::string;

// This function reads an assembly file and writes machine code.
void parseAssembly(const string& inputFile, const string& outputFile);

#endif // ASSEMBLER_H
