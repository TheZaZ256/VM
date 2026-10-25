// InstructionDecoder.h
#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct DecodedCommand {
    uint8_t opcode;
    uint8_t r1;
    uint8_t r2;
    uint8_t r3;
    uint16_t address;  // для 32-битных команд
};

class InstructionDecoder {
public:
    // ★★★ НОВАЯ ПРОВЕРКА по opcode ★★★
    static bool is32BitCommand(uint8_t opcode) {
        return (opcode & 0x40) != 0;  // бит 6 (64) = 1 → 32-битная
    }

    // Декодирование строки в команду (автоматически определяет длину)
    static DecodedCommand decodeCommand(const std::string& cmd_str);

    // Декодирование 16-битного слова
    static void decodeCommandWord(uint16_t word, DecodedCommand& cmd);

    // Кодирование команды в 16-битное слово
    static uint16_t encodeCommandWord(const DecodedCommand& cmd);

    // Загрузка команды в память
    static void loadCommandToMemory(DecodedCommand& cmd, class Memory& memory,
        uint16_t& current_addr, bool is32bit);

    // Вспомогательные методы для отладки
    static void printBytesInfo(const std::vector<uint8_t>& bytes, const std::string& title);
    static void printDecodedResult(const DecodedCommand& cmd, bool is32bit);

private:
    // Парсинг строки в байты
    static std::vector<uint8_t> parseBytes(const std::string& cmd_str);

};