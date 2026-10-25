#include "Processor.h"
#include "Memory.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "InstructionDecoder.h"

Processor::Processor(Memory* mem)
    : psw(0), halted(false), connectedMemory(mem)
{
    reset();

    // 16-битные команды (opcode 0..63)
    commands16[0x01] = std::make_unique<AddIntCommand>();
    commands16[0x02] = std::make_unique<SubIntCommand>();
    commands16[0x03] = std::make_unique<MulIntCommand>();
    commands16[0x04] = std::make_unique<DivIntCommand>();
    commands16[0x05] = std::make_unique<ModIntCommand>();

    commands16[0x06] = std::make_unique<AddFloatCommand>();
    commands16[0x07] = std::make_unique<SubFloatCommand>();
    commands16[0x08] = std::make_unique<MulFloatCommand>();
    commands16[0x09] = std::make_unique<DivFloatCommand>();

    commands16[0x0A] = std::make_unique<CmpIntCommand>();
    commands16[0x0B] = std::make_unique<CmpFloatCommand>();

    commands16[0x0C] = std::make_unique<AndCommand>();
    commands16[0x0D] = std::make_unique<OrCommand>();
    commands16[0x0E] = std::make_unique<XorCommand>();
    commands16[0x0F] = std::make_unique<NotCommand>();
    commands16[0x10] = std::make_unique<ShlCommand>();
    commands16[0x11] = std::make_unique<ShrCommand>();
    commands16[0x12] = std::make_unique<SarCommand>();

    commands16[0x13] = std::make_unique<RelJumpCommand>();
    commands16[0x14] = std::make_unique<CondJumpCommand>();
    commands16[0x15] = std::make_unique<IndJumpCommand>();
    commands16[0x16] = std::make_unique<ReturnCommand>();

    commands16[0x17] = std::make_unique<InIntCommand>();
    commands16[0x18] = std::make_unique<OutIntCommand>();
    commands16[0x19] = std::make_unique<InFloatCommand>();
    commands16[0x1A] = std::make_unique<OutFloatCommand>();

    commands16[0x20] = std::make_unique<MovRegRegCommand>();

    // 32-битные команды (opcode 64..127)
    commands32[0x40] = std::make_unique<MoveCommand>();
    commands32[0x41] = std::make_unique<JumpDirectCommand>();
    commands32[0x42] = std::make_unique<JumpIndexCommand>();
    commands32[0x43] = std::make_unique<CallCommand>();
    commands32[0x54] = std::make_unique<CondJump32Command>();
}

Processor::~Processor() {}

void Processor::connectMemory(Memory& memory) {
    connectedMemory = &memory;
    std::cout << "Память подключена к процессору" << std::endl;
}

uint16_t Processor::fetch() const {
    if (!connectedMemory) {
        std::cerr << "Ошибка: память не подключена!" << std::endl;
        return 0;
    }
    return connectedMemory->read(getIP());
}

void Processor::store(uint16_t address, uint16_t value) {
    if (!connectedMemory) {
        std::cerr << "Ошибка: память не подключена!" << std::endl;
        return;
    }
    connectedMemory->write(address, value);
}

uint32_t Processor::getPSW() const { return psw; }
void Processor::setPSW(uint32_t newPsw) { psw = newPsw; }

uint16_t Processor::getIP() const {
    return static_cast<uint16_t>((psw >> 16) & 0xFFFF);
}

void Processor::setIP(uint16_t ip) {
    psw = (psw & 0x0000FFFF) | (static_cast<uint32_t>(ip) << 16);
}

uint16_t Processor::getFlags() const {
    return static_cast<uint16_t>(psw & 0xFFFF);
}

void Processor::setFlags(uint16_t flags) {
    psw = (psw & 0xFFFF0000) | flags;
}

bool Processor::getFlag(Flags flag) const {
    return (getFlags() & flag) != 0;
}

void Processor::setFlag(Flags flag, bool value) {
    uint16_t flags = getFlags();
    if (value) flags |= flag;
    else flags &= ~flag;
    setFlags(flags);
}

uint32_t Processor::getRegister(uint8_t reg) const {
    if (!isValidRegister(reg)) {
        throw std::out_of_range("Некорректный номер регистра");
    }
    return registers[reg];
}

void Processor::setRegister(uint8_t reg, uint32_t value) {
    if (!isValidRegister(reg)) {
        throw std::out_of_range("Некорректный номер регистра");
    }
    registers[reg] = value;
}

