#ifndef PRINT_UTILITY_HPP
#define PRINT_UTILITY_HPP
#include <string>
#include <vector>

void printAssembly(std::vector<std::vector<std::string>> &instructions);

void printIntermediateFile(const std::string& filename);
#endif