#include ".\include\file_parser.hpp"
#include ".\include\print_utility.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <algorithm>


void firstPass(
    const std::vector<std::vector<std::string>>& instructions, 
    const std::unordered_map<std::string, std::pair<std::string, int>>& opcodeTable, 
    std::unordered_map<std::string, std::pair<int, bool>>& symbolTable,
    std::unordered_set<std::string> assemblerDirective)
    {
    int locationCounter = 0;
    
    // Open the intermediate file for writing
    std::ofstream intermediateFile("intermediate.txt");
    if (!intermediateFile.is_open()) {
        std::cerr << "Error: Unable to create intermediate file" << std::endl;
        return;
    }

    for (const auto& instruction : instructions) {
        // Write location counter and instruction to the intermediate file
        intermediateFile << std::setw(6) << std::hex << locationCounter << " "; // Set width to 4 characters for hexadecimal output
        if(!instruction[0].empty()){
            intermediateFile << std::setw(7) << instruction[0];
        }
        else{
            intermediateFile << "       ";
        }
        //opcode check
        if(instruction[1][0] == '+'){
            // Write '+' character for instructions starting with '+'
            intermediateFile << std::setw(7) << instruction[1];
            // Check opcode table without '+'
            if (opcodeTable.find(instruction[1].substr(1)) != opcodeTable.end()) {
                locationCounter += 4;
            }
            else{
                std::cerr << "Error: Opcode not found for mnemonic '" << instruction[1] << "'" << std::endl;
            }
                
            
        }
        else{
            intermediateFile << std::setw(7) << instruction[1];
            // Check opcode table normally
            if (opcodeTable.find(instruction[1]) != opcodeTable.end()) {
                locationCounter += opcodeTable.at(instruction[1]).second;
            } 
            else if(assemblerDirective.find(instruction[1]) != assemblerDirective.end()){
                // assembler directive
                if(*assemblerDirective.find(instruction[1]) == "BYTE"){
                    // according to length of third colm
                    if(instruction[2][0] == 'X'){
                        // hex values
                        locationCounter += (instruction[2].length()-3)/2; //POTENTIAL ERROR
                    }
                    else if(instruction[2][0] == 'C'){
                        // string values
                        locationCounter += (instruction[2].length()-3);
                    }
                }
                else if(*assemblerDirective.find(instruction[1]) == "WORD"){
                    locationCounter += 3;
                }
                else if(*assemblerDirective.find(instruction[1]) == "RESW"){
                    locationCounter += 3*stoi(instruction[2]);
                }
                else if(*assemblerDirective.find(instruction[1]) == "RESB"){
                    locationCounter += stoi(instruction[2]);
                }
            }
            else {
                std::cerr << "Error: Opcode not found for mnemonic '" << instruction[1] << "'" << std::endl;
            }
        }
        intermediateFile << " ";
        intermediateFile << instruction[2]; 
        
        intermediateFile << std::endl;

        // Check if the instruction has a label
        if (!instruction[0].empty()) {
            // Check if the label is already defined
            if (symbolTable.find(instruction[0]) != symbolTable.end()) {
                // Redefinition of symbol, set error flag to true
                symbolTable[instruction[0]].second = true;
                std::cerr << "Error: Redefinition of symbol '" << instruction[0] << "'" << std::endl;
            } else {
                // Add the label to the symbol table with the current location counter
                symbolTable[instruction[0]] = std::make_pair(locationCounter, false);
            }
        }
    }

    // Close the intermediate file
    intermediateFile.close();
    storeSymbolTable("symboltable.txt",symbolTable);
}

int main(){
    std::unordered_map<std::string, std::pair<std::string,int>> OpcodeTable;
    std::unordered_map<std::string, std::pair<int,bool>> SymbolTable;
    std::unordered_set<std::string> assemblerDirective;
    OpcodeTable = importOpcodeTable("opcodes.txt");
    assemblerDirective = importAssemblerDirectives("directives.txt");
    // for(auto &i : assemblerDirective){
    //     std::cout << i << " ";
    // }
    std::string filename;
    std::cout<<"input file name to read : ";
    std::cin>>filename; 
    std::vector<std::vector<std::string>> file = readInputFile(filename);
    printAssembly(file);
    firstPass(file, OpcodeTable, SymbolTable, assemblerDirective);
    printIntermediateFile("intermediate.txt");
}