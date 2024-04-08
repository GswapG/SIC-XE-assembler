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
        std::string col1, col2, col3, col4;
        std::istringstream iss(line);
        iss >> col1 >> col2 >> col3 >> col4;
        if(col4 == ""){
            col4 = col3;
            col3 = col2;
            col2 = "";
        }
        // Print each column with different colors
        std::cout << "\033[0;37m" << std::setw(11) << col1 << " ";  // White color for col1
        std::cout << "\033[0;34m" << std::setw(11) << col2 << " ";  // Blue color for col2
        std::cout << "\033[0;32m" << std::setw(11) << col3 << " ";  // Green color for col3
        std::cout << "\033[0;33m" << std::setw(11) << col4 << "\033[0m" << std::endl; // Yellow color for col4

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