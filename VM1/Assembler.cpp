#include "assembler.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include "Config.h"

uint32_t parseFloatFromString(const std::string& str) {
    // Простейший парсинг десятичной дроби
    int integerPart = 0;
    int fractionalPart = 0;
    int fractionalDigits = 0;
    bool isNegative = false;
    bool afterDot = false;

    size_t i = 0;
    if (str[0] == '-') {
        isNegative = true;
        i = 1;
    }

    for (; i < str.length(); i++) {
        char c = str[i];
        if (c == '.' || c == ',') {
            afterDot = true;
            continue;
        }
        if (c >= '0' && c <= '9') {
            if (!afterDot) {
                integerPart = integerPart * 10 + (c - '0');
            }
            else {
                fractionalPart = fractionalPart * 10 + (c - '0');
                fractionalDigits++;
            }
        }
    }

    float val = integerPart + fractionalPart / std::pow(10.0f, fractionalDigits);
    if (isNegative) val = -val;

    uint32_t bits;
    memcpy(&bits, &val, 4);

    if (Config::verbose) {

        std::cout << "DEBUG: parseFloat(" << str << ") = " << val << " -> 0x" << std::hex << bits << std::dec << std::endl;
    }

    return bits;
}

Assembler::Assembler() {
    // Регистры
    for (int i = 0; i < 8; i++) {
        registers["R" + std::to_string(i)] = i;
        if (Config::verbose) {
            std::cout << "DEBUG: registers[R" << i << "] = " << i << std::endl;
        }
    }

    // Условия для условных переходов
    conditions["EQ"] = 0;
    conditions["NE"] = 1;
    conditions["LT"] = 2;
    conditions["GT"] = 3;
    conditions["LE"] = 4;
    conditions["GE"] = 5;

    // 16-битные команды
    opcodes16["ADD"] = 0x01;
    opcodes16["SUB"] = 0x02;
    opcodes16["MUL"] = 0x03;
    opcodes16["DIV"] = 0x04;
    opcodes16["MOD"] = 0x05;
    opcodes16["ADDF"] = 0x06;
    opcodes16["SUBF"] = 0x07;
    opcodes16["MULF"] = 0x08;
    opcodes16["DIVF"] = 0x09;
    opcodes16["CMP"] = 0x0A;
    opcodes16["CMPF"] = 0x0B;
    opcodes16["AND"] = 0x0C;
    opcodes16["OR"] = 0x0D;
    opcodes16["XOR"] = 0x0E;
    opcodes16["NOT"] = 0x0F;
    opcodes16["SHL"] = 0x10;
    opcodes16["SHR"] = 0x11;
    opcodes16["SAR"] = 0x12;
    opcodes16["JMPR"] = 0x13;
    opcodes16["JMPC"] = 0x14;
    opcodes16["JMPI"] = 0x15;
    opcodes16["RET"] = 0x16;
    opcodes16["IN"] = 0x17;
    opcodes16["OUT"] = 0x18;
    opcodes16["INF"] = 0x19;
    opcodes16["OUTF"] = 0x1A;
    opcodes16["MOVRR"] = 0x20;

    // 32-битные команды
    opcodes32["MOV"] = 0x40;
    opcodes32["JUMP"] = 0x41;
    opcodes32["CALL"] = 0x43;
}

void Assembler::assemble(const std::string& inputFile, uint16_t startAddr) {
    startAddress = startAddr;
    firstPass(inputFile);
    secondPass(inputFile);
    if (Config::verbose) {
        std::cout << "Собрано " << output.size() << " слов" << std::endl;
    }
}

