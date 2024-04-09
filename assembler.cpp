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
#include <bitset>

std::unordered_map<std::string, std::pair<std::string,int>> OpcodeTable;
std::unordered_map<std::string, std::pair<int,bool>> SymbolTable;
std::unordered_set<std::string> assemblerDirective;
std::vector<std::vector<std::string>> file;
std::vector<std::vector<std::string>> listing;
std::unordered_map<std::string,int> registers;

std::string intToBinaryOpcode(int value) {
    // Mask to extract the last 8 bits
    const int mask = 0xFC;

    // Extract the last 8 bits using bitwise AND
    int last8Bits = value & mask;

    // Convert the last 8 bits to a binary string
    return std::bitset<8>(last8Bits).to_string().substr(0,6);
}

std::string intToBinaryRegister(int value) {
    return std::bitset<4>(value).to_string();
}

std::string addressGenerator15bit(int value) {
    return std::bitset<15>(value).to_string();
}

std::string addressGenerator20bit(int value) {
    return std::bitset<20>(value).to_string();
}

std::string binToHex(std::string bin){
    size_t binaryLength = bin.length();

    // Convert binary string to an integer
    int intValue = std::bitset<32>(bin).to_ulong(); // Use 64-bit integer to accommodate long binary strings

    // Calculate the width for the hexadecimal output
    int hexWidth = (binaryLength + 3) / 4; // Calculate width for hex string, 4 binary digits correspond to 1 hex digit

    // Convert integer to a hexadecimal string
    std::stringstream ss;
    ss << std::hex << std::setw(hexWidth) << std::setfill('0') << intValue;
    return ss.str(); // Convert stream to string and return
}

std::pair<std::string, std::string> getRegister(const std::string& lastColumn) {
    // Remove whitespace from the last column
    std::string trimmedColumn;
    std::remove_copy_if(lastColumn.begin(), lastColumn.end(), std::back_inserter(trimmedColumn), ::isspace);

    // Split the column based on the comma separator
    size_t commaPos = trimmedColumn.find(',');
    if (commaPos == std::string::npos) {
        // If no comma is found, return an empty string for r2
        return {trimmedColumn, ""};
    } else {
        // Extract r1 and r2 from the column
        std::string r1 = trimmedColumn.substr(0, commaPos);
        std::string r2 = trimmedColumn.substr(commaPos + 1);
        return {r1, r2};
    }
}

