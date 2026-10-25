#include "Commands.h"
#include "Processor.h"
#include <iostream>
#include <cstring>
#include <cmath>


// Вспомогательная функция для установки флагов
static void setFlags(Processor& proc, uint32_t result) {
    uint16_t flags = proc.getFlags();

    // Zero flag (бит 0)
    if (result == 0)
        flags |= 0x0001;
    else
        flags &= ~0x0001;

    // Negative flag (бит 1) - для знаковых чисел
    if (result & 0x80000000)
        flags |= 0x0002;
    else
        flags &= ~0x0002;

    proc.setFlags(flags);
}

// ============================================================================
// ADD_INT: R1 = R2 + R3
// ============================================================================
void AddIntCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op1 = proc.getRegister(cmd.r2);
    uint32_t op2 = proc.getRegister(cmd.r3);
    uint32_t result = op1 + op2;

    proc.setRegister(cmd.r1, result);
    setFlags(proc, result);

}

// ============================================================================
// SUB_INT: R1 = R2 - R3
// ============================================================================
void SubIntCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op1 = proc.getRegister(cmd.r2);
    uint32_t op2 = proc.getRegister(cmd.r3);
    uint32_t result = op1 - op2;

    proc.setRegister(cmd.r1, result);
    setFlags(proc, result);

}

// ============================================================================
// MUL_INT: R1 = R2 * R3
// ============================================================================
void MulIntCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op1 = proc.getRegister(cmd.r2);
    uint32_t op2 = proc.getRegister(cmd.r3);
    uint32_t result = op1 * op2;

    proc.setRegister(cmd.r1, result);
    setFlags(proc, result);

}

// ============================================================================
// DIV_INT: R1 = R2 / R3
// ============================================================================
void DivIntCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t divisor = proc.getRegister(cmd.r3);

    if (divisor == 0) {
        std::cerr << "Ошибка: деление на ноль!" << std::endl;
        return;
    }

    uint32_t op1 = proc.getRegister(cmd.r2);
    uint32_t result = op1 / divisor;

    proc.setRegister(cmd.r1, result);
    setFlags(proc, result);

}

// ============================================================================
// MOD_INT: R1 = R2 % R3
// ============================================================================
void ModIntCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t divisor = proc.getRegister(cmd.r3);

    if (divisor == 0) {
        std::cerr << "Ошибка: деление на ноль!" << std::endl;
        return;
    }

    uint32_t op1 = proc.getRegister(cmd.r2);
    uint32_t result = op1 % divisor;

    proc.setRegister(cmd.r1, result);
    setFlags(proc, result);

}

// ============================================================================
// Вспомогательная функция для конвертации float <-> uint32_t
// ============================================================================
static uint32_t floatToUint32(float value) {
    uint32_t result;
    memcpy(&result, &value, 4);
    return result;
}

static float uint32ToFloat(uint32_t value) {
    float result;
    memcpy(&result, &value, 4);
    return result;
}


// ============================================================================
// Дробная арифметика 
// ============================================================================

void AddFloatCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    // Читаем значения из регистров как float
    float op1 = uint32ToFloat(proc.getRegister(cmd.r2));
    float op2 = uint32ToFloat(proc.getRegister(cmd.r3));

    // Выполняем сложение
    float result = op1 + op2;

    // Записываем результат обратно в регистр
    proc.setRegister(cmd.r1, floatToUint32(result));

    // Устанавливаем флаги (для float своя логика)
    uint16_t flags = proc.getFlags();
    if (result == 0.0f) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result < 0) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

void SubFloatCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    float op1 = uint32ToFloat(proc.getRegister(cmd.r2));
    float op2 = uint32ToFloat(proc.getRegister(cmd.r3));
    float result = op1 - op2;

    proc.setRegister(cmd.r1, floatToUint32(result));

    uint16_t flags = proc.getFlags();
    if (result == 0.0f) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result < 0) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

void MulFloatCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    float op1 = uint32ToFloat(proc.getRegister(cmd.r2));
    float op2 = uint32ToFloat(proc.getRegister(cmd.r3));
    float result = op1 * op2;

    proc.setRegister(cmd.r1, floatToUint32(result));

    uint16_t flags = proc.getFlags();
    if (result == 0.0f) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result < 0) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

void DivFloatCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    float op3 = uint32ToFloat(proc.getRegister(cmd.r3));

    if (op3 == 0.0f) {
        std::cerr << "Ошибка: деление на ноль в DIV_FLOAT!" << std::endl;
        return;
    }

    float op2 = uint32ToFloat(proc.getRegister(cmd.r2));
    float result = op2 / op3;

    proc.setRegister(cmd.r1, floatToUint32(result));

    uint16_t flags = proc.getFlags();
    if (result == 0.0f) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result < 0) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