void Assembler::saveToText(const std::string& filename) {
    std::ofstream out(filename);
    size_t i = 0;

    while (i < output.size()) {
        uint16_t first_word = output[i];
        uint8_t opcode = (first_word >> 9) & 0x7F;

        // Проверяем, 32-битная ли команда
        bool is32bit = (opcode & 0x40) != 0;

        if (is32bit && i + 1 < output.size()) {
            // 32-битная команда: 2 слова подряд
            uint16_t second_word = output[i + 1];
            uint8_t hi1 = (first_word >> 8) & 0xFF;
            uint8_t lo1 = first_word & 0xFF;
            uint8_t hi2 = (second_word >> 8) & 0xFF;
            uint8_t lo2 = second_word & 0xFF;

            out << std::hex << std::setw(2) << std::setfill('0') << (int)hi1 << " "
                << std::hex << std::setw(2) << std::setfill('0') << (int)lo1 << " "
                << std::hex << std::setw(2) << std::setfill('0') << (int)hi2 << " "
                << std::hex << std::setw(2) << std::setfill('0') << (int)lo2 << std::endl;
            i += 2;
        }
        else {
            // 16-битная команда: 1 слово
            uint8_t hi = (first_word >> 8) & 0xFF;
            uint8_t lo = first_word & 0xFF;
            out << std::hex << std::setw(2) << std::setfill('0') << (int)hi << " "
                << std::hex << std::setw(2) << std::setfill('0') << (int)lo << std::endl;
            i += 1;
        }
    }

    std::cout << "Сохранено " << output.size() << " слов в " << filename << std::endl;
}

void Assembler::saveToBinary(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    for (uint16_t word : output) {
        uint8_t bytes[2];
        bytes[0] = word & 0xFF;
        bytes[1] = (word >> 8) & 0xFF;
        out.write(reinterpret_cast<char*>(bytes), 2);
    }
    std::cout << "Сохранено в бинарный файл: " << filename << std::endl;
}

void Assembler::firstPass(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    currentAddress = 0;

    while (std::getline(file, line)) {
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);

        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;

        if (line.back() == ':') {
            std::string label = line.substr(0, line.length() - 1);
            labels[label] = currentAddress;
            if (Config::verbose) {

                std::cout << "DEBUG: firstPass label '" << label << "' = 0x" << std::hex << currentAddress << std::dec << std::endl;
            }
            continue;
        }

        std::stringstream ss(line);
        std::string op;
        ss >> op;

        if (op == "MOVRR") {
            currentAddress += 1;
        }
        else if (op == "MOV" || op == "JUMP" || op == "CALL" || op == "JMPC") {
            currentAddress += 2;
        }
        else {
            currentAddress += 1;
        }
    }
}

void Assembler::secondPass(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    currentAddress = 0;
    output.clear();

    while (std::getline(file, line)) {
        size_t commentPos = line.find(';');

        if (commentPos != std::string::npos) line = line.substr(0, commentPos);

        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;
        if (line.back() == ':') continue;

        std::stringstream ss(line);
        std::string op;
        ss >> op;
        if (Config::verbose) {

            std::cout << "DEBUG: op='" << op << "'" << std::endl;
        }
        if (op == "MOV" || op == "MOVF") {
            assembleMOV(ss);
        }
        else if (op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV" || op == "MOD" ||
            op == "ADDF" || op == "SUBF" || op == "MULF" || op == "DIVF" ||
            op == "AND" || op == "OR" || op == "XOR" || op == "NOT" ||
            op == "SHL" || op == "SHR" || op == "SAR") {
            if (Config::verbose) {

                std::cout << "DEBUG: arithmetic op detected: '" << op << "'" << std::endl;
            }
            assembleArithmetic(op, ss);
        }
        else if (op == "CMP") {
            assembleCmp(ss, false);
        }
        else if (op == "CMPF") {
            assembleCmp(ss, true);
        }
        else if (op == "IN" || op == "OUT" || op == "INF" || op == "OUTF") {
            assembleIO(op, ss);
        }
        else if (op == "JMPR") {
            assembleJmpR(ss);
        }
        else if (op == "JMPC") {
            assembleJmpC(ss);
        }
        else if (op == "JMPI") {
            assembleJmpI(ss);
        }
        else if (op == "RET") {
            assembleRet(ss);
        }
        else if (op == "JUMP") {
            assembleJump(ss);
        }
        else if (op == "CALL") {
            assembleCall(ss);
        }
        else if (op == "MOVRR") {
            assembleMovRR(ss);
        }
        else if (op == "HALT") {
            encode16(0x00, 0, 0, 0);
        }

        if (op == "MOV" || op == "JUMP" || op == "CALL" || op == "JMPC") {
            currentAddress += 2;
        }
        else {
            currentAddress += 1;
        }
    }
}

