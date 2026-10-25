#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

class Assembler {
private:
    std::vector<uint16_t> output;
    std::map<std::string, uint8_t> opcodes16;
    std::map<std::string, uint8_t> opcodes32;
    std::map<std::string, uint8_t> registers;
    std::map<std::string, uint16_t> labels;
    std::map<std::string, uint8_t> conditions;

    uint16_t currentAddress = 0;

public:
    Assembler();

    void assemble(const std::string& inputFile, uint16_t startAddr);
    void saveToText(const std::string& filename);
    void saveToBinary(const std::string& filename);

private:
    int32_t parseNumber(const std::string& str);
    uint16_t startAddress = 0x1000;  // начальный адрес загрузки
    void firstPass(const std::string& filename);
    void secondPass(const std::string& filename);

    void assembleMOV(std::stringstream& ss);
    void assembleArithmetic(const std::string& op, std::stringstream& ss);
    void assembleCmp(std::stringstream& ss, bool isFloat);
    void assembleCmp(std::stringstream& ss);  // для CMP
    void assembleIO(const std::string& op, std::stringstream& ss);
    void assembleJmpR(std::stringstream& ss);
    void assembleJmpC(std::stringstream& ss);
    void assembleJmpI(std::stringstream& ss);
    void assembleRet(std::stringstream& ss);
    void assembleJump(std::stringstream& ss);
    void assembleCall(std::stringstream& ss);
    void assembleMovRR(std::stringstream& ss);

    void encode16(uint8_t op, uint8_t r1, uint8_t r2, uint8_t r3);
    void encode32(uint8_t op, uint8_t r1, uint8_t r2, uint8_t r3, uint16_t addr);
    uint16_t resolveLabel(const std::string& label, uint16_t currentAddr);
};