// ============================================================================
// Сравнения
// ============================================================================

void CmpIntCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    int32_t op1 = static_cast<int32_t>(proc.getRegister(cmd.r1));
    int32_t op2 = static_cast<int32_t>(proc.getRegister(cmd.r2));

    uint16_t flags = proc.getFlags();

    // Zero flag (равно)
    if (op1 == op2) flags |= 0x0001;
    else flags &= ~0x0001;

    // Negative flag (меньше)
    if (op1 < op2) flags |= 0x0002;
    else flags &= ~0x0002;

    // Less flag (для COND_JUMP LT)
    if (op1 < op2) flags |= 0x0004;
    else flags &= ~0x0004;

    // Greater flag (для COND_JUMP GT)
    if (op1 > op2) flags |= 0x0008;
    else flags &= ~0x0008;

    proc.setFlags(flags);

}

void CmpFloatCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    float op1 = uint32ToFloat(proc.getRegister(cmd.r1));
    float op2 = uint32ToFloat(proc.getRegister(cmd.r2));

    uint16_t flags = proc.getFlags();

    if (op1 == op2) flags |= 0x0001;
    else flags &= ~0x0001;

    if (op1 < op2) flags |= 0x0002;
    else flags &= ~0x0002;

    if (op1 < op2) flags |= 0x0004;
    else flags &= ~0x0004;

    if (op1 > op2) flags |= 0x0008;
    else flags &= ~0x0008;

    proc.setFlags(flags);

}


// ============================================================================
// Битовые операции
// ============================================================================

void AndCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op2 = proc.getRegister(cmd.r2);
    uint32_t op3 = proc.getRegister(cmd.r3);
    uint32_t result = op2 & op3;

    proc.setRegister(cmd.r1, result);

    uint16_t flags = proc.getFlags();
    if (result == 0) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result & 0x80000000) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

void OrCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op2 = proc.getRegister(cmd.r2);
    uint32_t op3 = proc.getRegister(cmd.r3);
    uint32_t result = op2 | op3;

    proc.setRegister(cmd.r1, result);

    uint16_t flags = proc.getFlags();
    if (result == 0) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result & 0x80000000) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

void XorCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op2 = proc.getRegister(cmd.r2);
    uint32_t op3 = proc.getRegister(cmd.r3);
    uint32_t result = op2 ^ op3;

    proc.setRegister(cmd.r1, result);

    uint16_t flags = proc.getFlags();
    if (result == 0) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result & 0x80000000) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

void NotCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op2 = proc.getRegister(cmd.r2);
    uint32_t result = ~op2;

    proc.setRegister(cmd.r1, result);

    uint16_t flags = proc.getFlags();
    if (result == 0) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result & 0x80000000) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

void ShlCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op2 = proc.getRegister(cmd.r2);
    uint32_t shift = proc.getRegister(cmd.r3) & 0x1F;
    uint32_t result = op2 << shift;

    proc.setRegister(cmd.r1, result);

    uint16_t flags = proc.getFlags();
    if (result == 0) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result & 0x80000000) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

void ShrCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t op1 = proc.getRegister(cmd.r2);
    uint32_t shift = proc.getRegister(cmd.r3) & 31;
    uint32_t result = op1 >> shift;

    proc.setRegister(cmd.r1, result);

    uint16_t flags = proc.getFlags();
    if (result == 0) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result & 0x80000000) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}
void SarCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    // SAR R1, R2, R3 -> R1 = R2 >> R3 (АРИФМЕТИЧЕСКИЙ сдвиг)

    // читаем как ЗНАКОВОЕ число!
    int32_t op2 = static_cast<int32_t>(proc.getRegister(cmd.r2));
    uint32_t shift = proc.getRegister(cmd.r3) & 0x1F;

    // Арифметический сдвиг вправо
    int32_t result = op2 >> shift;

    // Записываем результат (сохраняя битовое представление)
    proc.setRegister(cmd.r1, static_cast<uint32_t>(result));

    uint16_t flags = proc.getFlags();
    if (result == 0) flags |= 0x0001;
    else flags &= ~0x0001;
    if (result < 0) flags |= 0x0002;
    else flags &= ~0x0002;
    proc.setFlags(flags);

}

// ============================================================================
// Вспомогательная функция для извлечения смещения (9 бит со знаком)
// ============================================================================
static int16_t getOffset(uint8_t r1, uint8_t r2, uint8_t r3) {
    int16_t offset = (r1 << 6) | (r2 << 3) | r3;
    // Расширение знака для 9 бит
    if (offset & 0x100) {
        offset |= 0xFE00;
    }
    return offset;
}