void Assembler::encode16(uint8_t op, uint8_t r1, uint8_t r2, uint8_t r3) {
    uint16_t word = (op << 9) | (r1 << 6) | (r2 << 3) | r3;
    if (Config::verbose) {

        std::cout << "DEBUG: encode16: word=0x" << std::hex << word << std::dec << std::endl;
    }
    output.push_back(word);
}

void Assembler::encode32(uint8_t op, uint8_t r1, uint8_t r2, uint8_t r3, uint16_t addr) {
    uint16_t word = (op << 9) | (r1 << 6) | (r2 << 3) | r3;
    if (Config::verbose) {

        std::cout << "DEBUG: encode32: op=0x" << std::hex << (int)op
            << " r1=" << (int)r1 << " r2=" << (int)r2 << " r3=" << (int)r3
            << " word=0x" << word << ", addr=0x" << addr << std::dec << std::endl;
    }
    output.push_back(word);
    output.push_back(addr);
}

uint16_t Assembler::resolveLabel(const std::string& label, uint16_t currentAddr) {
    if (labels.find(label) != labels.end()) {
        uint16_t addr = labels[label] + startAddress;
        if (Config::verbose) {

            std::cout << "DEBUG: resolveLabel '" << label << "' = 0x" << std::hex << addr << std::dec << std::endl;
        }
        return addr;
    }
    return std::stoi(label, nullptr, 0);
}