void firstPass(
    const std::vector<std::vector<std::string>>& instructions, 
    const std::unordered_map<std::string, std::pair<std::string, int>>& opcodeTable, 
    std::unordered_map<std::string, std::pair<int, bool>>& symbolTable,
    std::unordered_set<std::string> assemblerDirective)
    {
    std::vector<std::string> temp;
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
        temp.push_back(std::to_string(locationCounter));
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

void secondPass(
    const std::string &intermediate, 
    const std::unordered_map<std::string, std::pair<std::string, int>>& opcodeTable, 
    std::unordered_map<std::string, std::pair<int, bool>>& symbolTable,
    std::unordered_set<std::string>& assemblerDirective,
    std::vector<std::vector<std::string>>& listing)
    {
    std::ifstream inter(intermediate);
    if(!inter.is_open()){
        std::cerr << "Error: Unable to open intermediate file" << std::endl;
        return;
    }
    std::string line;

    while(std::getline(inter,line)){
        std::vector<std::string> temp;
        int length;
        std::stringstream ss(line);
        std::string col1,col2,col3,col4;
        ss >> col1 >> col2 >> col3 >> col4;
        if(col4 == ""){
            col4 = col3;
            col3 = col2;
            col2 = "";
        }
        std::string opc = "";
        if(col3[0] == '+'){
            if(opcodeTable.find(col3.substr(1)) != opcodeTable.end()){
                opc = opcodeTable.at(col3.substr(1)).first;
                length = opcodeTable.at(col3.substr(1)).second;
            }
            else if(assemblerDirective.find(col3.substr(1)) != assemblerDirective.end()){
                std::cout << "found ASS : " << col3.substr(1) << std::endl;
            }
            else{
                std::cerr << "Unable to find opcode : " << col3.substr(1) << std::endl;
            }
        }
        else{
            if(opcodeTable.find(col3) != opcodeTable.end()){
                opc = opcodeTable.at(col3).first;
                length = opcodeTable.at(col3).second;
            }
            else if(assemblerDirective.find(col3) != assemblerDirective.end()){
                std::cout << "found ASS : " << col3 << std::endl;
            }
            else{
                std::cerr << "Unable to find opcode : " << col3 << std::endl;
            }
        }
        int opcode = 0;
        if(opc != ""){
            // std::cout << opc << std::endl;   
            opcode = std::stoi(opc,nullptr,16);
        }
        else{
            // This is a assembler directive
        }
        std::string binstruction = intToBinaryOpcode(opcode);
        // std::cout << col3 << " " << binstruction << std::endl;
        
        // NOW WE MUST CHECK TYPE OF INSTRUCTION
        if(length == 2){ // R type
            //opcode ko 8 bit kar diya
            binstruction.push_back('0');
            binstruction.push_back('0');
            //finding registers involved
            std::pair<std::string,std::string> regs = getRegister(col4);
            if(regs.second == ""){
                if(registers.find(regs.first)!=registers.end()){
                    int regno1 = (*(registers.find(regs.first))).second;
                    binstruction += intToBinaryRegister(regno1);
                }
                else{
                    std::cerr << "Unable to find register : " << regs.second << std::endl;
                    return;
                }
                binstruction += "0000";
            }
            else{
                if(registers.find(regs.first) != registers.end()){
                    int regno1 = (*(registers.find(regs.first))).second;
                    binstruction += intToBinaryRegister(regno1);
                }
                else{
                    std::cerr << "Unable to find register : " << regs.first << std::endl;
                    return;
                }
                if(registers.find(regs.second)!=registers.end()){
                    int regno2 = (*(registers.find(regs.second))).second;
                    binstruction += intToBinaryRegister(regno2);
                }
                else{
                    std::cerr << "Unable to find register : " << regs.second << std::endl;
                    return;
                }
            }
        }
        else if(length == 1){ 
            binstruction.push_back('0');
            binstruction.push_back('0');
        }
        else if(col3[0] == '+'){ //extended format
            // b = 0 p = 0
            if(col4[col4.size()-1] == 'X' && col4[col4.size()-2] == ','){ //indexed
                //nix = 1 e = 1
                binstruction.push_back('1');
                binstruction.push_back('1');
                binstruction.push_back('1');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('1');
            }
            else if(col4[0] == '@'){//immediate
                binstruction.push_back('1');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('1');    

            }
            else if(col4[0] == '#'){
                binstruction.push_back('0');
                binstruction.push_back('1');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('1');
            }
            else{//simple
                binstruction.push_back('1');
                binstruction.push_back('1');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('1');
                binstruction += addressGenerator20bit((*(symbolTable.find(col4))).second.first);
            }

        }
        else{ //length = 3 or error 

        }
        temp.push_back(col1);
        temp.push_back(col2);
        temp.push_back(col3);
        temp.push_back(col4);
        temp.push_back(binToHex(binstruction));
        listing.push_back(temp);
    }
}

int main(){
    OpcodeTable = importOpcodeTable("opcodes.txt");
    assemblerDirective = importAssemblerDirectives("directives.txt");
    registers = importRegisters("registers.txt");
    std::string filename;
    std::cout<<"input file name to read : ";
    std::cin>>filename; 
    file = readInputFile(filename);
    printAssembly(file);
    firstPass(file, OpcodeTable, SymbolTable, assemblerDirective);
    printIntermediateFile("intermediate.txt");
    secondPass("intermediate.txt", OpcodeTable, SymbolTable, assemblerDirective,listing);
    printListing(listing);
}