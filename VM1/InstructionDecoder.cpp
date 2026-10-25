// InstructionDecoder.cpp
#include "InstructionDecoder.h"
#include "Memory.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>
#include "Config.h"

// ★★★ ПАРСИНГ СТРОКИ В БАЙТЫ ★★★
std::vector<uint8_t> InstructionDecoder::parseBytes(const std::string& cmd_str) {
    std::vector<uint8_t> bytes;
    std::stringstream ss(cmd_str);
    uint32_t byte;

    while (ss >> std::hex >> byte) {
        bytes.push_back(static_cast<uint8_t>(byte));
    }

    return bytes;
}

// ★★★ ГЛАВНЫЙ МЕТОД ДЕКОДИРОВАНИЯ (сам определяет длину) ★★★
DecodedCommand InstructionDecoder::decodeCommand(const std::string& cmd_str) {
    std::vector<uint8_t> bytes = parseBytes(cmd_str);

    if (bytes.empty()) {
        std::cerr << "Ошибка: пустая команда!" << std::endl;
        return DecodedCommand{ 0, 0, 0, 0, 0 };
    }

    // Собираем первое слово (первые 2 байта)
    uint16_t word1 = 0;
    if (bytes.size() >= 1) word1 |= (bytes[0] << 8);
    if (bytes.size() >= 2) word1 |= bytes[1];

    // Декодируем первое слово
    DecodedCommand cmd;
    decodeCommandWord(word1, cmd);

    // Определяем длину по opcode
    bool is32bit = is32BitCommand(cmd.opcode);

    if (Config::verbose) {

        printBytesInfo(bytes, is32bit ? "=== Декодирование 32-битной команды ===" : "=== Декодирование 16-битной команды ===");
    }


    // Если 32-битная, читаем второе слово (адрес)
    if (is32bit && bytes.size() >= 4) {
        uint16_t word2 = (bytes[2] << 8) | bytes[3];
        cmd.address = word2;
        if (Config::verbose) {
            std::cout << "Второе слово (адрес): " << std::bitset<16>(word2) << std::endl;

            printDecodedResult(cmd, true);
        }
    }
    else {
        cmd.address = 0;
        if (Config::verbose) {

            printDecodedResult(cmd, false);
        }
    }

    return cmd;
}

// Декодирование 16-битного слова
void InstructionDecoder::decodeCommandWord(uint16_t word, DecodedCommand& cmd) {
    cmd.opcode = (word >> 9) & 0x7F;
    cmd.r1 = (word >> 6) & 0x07;
    cmd.r2 = (word >> 3) & 0x07;
    cmd.r3 = word & 0x07;
}

// Кодирование команды в 16-битное слово
uint16_t InstructionDecoder::encodeCommandWord(const DecodedCommand& cmd) {
    return (cmd.opcode << 9) | (cmd.r1 << 6) | (cmd.r2 << 3) | cmd.r3;
}

// Вывод информации о байтах
void InstructionDecoder::printBytesInfo(const std::vector<uint8_t>& bytes, const std::string& title) {
    std::cout << "\n" << title << std::endl;

    std::cout << "Байты: ";
    for (size_t i = 0; i < bytes.size(); i++) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
        if (i < bytes.size() - 1) std::cout << " ";
    }
    std::cout << std::endl;

    for (size_t i = 0; i < bytes.size(); i++) {
        std::cout << "Биты байта " << i << ": " << std::bitset<8>(bytes[i]) << std::endl;
    }
}

// Вывод результата декодирования
void InstructionDecoder::printDecodedResult(const DecodedCommand& cmd, bool is32bit) {
    std::cout << "Результат: op=0x" << std::hex << std::setw(2) << std::setfill('0')
        << (int)cmd.opcode
        << ", r1=" << std::dec << (int)cmd.r1
        << ", r2=" << (int)cmd.r2
        << ", r3=" << (int)cmd.r3;

    if (is32bit) {
        std::cout << ", addr=0x" << std::hex << std::setw(4) << std::setfill('0')
            << cmd.address;
    }
    std::cout << std::endl;
}

// Загрузка команды в память
void InstructionDecoder::loadCommandToMemory(DecodedCommand& cmd, Memory& memory,
    uint16_t& current_addr, bool is32bit) {
    if (Config::verbose) {

        std::cout << "\nЗагрузка команды в память:" << std::endl;
    }

    uint16_t word1 = encodeCommandWord(cmd);

    if (Config::verbose) {
       
        std::cout << "Запись слова 0x" << std::hex << std::setw(4) << std::setfill('0')
            << word1 << " по адресу 0x" << std::setw(4) << current_addr << std::endl;
        
    }

    memory.write(current_addr, word1);

    if (is32bit) {
        uint16_t word2 = cmd.address;
        if (Config::verbose) {
            
        std::cout << "Запись второго слова 0x" << std::setw(4) << word2
            << " по адресу 0x" << std::setw(4) << (current_addr + 1) << std::endl;
        }
        memory.write(current_addr + 1, word2);
        current_addr += 2;
    }
    else {
        current_addr += 1;
    }
}