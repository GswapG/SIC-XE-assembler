#ifndef FILE_PARSER_HPP
#define FILE_PARSER_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>

std::vector<std::vector<std::string>> readInputFile(
    const std::string& filename);

std::unordered_map<std::string, std::pair<std::string, int>> importOpcodeTable(
    const std::string& filename);

std::unordered_map<std::string, std::pair<int, bool>> importSymbolTable(
    const std::string& filename);

std::unordered_set<std::string> importAssemblerDirectives(
    std::string filename);

void storeSymbolTable(
    const std::string& filename, 
    const std::unordered_map<std::string,std::pair<int, bool>>& symbolTable);

std::unordered_map<std::string, int> importRegisters(const std::string& filename);

#endif