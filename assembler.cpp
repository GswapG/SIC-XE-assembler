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
std::unordered_map<std::string, std::pair<std::pair<int,int>,bool>> SymbolTable;
std::unordered_map<std::string, std::pair<int,bool>> literalTable;
std::unordered_set<std::string> assemblerDirective;
std::vector<std::vector<std::string>> file;
std::vector<std::vector<std::string>> listing;
std::vector<std::string> modification;          
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

std::string wordGenerator24bit(int value) {
    return std::bitset<24>(value).to_string();
}

std::string addressGenerator12bit(int value) {
    return std::bitset<12>(value).to_string();
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

std::string stringToAsciiHex(const std::string& str) {
    std::stringstream ss;

    // Iterate over each character of the string
    for (char c : str) {
        // Convert the character to its ASCII value
        int asciiValue = static_cast<int>(c);

        // Convert the ASCII value to its hexadecimal representation
        ss << std::hex << std::setw(2) << std::setfill('0') << asciiValue;
    }

    // Return the hexadecimal representation as a string
    return ss.str();
}

std::string generateModification(std::string &s){
    int val = stoi(s,nullptr,16);
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << std::hex << val+1;
    return ss.str();
}

std::pair<int,bool> parseExpression(const std::string &s){
    std::string a = "", b = "";
    int j = 0;
    bool found = false;
    char operand;
    for(int i = 0;i<s.size();i++){
        char c = s[i];
        if(c == '+' || c == '-' || c == '/' || c == '*'){
            j = i+1;
            found = true;
            operand = c;
            break;
        }
        else{
            a.push_back(c);
        }
    }
    if(found){
        b = s.substr(j);
        if(operand == '+'){
            if(char(a[0]) >= char('0') && char(a[0]) <= char('9') || a[0] == '-'){
                //a is a number
                if(SymbolTable.find(b) != SymbolTable.end()){
                    if(SymbolTable[b].second){
                        return std::make_pair(std::stoi(a) + SymbolTable[b].first.first,true);
                    }
                    else return std::make_pair(std::stoi(a) + SymbolTable[b].first.first,false);
                }
                else{
                    std::cerr << "ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" << std::endl;
                }
            }
            else if(char(b[0]) >= char('0') && char(b[0]) <= char('9') || b[0] == '-'){
                //b is a number
                if(SymbolTable.find(a) != SymbolTable.end()){
                    if(SymbolTable[a].second){
                        return std::make_pair(std::stoi(b) + SymbolTable[a].first.first,true);
                    }
                    else return std::make_pair(std::stoi(b) + SymbolTable[a].first.first,false);
                }
                else{
                    std::cerr << "ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" << std::endl;
                }
            }
            else if(SymbolTable.find(a) != SymbolTable.end() && SymbolTable.find(b) != SymbolTable.end()){
                if((!SymbolTable[a].second) || (!SymbolTable[b].second)){
                    std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
                }  
                else{
                    return std::make_pair(SymbolTable[a].first.first + SymbolTable[b].first.first, true);
                }
            }   
            else{
                std::cerr << "ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" << std::endl;
            }
        }
        else if(operand == '-'){
            if(char(a[0]) >= char('0') && char(a[0]) <= char('9') || a[0] == '-'){
                //a is a number
                if(SymbolTable.find(b) != SymbolTable.end()){
                    if(SymbolTable[b].second){
                        return std::make_pair(std::stoi(a) - SymbolTable[b].first.first,true);
                    }
                    else{
                        std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
                    }
                }
                else std::cerr << "ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" << std::endl;
            }
            else if(char(b[0]) >= char('0') && char(b[0]) <= char('9') || b[0] == '-'){
                //b is a number
                if(SymbolTable.find(a) != SymbolTable.end()){
                    if(SymbolTable[a].second){
                        return std::make_pair(SymbolTable[a].first.first - std::stoi(b),true);
                    }
                    else return std::make_pair(SymbolTable[a].first.first - std::stoi(b),false);
                }
                else std::cerr << "ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" << std::endl;
            }
            else if(SymbolTable.find(a) != SymbolTable.end() && SymbolTable.find(b) != SymbolTable.end()){
                if(SymbolTable[b].second){
                    //b abs
                    if(SymbolTable[a].second){
                        // a abs
                        return std::make_pair(SymbolTable[a].first.first - SymbolTable[b].first.first,true);
                    }
                    else{
                        // a rel
                        return std::make_pair(SymbolTable[a].first.first - SymbolTable[b].first.first,false);
                    }
                }
                else{
                    //b rel
                    if(SymbolTable[a].second){
                        // a abs
                        std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
                    }
                    else{
                        // a rel
                        return std::make_pair(SymbolTable[a].first.first - SymbolTable[b].first.first,true);
                    }
                }
            }
            else{
                std::cerr << "ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" << std::endl;
            }
        }
        else if(operand == '*'){
            if(char(a[0]) >= char('0') && char(a[0]) <= char('9') || a[0] == '-'){
                //a is a number
                if(SymbolTable.find(b) != SymbolTable.end()){
                    if(SymbolTable[b].second){
                        return std::make_pair(std::stoi(a)*(SymbolTable[b].first.first),true);
                    }
                    else{
                        std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
                    }
                }
                else{
                    std::cerr << "ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" << std::endl;
                }
            }
            else if(char(b[0]) >= char('0') && char(b[0]) <= char('9') || b[0] == '-'){
                std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
            }
            else if(SymbolTable.find(a) != SymbolTable.end() && SymbolTable.find(b) != SymbolTable.end()){
                if((!SymbolTable[a].second) || (!SymbolTable[b].second)){
                    std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
                }  
                else{
                    return std::make_pair(SymbolTable[a].first.first * SymbolTable[b].first.first, true);
                }
            }
            else{
                std::cerr<<"ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" <<std::endl;
            }
        }
        else if(operand == '/'){
            if(char(a[0]) >= char('0') && char(a[0]) <= char('9') || a[0] == '-'){
                //a is a number
                std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
            }
            else if(char(b[0]) >= char('0') && char(b[0]) <= char('9') || b[0] == '-'){
                if(SymbolTable.find(a) != SymbolTable.end()){
                    if(SymbolTable[a].second){
                        return std::make_pair((SymbolTable[a].first.first)/std::stoi(b),true);
                    }
                    else{
                        std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
                    }
                }
                else std::cerr << "ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" << std::endl;
            }
            else if(SymbolTable.find(a) != SymbolTable.end() && SymbolTable.find(b) != SymbolTable.end()){
                if((!SymbolTable[a].second) || (!SymbolTable[b].second)){
                    std::cerr << "WRONG RELATIVE EXPRESSION" << std::endl;
                }  
                else{
                    return std::make_pair(SymbolTable[a].first.first / SymbolTable[b].first.first, true);
                }
            }
            else{
                std::cerr<<"ERROR IN PARSING EXPRESSION, SYMBOL NOT DEFINED EARLIER" <<std::endl;
            }
        }
    }
    else{
        if(char(a[0]) >= char('0') && char(a[0]) <= char('9') || a[0] == '-'){
            return std::make_pair(std::stoi(a),true);
        }
        else return std::make_pair(SymbolTable[a].first.first,SymbolTable[a].second);
    }

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
    std::unordered_map<std::string, std::pair<std::pair<int,int>, bool>>& symbolTable,
    std::unordered_set<std::string> assemblerDirective)
    {
    int locationCounter = 0;
    
    // Open the intermediate file for writing
    std::ofstream intermediateFile("intermediate.txt");
    if (!intermediateFile.is_open()) {
        std::cerr << "Error: Unable to create intermediate file" << std::endl;
        return;
    }

    for(const auto& instruction : instructions){
        // check for literal
        if(instruction[2][0] == '='){
            //literal present
            literalTable[instruction[2].substr(1)] = std::make_pair(-1, false);
        }
        // Check if the instruction has a label
        if(!instruction[0].empty()){
            // Check if the label is already defined
            if (symbolTable.find(instruction[0]) != symbolTable.end()) {
                // Redefinition of symbol, set error flag to true
                // symbolTable[instruction[0]].second = true;
                std::cerr << "Error: Redefinition of symbol '" << instruction[0] << "'" << std::endl;
            } else {
                // Add the label to the symbol table with the current location counter
                symbolTable[instruction[0]] = std::make_pair(std::make_pair(locationCounter,0), false);
            }
        }
        if(instruction[1] == "EQU"){
            // check if *
            if(instruction[2] == "*"){
                symbolTable[instruction[0]] = std::make_pair(std::make_pair(locationCounter,0),false);
            }
            else{
                std::pair<int,bool> val = parseExpression(instruction[2]);
                symbolTable[instruction[0]] = std::make_pair(std::make_pair(val.first,0),val.second);
            }
            
            continue;
        }
        if(instruction[1] == "LTORG" || instruction[1] == "END"){
            for(auto &sym : literalTable){
                if(!sym.second.second){
                    std::string t = sym.first;
                    t = "=" + t;
                    sym.second.first = locationCounter;
                    sym.second.second = true;
                    intermediateFile << std::setw(6) << std::hex << locationCounter << " ";
                    intermediateFile << "       ";
                    intermediateFile << std::setw(7) << t;
                    intermediateFile << std::endl;
                    if(sym.first[0] == 'C'){
                        locationCounter += (sym.first.length()-3);
                    }
                    else if(sym.first[0] == 'X'){
                        locationCounter += (sym.first.length()-3)/2;
                    }
                    else{
                        std::cerr << "Can't parse literals with non-byte values" << std::endl;
                    }
                }
            }
            if(instruction[1] == "END"){
                intermediateFile << std::setw(6) << std::hex << locationCounter << " ";
                intermediateFile << std::setw(7) << "END" << " " << instruction[2] << std::endl; 
            }
            continue;
        }
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
    }

    // Close the intermediate file
    intermediateFile.close();
    storeSymbolTable("symboltable.txt",symbolTable);
}

void secondPass(
    const std::string &intermediate, 
    const std::unordered_map<std::string, std::pair<std::string, int>>& opcodeTable, 
    std::unordered_map<std::string, std::pair<std::pair<int,int>, bool>>& symbolTable,
    std::unordered_set<std::string>& assemblerDirective,
    std::vector<std::vector<std::string>>& listing)
    {
    std::ifstream inter(intermediate);
    if(!inter.is_open()){
        std::cerr << "Error: Unable to open intermediate file" << std::endl;
        return;
    }
    std::string line;
    bool base = false;
    int base_contents;
    int word_generated;
    bool directive = false;
    while(std::getline(inter,line)){
        std::vector<std::string> temp;
        int length = -1;
        std::stringstream ss(line);
        std::string col1,col2,col3,col4;
        ss >> col1 >> col2 >> col3 >> col4;
        if(col4 == ""){
            col4 = col3;
            col3 = col2;
            col2 = "";
        }
        std::cout << "assembling instruction : " << col3 << std::endl;
        std::string opc = "";
        if(col3[0] == '='){
            //literal data generate karna hai
            temp.push_back(col1);
            temp.push_back(col2);
            temp.push_back(col3);
            temp.push_back(col4);
            std::string bins;
            if(col3[1] == 'C'){
                int len_of_data = (col3.length()-4);
                bins = stringToAsciiHex(col3.substr(3,len_of_data));
            }
            else if(col3[1] == 'X'){
                int len_of_data = (col3.length()-4);
                bins = col3.substr(3,len_of_data);
            }
            else if(col3[1] == '*'){
                bins = col1;
            }
            else{
                std::cerr << "Can't generate data for literal " << std::endl;
                return;
            }
            temp.push_back(bins);
            listing.push_back(temp);
            continue;
        }
        if(col3[0] == '+'){
            if(opcodeTable.find(col3.substr(1)) != opcodeTable.end()){
                opc = opcodeTable.at(col3.substr(1)).first;
                length = opcodeTable.at(col3.substr(1)).second;
            }
            else if(assemblerDirective.find(col3.substr(1)) != assemblerDirective.end()){
                directive = true;
                std::cout << "found ASS : " << col3.substr(1) << std::endl;
                if(col3.substr(1) == "BASE"){
                    base_contents = (*(symbolTable.find(col4))).second.first.first;
                    base = true;
                }
                else if(col3.substr(1) == "NOBASE"){
                    base = false;
                }
                else if(col3.substr(1) == "WORD"){
                    length = 10;
                    word_generated = std::stoi(col4);
                }
                else if(col3.substr(1) == "BYTE"){
                    length = 100;
                }
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
                directive = true;
                std::cout << "found ASS : " << col3 << std::endl;
                if(col3 == "BASE"){
                    base_contents = (*(symbolTable.find(col4))).second.first.first;
                    base = true;
                }
                else if(col3 == "NOBASE"){
                    base = false;
                }
                else if(col3 == "WORD"){
                    length = 10;
                    word_generated = std::stoi(col4);
                }
                else if(col3 == "BYTE"){
                    length = 100;
                }
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
            //onl opcode nothing else
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
                binstruction += addressGenerator20bit((*(symbolTable.find(col4.substr(0,col4.length()-2)))).second.first.first);
                modification.push_back(generateModification(col1));
            }
            else if(col4[0] == '@'){//immediate
                binstruction.push_back('1');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('1');  
                binstruction += addressGenerator20bit((*(symbolTable.find(col4.substr(1)))).second.first.first);
                modification.push_back(generateModification(col1));
            }
            else if(col4[0] == '#'){
                binstruction.push_back('0');
                binstruction.push_back('1');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('1');
                if(char(col4[1]) >= char('0') && char(col4[1]) <= char('9') || col4[1] == '-'){
                    //number
                    int val = std::stoi(col4.substr(1));
                    binstruction += addressGenerator20bit(val);
                }
                else{
                    //label
                    binstruction += addressGenerator20bit((*(symbolTable.find(col4.substr(1)))).second.first.first);
                    modification.push_back(generateModification(col1));
                }
                
            }
            else{//simple
                binstruction.push_back('1');
                binstruction.push_back('1');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('0');
                binstruction.push_back('1');
                if(col4[0] == '='){
                    binstruction += addressGenerator20bit((*(literalTable.find(col4.substr(1)))).second.first);
                }
                else binstruction += addressGenerator20bit((*(symbolTable.find(col4))).second.first.first);
                modification.push_back(generateModification(col1));
            }

        }
        else if(length == 3){ //length = 3
            // first try PC relative
            // then try base relative
            //check if RSUB
            if(binstruction == "010011"){
                binstruction = "010011110000000000000000";
            }
            else if(col4[0] == '#'){
                //immediate addressing
                binstruction.push_back('0');
                binstruction.push_back('1');
                if(char(col4[1]) >= char('0') && char(col4[1]) <= char('9') || col4[1] == '-'){
                    //number
                    int val = std::stoi(col4.substr(1));
                    binstruction += "0000";
                    binstruction += addressGenerator12bit(val);
                }
                else{
                    //label 
                    //try PC relative
                    int pc = std::stoi(col1,nullptr,16) + length;
                    int disp = (*(symbolTable.find(col4.substr(1)))).second.first.first - pc;
                    if(disp >= -2048 && disp <= 2047){
                        binstruction += "0010";
                        binstruction += addressGenerator12bit(disp);
                    }
                    else if(base){
                        disp = (*(symbolTable.find(col4.substr(1)))).second.first.first - base_contents;
                        if(disp >= 0 && disp <= 4095){
                            binstruction += "0100";
                            binstruction += addressGenerator12bit(disp);
                        }
                        else{
                            std::cerr << "Address too large to fit in 12 bits !!" << std::endl;
                        }
                    }
                    else{
                        std::cerr << "Too large for PC relative, and NOBASE!!" << std::endl;
                    }
                }   
            } 
            else if(col4[0] == '@'){
                //indirect
                binstruction.push_back('1');
                binstruction.push_back('0');
                if(char(col4[1]) >= char('0') && char(col4[1]) <= char('9') || col4[1] == '-'){
                    //number
                    int val = std::stoi(col4.substr(1));
                    binstruction += "0000";
                    binstruction += addressGenerator12bit(val);
                }
                else{
                    //label 
                    //try PC relative
                    int pc = std::stoi(col1,nullptr,16) + length;
                    int disp = (*(symbolTable.find(col4.substr(1)))).second.first.first - pc;
                    if(disp >= -2048 && disp <= 2047){
                        binstruction += "0010";
                        binstruction += addressGenerator12bit(disp);
                    }
                    else if(base){
                        disp = (*(symbolTable.find(col4.substr(1)))).second.first.first - base_contents;
                        if(disp >= 0 && disp <= 4095){
                            binstruction += "0100";
                            binstruction += addressGenerator12bit(disp);
                        }
                        else{
                            std::cerr << "Address too large to fit in 12 bits !!" << std::endl;
                        }
                    }
                    else{
                        std::cerr << "Too large for PC relative, and NOBASE!!" << std::endl;
                    }
                }   
                
            }
            else{
                //simple addressing
                binstruction.push_back('1');
                binstruction.push_back('1');
                if(col4[col4.length()-1] == 'X'){   
                    //indexed
                    if(char(col4[1]) >= char('0') && char(col4[1]) <= char('9') || col4[1] == '-'){
                        //numeral
                        binstruction += "1000";
                        binstruction += addressGenerator12bit(std::stoi(col4.substr(0,col4.length()-2)));
                    }
                    else{
                        //label
                        int pc = std::stoi(col1,nullptr,16) + length;
                        int disp = (*(symbolTable.find(col4.substr(0,col4.length()-2)))).second.first.first - pc;
                        if(disp >= -2048 && disp <= 2047){
                            binstruction += "1010";
                            binstruction += addressGenerator12bit(disp);
                        }
                        else if(base){
                            disp = (*(symbolTable.find(col4.substr(0,col4.length()-2)))).second.first.first - base_contents;
                            if(disp >= 0 && disp <= 4095){
                                binstruction += "1100";
                                binstruction += addressGenerator12bit(disp);
                            }
                            else{
                                std::cerr << "Address too large to fit in 12 bits !!" << std::endl;
                            }
                        }
                        else{
                            std::cerr << "Too large for PC relative, and NOBASE!!" << std::endl;
                        }
                    }
                }
                else{
                    //normal 
                    if(char(col4[1]) >= char('0') && char(col4[1]) <= char('9') || col4[1] == '-'){
                        //numeral
                        binstruction += "0000";
                        binstruction += addressGenerator12bit(std::stoi(col4));
                    }
                    else{
                        //label
                        int pc = std::stoi(col1,nullptr,16) + length;
                        int disp;
                        if(col4[0] == '='){
                            disp = (*(literalTable.find(col4.substr(1)))).second.first - pc;
                        }
                        else disp = (*(symbolTable.find(col4))).second.first.first - pc;
                        if(disp >= -2048 && disp <= 2047){
                            binstruction += "0010";
                            binstruction += addressGenerator12bit(disp);
                        }
                        else if(base){
                            disp = (*(symbolTable.find(col4))).second.first.first - base_contents;
                            if(disp >= 0 && disp <= 4095){
                                binstruction += "0100";
                                binstruction += addressGenerator12bit(disp);
                            }
                            else{
                                std::cerr << "Address too large to fit in 12 bits !!" << std::endl;
                            }
                        }
                        else{
                            std::cerr << "Too large for PC relative, and NOBASE!!" << std::endl;
                        }
                    }

                }
            }
            
        }   
        else if(length == 10){
            binstruction = wordGenerator24bit(word_generated);
        }
        else if(length == 100){
            if(col4[0] == 'X'){
                //hex data
                int len_of_data = (col4.length()-3);
                binstruction = col4.substr(2,len_of_data);
            }   
            else if(col4[0] == 'C'){
                //string data
                int len_of_data = (col4.length()-3);
                binstruction = stringToAsciiHex(col4.substr(2,len_of_data));
            }
        }
        temp.push_back(col1);
        temp.push_back(col2);
        temp.push_back(col3);
        temp.push_back(col4);
        if(length != -1 && length != 100) temp.push_back(binToHex(binstruction));
        if(length == 100) temp.push_back(binstruction);
        else temp.push_back("");
        listing.push_back(temp);
    }
}

void writeObjectFile(std::vector<std::vector<std::string>> listing, std::string objFile){
    std::ofstream obj(objFile);
    if(!obj.is_open()){
        std::cerr << "Cannot write to file : " << objFile << std::endl;
        return;
    }

    //====HEADER====
    obj << "H";
    obj << std::setw(6) << std::left << listing[0][1];
    obj << "000000";
    obj << std::setw(6) << std::right << std::setfill('0') << listing[listing.size()-1][0];
    obj << std::endl;
    //====TEXT======
    int i = 1;
    while(i < listing.size()-1){
        int size = 0;
        int j = i;
        std::stringstream tempss;
        while(size + listing[j][4].length()/2 <= 30 && j < listing.size()-1){
            if(listing[j][4] == "" && listing[j+1][0] != listing[j][0]){
                j++;
                break;
            }
            else{
                tempss << " " << listing[j][4];
                size += listing[j][4].length()/2;
                j++;
            }
        }
        if(tempss.str() == ""){
            i = j;
            continue;
        }
        obj << "T";
        obj << std::setw(6) << std::setfill('0') << listing[i][0];
        obj << std::setw(2) << std::setfill('0') << std::hex << size;
        obj << tempss.str();
        obj << std::endl;
        i = j;
    }   
    //=MODIFICATION=
    for(auto &mod : modification){
        obj << "M";
        obj << mod;
        obj << "05";
        obj << std::endl;
    }
    //=====END======
    obj << "E";
    obj << "000000";
    obj << std::endl;

    obj.close();
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
    printSymbolTable(SymbolTable);
    // printSymbolTable(literalTable);
    printIntermediateFile("intermediate.txt");
    secondPass("intermediate.txt", OpcodeTable, SymbolTable, assemblerDirective,listing);
    printListing(listing);
    storeListing(listing,"listing.txt");
    writeObjectFile(listing,"obj.txt");
}