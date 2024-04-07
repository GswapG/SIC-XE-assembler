#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <algorithm>

std::vector<std::vector<std::string>> readInputFile(const std::string& filename) {
    std::vector<std::vector<std::string>> result;
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return result; // Return empty vector if file cannot be opened
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::vector<std::string> columns;
        std::stringstream ss(line);
        std::string col1, col2, col3;

        // Extract columns using whitespace as delimiter
        ss >> col1 >> col2 >> col3;
        if(col3 == ""){
            col3 = col2;
            col2 = col1;
            col1 = "";
        }
        // Push extracted columns into the result vector
        columns.push_back(col1);
        columns.push_back(col2);
        columns.push_back(col3);

        result.push_back(columns);
    }

    inputFile.close();
    return result;
}

std::unordered_map<std::string, std::pair<std::string, int>> importOpcodeTable(const std::string& filename) {
    std::unordered_map<std::string, std::pair<std::string, int>> opcodeTable;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return opcodeTable; // Return empty map if file cannot be opened
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string mnemonic, opcode;
        int length;

        // Read fields from the line
        if (std::getline(ss, mnemonic, ',') &&
            std::getline(ss, opcode, ',') &&
            (ss >> length)) {
            // Insert data into the unordered map
            opcodeTable[mnemonic] = std::make_pair(opcode, length);
        } else {
            // Error handling if line is not formatted correctly
            std::cerr << "Error: Invalid line format in file" << std::endl;
        }
    }

    file.close();
    return opcodeTable;
}

void printAssembly(std::vector<std::vector<std::string>> &instructions){
    for (const auto& row : instructions) {
        for (size_t i = 0; i < row.size(); ++i) {
            switch (i) {
                case 0:
                    std::cout << "\033[34m"; // Blue
                    break;
                case 1:
                    std::cout << "\033[32m"; // Green
                    break;
                case 2:
                    std::cout << "\033[33m"; // Yellow
                    break;
                default:
                    break;
            }
            std::cout << std::setw(8) << row[i];
            // Reset color
            std::cout << "\033[0m";
        }
        std::cout << std::endl;
    }

}

std::unordered_map<std::string, std::pair<int, bool>> importSymbolTable(const std::string& filename) {
    std::unordered_map<std::string, std::pair<int, bool>> symbolTable;
    std::ifstream file(filename);

    // Check if the file exists and can be opened
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return symbolTable; // Return empty map if file cannot be opened
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string symbolName;
        int memoryLocation;
        bool errorFlag;

        // Read fields from the line
        if (std::getline(ss, symbolName, ',') && (ss >> memoryLocation) && (ss >> std::boolalpha >> errorFlag)) {
            // Insert data into the unordered map
            symbolTable[symbolName] = std::make_pair(memoryLocation, errorFlag);
        } else {
            // Error handling if line is not formatted correctly
            std::cerr << "Error: Invalid line format in file" << std::endl;
        }
    }

    file.close();
    return symbolTable;
}

void firstPass(const std::vector<std::vector<std::string>>& instructions, const std::unordered_map<std::string, std::pair<std::string, int>>& opcodeTable, std::unordered_map<std::string, std::pair<int, bool>>& symbolTable) {
    int locationCounter = 0;
    
    // Open the intermediate file for writing
    std::ofstream intermediateFile("intermediate.txt");
    if (!intermediateFile.is_open()) {
        std::cerr << "Error: Unable to create intermediate file" << std::endl;
        return;
    }

    for (const auto& instruction : instructions) {
        // Write location counter and instruction to the intermediate file
        intermediateFile << std::setw(4) << std::hex << locationCounter << " "; // Set width to 4 characters for hexadecimal output
        if(!instruction[0].empty()){
            intermediateFile << instruction[0];
            intermediateFile << " ";
        }
        else{
            intermediateFile << "     ";
        }
        //opcode check
        if(instruction[1][0] == '+'){
            // Write '+' character for instructions starting with '+'
            intermediateFile << instruction[1];
            // Check opcode table without '+'
            if (opcodeTable.find(instruction[1].substr(1)) != opcodeTable.end()) {
                locationCounter += 4;
            } else {
                std::cerr << "Error: Opcode not found for mnemonic '" << instruction[1] << "'" << std::endl;
            }
        }
        else{
            intermediateFile << instruction[1];
            // Check opcode table normally
            if (opcodeTable.find(instruction[1]) != opcodeTable.end()) {
                locationCounter += opcodeTable.at(instruction[1]).second;
            } else {
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
}

int main(){
    std::unordered_map<std::string, std::pair<std::string,int>> OpcodeTable;
    std::unordered_map<std::string, std::pair<int,bool>> SymbolTable;
    OpcodeTable = importOpcodeTable("opcodes.txt");
    std::string filename;
    std::cout<<"input file name to read : ";
    std::cin>>filename; 
    std::vector<std::vector<std::string>> file = readInputFile(filename);
    printAssembly(file);
    firstPass(file, OpcodeTable, SymbolTable);
}