#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <algorithm>

void printAssembly(
    std::vector<std::vector<std::string>> &instructions)
    {
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
            std::cout << std::setw(11) << row[i];
            // Reset color
            std::cout << "\033[0m";
        }
        std::cout << std::endl;
    }

}

void printIntermediateFile(const std::string& filename){
    // Open the intermediate file for reading
    std::ifstream intermediateFile(filename);
    if (!intermediateFile.is_open()) {
        std::cerr << "Error: Unable to open intermediate file" << std::endl;
        return;
    }

    // Read and print each line of the intermediate file
    std::string line;
    while (std::getline(intermediateFile, line)) {
        // Extract columns from the line
        std::string col1, col2, col3, col4,col5;
        std::istringstream iss(line);
        iss >> col1 >> col2 >> col3 >> col4 >> col5;
        if(col5 == "" && col4 == ""){
            //no label and no operand
            col5 = col3;
            col3 = col2;
            col2 = "";
        }
        else if(col5 == ""){
            //no label
            col5 = col4;
            col4 = col3;
            col3 = col2;
            col2 = "";
        }
        // Print each column with different colors
        std::cout << "\033[0;37m" << std::setw(11) << col1 << " ";  // White color for col1
        std::cout << "\033[0;34m" << std::setw(11) << col2 << " ";  // Blue color for col2
        std::cout << "\033[0;32m" << std::setw(11) << col3 << " ";  // Green color for col3
        std::cout << "\033[0;33m" << std::setw(11) << col4 << "\033[0m"; // Yellow color for col4
        std::cout << "\033[0;33m" << std::setw(11) << col5 << "\033[0m" << std::endl;
        // Reset color to default
        std::cout << "\033[0m";
    }

    // Close the intermediate file
    intermediateFile.close();
}

void printListing(
    std::vector<std::vector<std::string>> &listing)
    {
    for (const auto& row : listing) {
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
                case 3:
                    std::cout << "\033[31m";
                    break;
                case 4:
                    std::cout << "\033[32m";
                    break;
                default:
                    break;
            }
            std::cout << std::setw(11) << row[i];
            // Reset color
            std::cout << "\033[0m";
        }
        std::cout << std::endl;
    }

}

void printSymbolTable(const std::unordered_map<std::string, std::pair<std::pair<int,int>, bool>>& symbolTable) {
    std::cout << "===========SYMBOL TABLE===========" << std::endl;
    // Print header with different colors for each column
    std::cout << "\033[0;32m" << std::setw(7) << std::setfill(' ') << "Symbol";
    std::cout << "\033[0;34m" << std::setw(16) << std::setfill(' ') << "Memory Location";
    std::cout << "\033[0;35m" << std::setw(11) << std::setfill(' ') << "Error Flag";
    std::cout << "\033[0;35m" << std::setw(11) << std::setfill(' ') << "Block";
    std::cout << "\033[0m" << std::endl;

    // Print each entry in the symbol table
    for (const auto& entry : symbolTable) {
        // Print symbol name in white color
        std::cout << "\033[0;32m" << std::setw(7) << std::setfill(' ') << entry.first;

        // Print decimal memory location in blue color
        std::cout << "\033[0;34m" << std::setw(16) << std::setfill(' ') << std::hex << entry.second.first.first;

        // Print error flag in magenta color
        std::cout << "\033[0;35m" << std::setw(11) << std::setfill(' ') << entry.second.second;

        // Reset color and print newline
        std::cout << "\033[0;37m" << std::setw(11) << std::setfill(' ') << entry.second.first.second;

        std::cout << "\033[0m" << std::endl;
    }
    std::cout << "==================================" << std::endl;
}