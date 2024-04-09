#ifndef PRINT_UTILITY_HPP
#define PRINT_UTILITY_HPP
#include <string>
#include <vector>

void printAssembly(std::vector<std::vector<std::string>> &instructions);

void printIntermediateFile(const std::string& filename);

void printListing(
    std::vector<std::vector<std::string>> &listing);

void printSymbolTable(const std::unordered_map<std::string, std::pair<int, bool>>& symbolTable);
#endif