void Assembler::assembleMOV(std::stringstream& ss) {
    std::string line = ss.str();

    // Удаляем комментарий
    size_t commentPos = line.find(';');
    if (commentPos != std::string::npos) {
        line = line.substr(0, commentPos);
    }

    // Находим операнды
    size_t opPos = line.find("MOV");
    if (opPos == std::string::npos) return;

    std::string operands = line.substr(opPos + 3);
    operands.erase(0, operands.find_first_not_of(" \t"));

    // Разбиваем по запятым
    std::vector<std::string> parts;
    std::stringstream ss_operands(operands);
    std::string part;

    while (std::getline(ss_operands, part, ',')) {
        part.erase(0, part.find_first_not_of(" \t"));
        part.erase(part.find_last_not_of(" \t") + 1);
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    if (parts.size() < 2) {
        std::cerr << "Ошибка: недостаточно операндов для MOV" << std::endl;
        return;
    }

    std::string dest = parts[0];
    std::string src = parts[1];

    // ===== MOV [addr], #value =====
    if (dest[0] == '[' && dest.back() == ']' && src[0] == '#') {
        std::string addrStr = dest.substr(1, dest.length() - 2);
        uint16_t addr = std::stoi(addrStr, nullptr, 0);

        std::string numStr = src.substr(1);
        int32_t value = parseNumber(numStr);
        uint16_t val16 = static_cast<uint16_t>(value & 0xFFFF);

        // Загружаем константу в R7 и сохраняем в память
        encode32(0x40, 7, 0x02, 0, val16);
        encode32(0x40, 7, 0x01, 0, addr);
        std::cout << "MOV [0x" << std::hex << addr << "], #" << std::dec << value << std::endl;
        return;
    }

    // ===== MOV R1, #value (целое, дробное или метка) =====
    if (src[0] == '#') {
        if (Config::verbose) {

            std::cout << "DEBUG: assembleMOV: src='" << src << "'" << std::endl;
        }
        uint8_t rd = registers[dest];
        std::string numStr = src.substr(1);

        // Очистка от пробелов и запятых
        numStr.erase(std::remove(numStr.begin(), numStr.end(), ' '), numStr.end());
        numStr.erase(std::remove(numStr.begin(), numStr.end(), ','), numStr.end());
        numStr.erase(std::remove(numStr.begin(), numStr.end(), '\t'), numStr.end());

        // Проверяем, является ли это дробным числом (содержит точку)
        bool isFloat = (numStr.find('.') != std::string::npos);

        if (isFloat) {
            if (Config::verbose) {

                std::cout << "DEBUG: isFloat=true, numStr='" << numStr << "'" << std::endl;
            }
            // Дробное число - загружаем как 32-битное float
            uint32_t bits = parseFloatFromString(numStr);
            uint16_t hi = (bits >> 16) & 0xFFFF;
            uint16_t lo = bits & 0xFFFF;

            encode32(0x40, rd, 0x04, 0, hi);  // MOVH
            if (Config::verbose) {

                std::cout << "DEBUG: MOVH type=0x04, rd=" << (int)rd << ", hi=0x" << std::hex << hi << std::dec << std::endl;
            }

            encode32(0x40, rd, 0x02, 0x01, lo);  // MOVL с флагом
            if (Config::verbose) {

                std::cout << "DEBUG: MOVL type=0x08, rd=" << (int)rd << ", lo=0x" << std::hex << lo << std::dec << std::endl;
            }

            float fvalue;
            memcpy(&fvalue, &bits, 4);
            
            if (Config::verbose) {

                std::cout << "MOVF R" << (int)rd << ", #" << fvalue << " (0x" << std::hex << bits << ")" << std::dec << std::endl;
            }
            
            return;
        }

        // Не дробное - проверяем метку или целое число
        int32_t value;

        // Проверяем, является ли это меткой
        if (labels.find(numStr) != labels.end()) {
            value = labels[numStr] + startAddress;
            if (Config::verbose) {

                std::cout << "DEBUG: MOV label '" << numStr << "' = 0x" << std::hex << value << std::dec << std::endl;
            }
        }
        else {
            value = parseNumber(numStr);
        }

        // Проверяем, влезает ли в 16 бит (со знаком)
        if (value < -32768 || value > 65535) {
            // 32-битное значение
            uint16_t hi = (value >> 16) & 0xFFFF;
            uint16_t lo = value & 0xFFFF;

            encode32(0x40, rd, 0x04, 0, hi);  // MOVH
            if (Config::verbose) {

                std::cout << "DEBUG: MOVH type=0x04, rd=" << (int)rd << ", hi=0x" << std::hex << hi << std::dec << std::endl;
            }

            encode32(0x40, rd, 0x02, 0x01, lo);  // MOVL с флагом
            if (Config::verbose) {

                std::cout << "DEBUG: MOVL type=0x08, rd=" << (int)rd << ", lo=0x" << std::hex << lo << std::dec << std::endl;
            }

            if (Config::verbose) {

                std::cout << "MOV32 R" << (int)rd << ", #0x" << std::hex << value << std::dec << std::endl;
            }
        }
        else {
            // 16-битное значение - обычный MOV
            uint16_t val16 = static_cast<uint16_t>(value & 0xFFFF);
            encode32(0x40, rd, 0x02, 0, val16);
            if (Config::verbose) {

                std::cout << "MOV R" << (int)rd << ", #" << std::dec << value
                    << " (0x" << std::hex << value << ")" << std::dec << std::endl;
            }
        }
        return;
    }
    // ===== MOV [R1], R2 (косвенная адресация) =====
    if (dest[0] == '[' && dest.back() == ']') {
        std::string inner = dest.substr(1, dest.length() - 2);

        // Проверяем, является ли inner регистром
        if (registers.find(inner) != registers.end()) {
            // Косвенная адресация: MOV [R1], R2
            uint8_t rbase = registers[inner];
            uint8_t rs = registers[src];
            encode32(0x40, rs, 0x03, rbase, 0);  // type=0x03, addr=0
            if (Config::verbose) {

                std::cout << "MOV [R" << (int)rbase << "], R" << (int)rs << std::endl;
            }
            return;
        }

        // Иначе - прямая адресация: MOV [addr], R1
        uint16_t addr = std::stoi(inner, nullptr, 0);
        uint8_t rs = registers[src];
        encode32(0x40, rs, 0x01, 0, addr);
        if (Config::verbose) {

            std::cout << "MOV [0x" << std::hex << addr << "], R" << (int)rs << std::dec << std::endl;
        }
        return;
    }

    // ===== MOV [addr], R1 =====
    if (dest[0] == '[' && dest.back() == ']') {
        std::string addrStr = dest.substr(1, dest.length() - 2);
        uint16_t addr = std::stoi(addrStr, nullptr, 0);
        uint8_t rs = registers[src];
        encode32(0x40, rs, 0x01, 0, addr);
        if (Config::verbose) {

            std::cout << "MOV [0x" << std::hex << addr << "], R" << (int)rs << std::dec << std::endl;
        }
        return;
    }

    // ===== MOV R1, [addr] =====
    if (src[0] == '[' && src.back() == ']') {
        uint8_t rd = registers[dest];
        std::string inner = src.substr(1, src.length() - 2);

        // Проверяем, является ли inner регистром
        if (registers.find(inner) != registers.end()) {
            // Косвенное чтение: MOV R1, [R2]
            uint8_t rbase = registers[inner];
            encode32(0x40, rd, 0x07, rbase, 0);  // type=0x07, addr=0
            if (Config::verbose) {

                std::cout << "MOV R" << (int)rd << ", [R" << (int)rbase << "]" << std::endl;
            }
            return;
        }

        // Иначе - прямая адресация: MOV R1, [addr]
        uint16_t addr = std::stoi(inner, nullptr, 0);
        encode32(0x40, rd, 0x05, 0, addr);
        if (Config::verbose) {

            std::cout << "MOV R" << (int)rd << ", [0x" << std::hex << addr << "]" << std::dec << std::endl;
        }
        return;
    }

    // ===== MOV R1, R2 =====
    if (registers.find(src) != registers.end()) {
        uint8_t rd = registers[dest];
        uint8_t rs = registers[src];
        encode16(0x20, rd, 0, rs);
        if (Config::verbose) {

            std::cout << "MOV R" << (int)rd << ", R" << (int)rs << std::endl;
        }
        return;
    }

    // ===== MOV R1, R2+offset =====
    size_t plusPos = src.find('+');
    if (plusPos != std::string::npos) {
        uint8_t rd = registers[dest];
        std::string regStr = src.substr(0, plusPos);
        std::string offsetStr = src.substr(plusPos + 1);

        regStr.erase(std::remove(regStr.begin(), regStr.end(), ' '), regStr.end());
        offsetStr.erase(std::remove(offsetStr.begin(), offsetStr.end(), ' '), offsetStr.end());

        uint8_t rbase = registers[regStr];
        uint16_t offset = std::stoi(offsetStr, nullptr, 0);

        encode32(0x40, rd, 0x06, rbase, offset);
        if (Config::verbose) {

            std::cout << "MOV R" << (int)rd << ", R" << (int)rbase << "+0x" << std::hex << offset << std::dec << std::endl;
        }
        return;
    }

    std::cerr << "Ошибка: неизвестный формат MOV: " << dest << ", " << src << std::endl;
}

// Вспомогательная функция для парсинга чисел
int32_t Assembler::parseNumber(const std::string& str) {
    if (str.empty()) return 0;

    // Символьная константа: 'A'
    if (str.length() == 3 && str[0] == '\'' && str[2] == '\'') {
        return (int32_t)str[1];
    }
    // Определяем систему счисления
    if (str.length() > 2 && str[0] == '0' && str[1] == 'x') {
        // Шестнадцатеричное

        return std::stoul(str.substr(2), nullptr, 16);
    }
    else if (str[0] == '-') {
        // Отрицательное десятичное
        return std::stoi(str, nullptr, 10);
    }
    else {
        // Десятичное или другое
        return std::stoi(str, nullptr, 0);
    }
}
void Assembler::assembleArithmetic(const std::string& op, std::stringstream& ss) {
    std::string line = ss.str();

    // Удаляем комментарии
    size_t commentPos = line.find(';');
    if (commentPos != std::string::npos) {
        line = line.substr(0, commentPos);
    }

    // Находим операнды (всё после кода операции)
    size_t opPos = line.find(op);
    if (opPos == std::string::npos) return;

    std::string operands = line.substr(opPos + op.length());

    // Удаляем пробелы в начале
    operands.erase(0, operands.find_first_not_of(" \t"));

    // Разбиваем по запятым
    std::vector<std::string> parts;
    std::stringstream ss_operands(operands);
    std::string part;

    while (std::getline(ss_operands, part, ',')) {
        // Удаляем пробелы
        part.erase(0, part.find_first_not_of(" \t"));
        part.erase(part.find_last_not_of(" \t") + 1);
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    if (parts.size() < 2) {
        std::cerr << "Ошибка: недостаточно операндов для " << op << std::endl;
        return;
    }

    uint8_t reg1 = registers[parts[0]];
    uint8_t opcode = opcodes16[op];

    // Обработка унарных операций (NOT)
    if (op == "NOT") {
        uint8_t reg2 = registers[parts[1]];
        encode16(opcode, reg1, reg2, 0);
        if (Config::verbose) {

            std::cout << "DEBUG: NOT R" << (int)reg1 << ", R" << (int)reg2 << std::endl;
        }
        return;
    }

    if (parts.size() == 2) {
        // Двухоперандная форма (если нужна)
        uint8_t reg2 = registers[parts[1]];
        encode16(opcode, reg1, reg2, 0);
        if (Config::verbose) {

            std::cout << "DEBUG: " << op << " R" << (int)reg1 << ", R" << (int)reg2 << std::endl;
        }
    }
    else if (parts.size() >= 3) {
        // Трёхоперандная форма
        uint8_t reg2 = registers[parts[1]];

        if (parts[2][0] == '#') {
            uint16_t value = std::stoi(parts[2].substr(1), nullptr, 0);
            encode32(0x40, 6, 0x02, 0, value);  // MOV R6, #value
            encode16(opcode, reg1, reg2, 6);    // OP R1, R2, R6
        }
        else {
            uint8_t reg3 = registers[parts[2]];
            encode16(opcode, reg1, reg2, reg3);
            if (Config::verbose) {

                std::cout << "DEBUG: " << op << " R" << (int)reg1 << ", R" << (int)reg2 << ", R" << (int)reg3 << std::endl;
            }
        }
    }
}

void Assembler::assembleCmp(std::stringstream& ss, bool isFloat) {
    std::string line = ss.str();

    size_t commentPos = line.find(';');
    if (commentPos != std::string::npos) {
        line = line.substr(0, commentPos);
    }

    // Определяем, какую команду ищем
    std::string opName = isFloat ? "CMPF" : "CMP";
    size_t opPos = line.find(opName);
    if (opPos == std::string::npos) return;

    std::string operands = line.substr(opPos + opName.length());
    operands.erase(0, operands.find_first_not_of(" \t"));

    std::vector<std::string> parts;
    std::stringstream ss_operands(operands);
    std::string part;

    while (std::getline(ss_operands, part, ',')) {
        part.erase(0, part.find_first_not_of(" \t"));
        part.erase(part.find_last_not_of(" \t") + 1);
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    if (parts.size() < 2) {
        std::cerr << "Ошибка: недостаточно операндов для " << opName << std::endl;
        return;
    }

    uint8_t reg1 = registers[parts[0]];
    uint8_t reg2 = registers[parts[1]];

    uint8_t opcode = isFloat ? 0x0B : 0x0A;  // CMPF = 0x0B, CMP = 0x0A
    encode16(opcode, reg1, reg2, 0);

    if (Config::verbose) {

        std::cout << "DEBUG: " << opName << " R" << (int)reg1 << ", R" << (int)reg2 << std::endl;
    }
}

// версия для обратной совместимости
void Assembler::assembleCmp(std::stringstream& ss) {
    assembleCmp(ss, false);
}

void Assembler::assembleIO(const std::string& op, std::stringstream& ss) {
    std::string operand;
    ss >> operand;

    // Если это символ в кавычках
    if (operand.length() == 3 && operand[0] == '\'' && operand[2] == '\'') {
        uint8_t charCode = operand[1];
        // Загружаем код символа в R6 и выводим
        encode32(0x40, 6, 0x02, 0, charCode);
        encode16(opcodes16[op], 6, 0, 0);
    }
    else {
        // Обычный регистр
        encode16(opcodes16[op], registers[operand], 0, 0);
    }
}

void Assembler::assembleJmpR(std::stringstream& ss) {
    std::string target;
    ss >> target;

    int16_t offset;
    if (target[0] == '+' || target[0] == '-' || isdigit(target[0])) {
        offset = std::stoi(target, nullptr, 0);
    }
    else {
        uint16_t targetAddr = resolveLabel(target, currentAddress);
        offset = targetAddr - (currentAddress + 1);
    }

    // Кодируем 9-битное смещение
    uint16_t offset9 = offset & 0x1FF;
    uint8_t r1 = (offset9 >> 6) & 0x07;
    uint8_t r2 = (offset9 >> 3) & 0x07;
    uint8_t r3 = offset9 & 0x07;

    encode16(0x13, r1, r2, r3);

    if (Config::verbose) {

        std::cout << "DEBUG: JMPR offset=" << offset << " (r1=" << (int)r1
            << " r2=" << (int)r2 << " r3=" << (int)r3 << ")" << std::endl;
    }
}
void Assembler::assembleJmpC(std::stringstream& ss) {
    std::string cond, target;
    ss >> cond >> target;
    cond.erase(std::remove(cond.begin(), cond.end(), ','), cond.end());

    // Принудительно выведите все условия
    if (Config::verbose) {
        std::cout << "DEBUG: conditions map:" << std::endl;
        for (auto& p : conditions) {
            std::cout << "  " << p.first << " = " << (int)p.second << std::endl;
        }
    }
    

    uint8_t condition = conditions[cond];
    if (Config::verbose) {

        std::cout << "DEBUG: JMPC cond='" << cond << "' -> " << (int)condition << std::endl;
    }
    uint16_t targetAddr = resolveLabel(target, currentAddress);

    // Для 32-битной команды JMPC: op=0x54, r2=condition, addr=offset
    // Смещение от следующей команды (JMPC занимает 2 слова)
    int16_t offset = targetAddr - (currentAddress + startAddress);

    // Кодируем как 32-битную команду
    encode32(0x54, 0, condition, 0, (uint16_t)offset);
    if (Config::verbose) {

        std::cout << "DEBUG: JMPC32 " << cond << " target=0x" << std::hex << targetAddr
            << " current=0x" << currentAddress
            << " next=0x" << (currentAddress + 2)
            << " offset=" << std::dec << offset << std::endl;
    }
}

void Assembler::assembleJmpI(std::stringstream& ss) {
    std::string r;
    ss >> r;

    encode16(0x15, registers[r], 0, 0);
}

void Assembler::assembleRet(std::stringstream& ss) {
    std::string r;
    ss >> r;

    // RET R1
    encode16(0x16, registers[r], 0, 0);
    if (Config::verbose) {

        std::cout << "DEBUG: RET " << r << std::endl;
    }
}

void Assembler::assembleJump(std::stringstream& ss) {
    std::string target;
    ss >> target;
    uint16_t addr = resolveLabel(target, currentAddress);
    encode32(0x41, 0, 0, 0, addr);
}

void Assembler::assembleCall(std::stringstream& ss) {
    std::string retReg, target;
    ss >> retReg >> target;

    retReg.erase(std::remove(retReg.begin(), retReg.end(), ','), retReg.end());
    target.erase(std::remove(target.begin(), target.end(), ','), target.end());

    uint8_t rd = registers[retReg];
    uint16_t addr = resolveLabel(target, currentAddress);
    if (Config::verbose) {

        std::cout << "DEBUG: CALL target='" << target << "' addr=0x" << std::hex << addr << std::dec << std::endl;
    }
    encode32(0x43, rd, 0, 0, addr);
    if (Config::verbose) {

        std::cout << "CALL R" << (int)rd << ", 0x" << std::hex << addr << std::dec << std::endl;
    }
}
void Assembler::assembleMovRR(std::stringstream& ss) {
    std::string dest, src;
    ss >> dest >> src;
    if (Config::verbose) {

        std::cout << "DEBUG: MOVRR dest=" << dest << ", src=" << src << std::endl;
    }

    uint8_t rd = registers[dest];
    uint8_t rs = registers[src];
    if (Config::verbose) {

        std::cout << "DEBUG: rd=" << (int)rd << ", rs=" << (int)rs << std::endl;
    }

    encode16(0x20, rd, 0, rs);
}