void Processor::reset() {
    psw = 0;
    registers.fill(0);
    halted = false;
    std::cout << "Процессор сброшен. IP = 0x" << std::hex << getIP() << std::endl;
}

bool Processor::isHalted() const {
    return halted || getFlag(FLAG_HALT);
}

void Processor::halt() {
    halted = true;
    setFlag(FLAG_HALT, true);
}

void Processor::resume() {
    halted = false;
    setFlag(FLAG_HALT, false);
}

bool Processor::isValidRegister(uint8_t reg) const {
    return reg < registers.size();
}

uint16_t Processor::fetchFromAddress(uint16_t address) const {
    if (!connectedMemory) return 0;
    return connectedMemory->read(address);
}

void Processor::incrementIP(uint16_t increment) {
    setIP(getIP() + increment);
}

void Processor::printConditionProcessor() {
    std::cout << "\n=== ЗАПУСК ВИРТУАЛЬНОЙ МАШИНЫ ===" << std::endl;
    std::cout << "Начальный IP установлен: 0x" << std::hex << getIP() << std::endl;
    std::cout << "\nСостояние процессора:" << std::endl;
    std::cout << "  IP = 0x" << std::hex << getIP() << " (адрес слова)" << std::endl;
    std::cout << "  Flags = 0x" << std::hex << getFlags() << std::endl;
    std::cout << "  Halted = " << isHalted() << std::endl;
    std::cout << "\n  Регистры:" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << "    R" << i << " = 0x" << std::setw(8) << std::setfill('0')
            << std::hex << registers[i] << std::setfill(' ') << std::endl;
    }
}

void Processor::advanceIP() {
    uint16_t ip = getIP();
    uint16_t cmdWord = connectedMemory->read(ip);
    uint8_t opcode = (cmdWord >> 9) & 0x7F;
    uint16_t increment = (opcode & 0x40) ? 2 : 1;
    setIP(ip + increment);
}

void Processor::runSteps() {
    while (!halted) {
        step();
    }
}

void Processor::executeCMD(uint16_t first_word, uint16_t second_word) {
    DecodedCommand cmd;
    InstructionDecoder::decodeCommandWord(first_word, cmd);

    if ((cmd.opcode & 0x40) != 0) {
        auto it = commands32.find(cmd.opcode);
        if (it != commands32.end()) {
            (*it->second)(*this, cmd, second_word);
        }
    }
    else {
        auto it = commands16.find(cmd.opcode);
        if (it != commands16.end()) {
            (*it->second)(*this, cmd, 0);
        }
    }
}

void Processor::run() {
    printConditionProcessor();

    if (!connectedMemory) {
        std::cerr << "ОШИБКА: Процессор не подключен к памяти!" << std::endl;
        return;
    }

    float f1 = 10.5f;
    float f2 = 3.5f;
    setRegister(0, *reinterpret_cast<uint32_t*>(&f1));
    setRegister(1, *reinterpret_cast<uint32_t*>(&f2));

    runSteps();
}

void Processor::step() {
    if (halted) return;

    uint16_t ip = getIP();
    uint16_t cmdWord = connectedMemory->read(ip);

    if (cmdWord == 0x0000) { halt(); return; }

    uint8_t opcode = (cmdWord >> 9) & 0x7F;
    bool is32bit = (opcode & 0x40) != 0;

    if (is32bit) {
        uint16_t second_word = connectedMemory->read(ip + 1);
        executeCMD(cmdWord, second_word);
        // НЕ УВЕЛИЧИВАЕМ IP, если команда сама его изменила (CALL, JUMP)
        // Проверяем, изменился ли IP
        if (getIP() == ip) {
            setIP(ip + 2);
        }
    }
    else {
        executeCMD(cmdWord, 0);
        if (getIP() == ip) {
            setIP(ip + 1);
        }
    }
}

void Processor::dumpRegisters() const {
    std::cout << "\n========== СОСТОЯНИЕ РЕГИСТРОВ ==========" << std::endl;
    for (int i = 0; i < 8; i++) {
        float f;
        memcpy(&f, &registers[i], 4);
        std::cout << "R" << i << " = " << std::dec << std::setw(12) << registers[i]
            << "  (0x" << std::hex << std::setw(8) << std::setfill('0') << registers[i]
            << ")  float: " << std::dec << f << std::endl;
    }
    std::cout << "IP = 0x" << std::hex << getIP() << std::dec << std::endl;
    std::cout << "Flags = 0x" << std::hex << getFlags() << std::dec << std::endl;
    std::cout << "=========================================" << std::endl;
}