// ============================================================================
// Переходы
// ============================================================================

void RelJumpCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    // Смещение закодировано в полях r1, r2, r3 (9 бит)
    int16_t offset = getOffset(cmd.r1, cmd.r2, cmd.r3);

    uint16_t currentIP = proc.getIP();
    uint16_t newIP = currentIP + offset;

    proc.setIP(newIP);
}

void CondJumpCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    // Условие в cmd.r2 (3 бита)
    uint8_t condition = cmd.r2 & 0x07;  // Маскируем младшие 3 бита

    // Смещение из r1 и r3 (9 бит)
    int16_t offset = ((cmd.r1 & 0x07) << 6) | (cmd.r3 & 0x3F);

    // Расширение знака для 9 бит
    if (offset & 0x100) {
        offset |= 0xFE00;
    }

    uint16_t flags = proc.getFlags();
    bool shouldJump = false;

    // Проверка условия
    switch (condition) {
    case 0: // EQ
        shouldJump = (flags & 0x0001) != 0;
        std::cout << "COND_JUMP EQ";
        break;
    case 1: // NE
        shouldJump = (flags & 0x0001) == 0;
        std::cout << "COND_JUMP NE";
        break;
    case 2: // LT
        shouldJump = (flags & 0x0004) != 0;
        std::cout << "COND_JUMP LT";
        break;
    case 3: // GT
        shouldJump = (flags & 0x0008) != 0;
        std::cout << "COND_JUMP GT";
        break;
    case 4: // LE
        shouldJump = ((flags & 0x0004) != 0) || ((flags & 0x0001) != 0);
        std::cout << "COND_JUMP LE";
        break;
    case 5: // GE
        shouldJump = ((flags & 0x0008) != 0) || ((flags & 0x0001) != 0);
        std::cout << "COND_JUMP GE";
        break;
    default:
        std::cout << "COND_JUMP (unknown condition " << (int)condition << ")";
        shouldJump = false;
        break;
    }

    if (shouldJump) {
        uint16_t currentIP = proc.getIP();
        uint16_t newIP = currentIP + offset;
        std::cout << " +" << offset << " (0x" << std::hex << currentIP << " -> 0x" << newIP << ")" << std::dec << std::endl;
        proc.setIP(newIP);
    }
    else {
        std::cout << " - не выполнен" << std::endl;
    }
}

void IndJumpCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    // Косвенный переход: IP = значение из регистра r1
    uint32_t target = proc.getRegister(cmd.r1);
    uint16_t newIP = static_cast<uint16_t>(target & 0xFFFF);

    proc.setIP(newIP);
}

void ReturnCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t target = proc.getRegister(cmd.r1);
    uint16_t newIP = static_cast<uint16_t>(target & 0xFFFF);

    proc.setIP(newIP);
}

// ============================================================================
// Ввод-вывод
// ============================================================================

void InIntCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    int32_t value;
    std::cout << "Введите целое число для R" << (int)cmd.r1 << ": ";
    std::cin >> value;

    proc.setRegister(cmd.r1, static_cast<uint32_t>(value));

}

void OutIntCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    int32_t value = static_cast<int32_t>(proc.getRegister(cmd.r1));
    std::cout << "OUT_INT R" << (int)cmd.r1 << " = " << std::dec << value << std::endl;
}

void InFloatCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    float value;
    std::cout << "Введите дробное число для R" << (int)cmd.r1 << ": ";
    std::cin >> value;

    proc.setRegister(cmd.r1, floatToUint32(value));

}

void OutFloatCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint32_t value = proc.getRegister(cmd.r1);
    float floatValue = uint32ToFloat(value);

    std::cout << "OUT_FLOAT R" << (int)cmd.r1 << " = " << floatValue << std::endl;
}

// ============================================================================
// 32-битные команды
// ============================================================================

// Типы пересылок (кодируются в поле cmd.r2)
// 0x01 - регистр->память, прямая адресация
// 0x03 - регистр->память, индексная адресация (адрес = r3 + константа)
// 0x05 - память->регистр, прямая адресация
// 0x07 - память->регистр, индексная адресация (адрес = r3 + константа)
// 0x02 - загрузка адреса в регистр, прямая
// 0x06 - загрузка адреса в регистр, индексная

void MoveCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint8_t type = cmd.r2;
    uint16_t addr = second_word;

    switch (type) {
    case 0x01: { // регистр->память, прямая
        uint16_t value = static_cast<uint16_t>(proc.getRegister(cmd.r1) & 0xFFFF);
        proc.store(addr, value);
        break;
    }
    case 0x03: { // регистр->память, индексная
        uint16_t eff_addr = static_cast<uint16_t>(proc.getRegister(cmd.r3) + addr);
        uint16_t value = static_cast<uint16_t>(proc.getRegister(cmd.r1) & 0xFFFF);
        proc.store(eff_addr, value);
        break;
    }
    case 0x04: { // MOVH: загрузка в старшие 16 бит
        uint32_t oldValue = proc.getRegister(cmd.r1);
        uint32_t newValue = (oldValue & 0x0000FFFF) | (addr << 16);
        proc.setRegister(cmd.r1, newValue);
        break;
    }

    case 0x05: { // память->регистр, прямая
        uint16_t value = proc.fetchFromAddress(addr);
        proc.setRegister(cmd.r1, value);
        break;
    }
    case 0x07: { // память->регистр, индексная
        uint16_t eff_addr = static_cast<uint16_t>(proc.getRegister(cmd.r3) + addr);
        uint16_t value = proc.fetchFromAddress(eff_addr);
        proc.setRegister(cmd.r1, value);
        break;
    }
    case 0x02: { // MOV или MOVL
        if (cmd.r3 == 0x01) {
            // MOVL
            uint32_t oldValue = proc.getRegister(cmd.r1);
            uint32_t newValue = (oldValue & 0xFFFF0000) | addr;
            proc.setRegister(cmd.r1, newValue);
        }
        else {
            // Обычный MOV
            uint32_t value;
            if (addr & 0x8000) {
                value = addr | 0xFFFF0000;
            }
            else {
                value = addr;
            }
            proc.setRegister(cmd.r1, value);
        }
        break;
    }
    case 0x06: { // загрузка адреса, индексная
        uint32_t eff_addr = proc.getRegister(cmd.r3) + addr;
        proc.setRegister(cmd.r1, eff_addr);
        break;
    }
    case 0x08: { // MOVL: загрузка в младшие 16 бит
        uint32_t oldValue = proc.getRegister(cmd.r1);
        uint32_t newValue = (oldValue & 0xFFFF0000) | addr;
        proc.setRegister(cmd.r1, newValue);
        break;
    }
    default:
        std::cerr << "Неизвестный тип пересылки: 0x" << std::hex << (int)type << std::dec << std::endl;
        break;
    }
}

void JumpDirectCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint16_t target = second_word;
    std::cout << "JUMP 0x" << std::hex << target << std::dec << std::endl;
    proc.setIP(target);
}

void JumpIndexCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint16_t target = static_cast<uint16_t>(proc.getRegister(cmd.r3) + second_word);
    proc.setIP(target);
}
void CallCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    // Сохраняем адрес возврата (следующая команда после CALL)
    // CALL занимает 2 слова, поэтому следующая команда = IP + 2
    uint16_t returnAddr = proc.getIP() + 2;
    proc.setRegister(cmd.r1, returnAddr);

    uint16_t target = second_word;

    proc.setIP(target);
}
void MovRegRegCommand::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    // r1 = приёмник, r3 = источник
    uint32_t value = proc.getRegister(cmd.r3);
    proc.setRegister(cmd.r1, value);

}

void CondJump32Command::operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) {
    uint8_t condition = cmd.r2 & 0x07;
    int16_t offset = (int16_t)second_word;

    uint16_t flags = proc.getFlags();
    bool shouldJump = false;

    switch (condition) {
    case 0: shouldJump = (flags & 0x0001) != 0; std::cout << "COND_JUMP32 EQ"; break;
    case 1: shouldJump = (flags & 0x0001) == 0; std::cout << "COND_JUMP32 NE"; break;
    case 2: shouldJump = (flags & 0x0004) != 0; std::cout << "COND_JUMP32 LT"; break;
    case 3: shouldJump = (flags & 0x0008) != 0; std::cout << "COND_JUMP32 GT"; break;
    case 4: shouldJump = ((flags & 0x0004) != 0) || ((flags & 0x0001) != 0); std::cout << "COND_JUMP32 LE"; break;
    case 5: shouldJump = ((flags & 0x0008) != 0) || ((flags & 0x0001) != 0); std::cout << "COND_JUMP32 GE"; break;
    default: shouldJump = false; break;
    }

    if (shouldJump) {
        uint16_t currentIP = proc.getIP();
        uint16_t newIP = currentIP + offset;
        std::cout << " +" << offset << " (0x" << std::hex << currentIP << " -> 0x" << newIP << ")" << std::dec << std::endl;
        proc.setIP(newIP);
    }
    else {
        std::cout << " - не выполнен" << std::endl;
    }
}
