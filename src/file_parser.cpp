#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <algorithm>

std::vector<std::vector<std::string>> readInputFile(
    const std::string& filename) 
    {
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

std::unordered_map<std::string, std::pair<std::string, int>> importOpcodeTable(
    const std::string& filename) 
    {
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

std::unordered_map<std::string, std::pair<int, bool>> importSymbolTable(
    const std::string& filename) 
    {
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

std::unordered_set<std::string> importAssemblerDirectives(
    std::string filename)
    {
    std::unordered_set<std::string> assemblerDirective;
    std::ifstream file(filename);
    // Check if the file exists and can be opened
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return assemblerDirective; 
    }
    std::string line;
    while(std::getline(file,line)){
        assemblerDirective.insert(line);
    }
    file.close();
    return assemblerDirective;
}

void storeSymbolTable(
    const std::string& filename, 
    const std::unordered_map<std::string, 
    std::pair<int, bool>>& symbolTable) 
    {
    // Open the CSV file for writing
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to create CSV file" << std::endl;
        return;
    }

    // Write column headers
    outputFile << "Label, Location, Error Flag" << std::endl;

    // Write data from the symbol table
    for (const auto& entry : symbolTable) {
        outputFile << entry.first << ", " << entry.second.first << ", " << std::boolalpha << entry.second.second << std::endl;
    }

    // Close the CSV file
    outputFile.close();
}

std::unordered_map<std::string, int> importRegisters(const std::string& filename) {
    std::unordered_map<std::string, int> registers;

    // Open the registers file for reading
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return registers;
    }

    // Read each line from the file and extract register mnemonic and number
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string mnemonic;
        int number;
        if (iss >> mnemonic >> number) {
            registers[mnemonic.substr(0,1)] = number;
        } else {
            std::cerr << "Error: Invalid line format in file " << filename << std::endl;
        }
    }

    // Close the file
    file.close();

    return registers;
}

void storeListing(const std::vector<std::vector<std::string>>& listing, const std::string& filename){
    // Open the file for writing
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return;
    }

    // Iterate over each line in the listing and write to the file
    for (const auto& line : listing) {
        for (const auto& word : line) {
            file << word << " "; // Write each word separated by space
        }
        file << std::endl; // Write newline after each line
    }

    // Close the file
    file.close();
}