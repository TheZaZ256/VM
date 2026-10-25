#pragma once

#include <cstdint>
#include <array>
#include "InstructionDecoder.h"
#include <unordered_map>
#include "Commands.h"

class Memory;  // Предварительное объявление

// Определение флагов
enum Flags : uint16_t {
    FLAG_ZERO = 0x0001,  // Z - результат операции был ноль
    FLAG_CARRY = 0x0002,  // C - перенос/заем
    FLAG_OVERFLOW = 0x0004,  // V - переполнение
    FLAG_NEGATIVE = 0x0008,  // N - отрицательный результат
    FLAG_PARITY = 0x0010,  // P - четность
    FLAG_INTERRUPT = 0x0020,  // I - разрешение прерываний
    FLAG_TRAP = 0x0040,  // T - режим трассировки
    FLAG_ERROR = 0x0080,  // E - ошибка выполнения
    FLAG_HALT = 0x0100   // H - останов процессора
};

class IProcessor // Интерфейс
{
public:
    virtual ~IProcessor() = default;

    // Методы для работы с PSW
    virtual uint32_t getPSW() const = 0;
    virtual void setPSW(uint32_t psw) = 0;

    // Методы для работы с IP
    virtual uint16_t getIP() const = 0;
    virtual void setIP(uint16_t ip) = 0;

    // Методы для работы с флагами
    virtual uint16_t getFlags() const = 0;
    virtual void setFlags(uint16_t flags) = 0;

    virtual bool getFlag(Flags flag) const = 0;
    virtual void setFlag(Flags flag, bool value) = 0;

    // Методы для работы с регистрами
    virtual uint32_t getRegister(uint8_t reg) const = 0;
    virtual void setRegister(uint8_t reg, uint32_t value) = 0;

    // Управление процессором
    virtual void reset() = 0;
    virtual bool isHalted() const = 0;
    virtual void halt() = 0;
    virtual void resume() = 0;

    virtual void run() = 0;
    virtual void printConditionProcessor() = 0;


protected:
    IProcessor() = default;
};

// Реализация процессора
class Processor : public IProcessor
{
private:
    // PSW = 32 бита: [IP:16][Flags:16]
    uint32_t psw;

    // Регистры общего назначения (8 штук по 32 бита)
    std::array<uint32_t, 8> registers;

    // Состояние процессора
    bool halted;

    // Ссылка на память ВМ
    Memory* connectedMemory;

    //карта команд
    std::unordered_map<uint8_t, std::unique_ptr<ICommand>> commands16;
    std::unordered_map<uint8_t, std::unique_ptr<ICommand>> commands32;

public:
    Processor(Memory* mem = nullptr);
    ~Processor();

    // Реализация интерфейса IProcessor
    uint32_t getPSW() const override;
    void setPSW(uint32_t psw) override;

    uint16_t getIP() const override;
    void setIP(uint16_t ip) override;

    uint16_t getFlags() const override;
    void setFlags(uint16_t flags) override;
    bool getFlag(Flags flag) const override;
    void setFlag(Flags flag, bool value) override;

    uint32_t getRegister(uint8_t reg) const override;
    void setRegister(uint8_t reg, uint32_t value) override;

    void reset() override;
    bool isHalted() const override;
    void halt() override;
    void resume() override;

    void incrementIP(uint16_t increment);

    // Работа с памятью
    void connectMemory(Memory& memory);
    uint16_t fetch() const;
    void store(uint16_t address, uint16_t value);


    uint16_t fetchFromAddress(uint16_t address) const;


    void run() override;
    void printConditionProcessor() override;
    void step();
    void advanceIP();
    void runSteps();


    // Дамп состояния регистров
    void dumpRegisters() const;
    

private:
    // Валидация номера регистра
    bool isValidRegister(uint8_t reg) const;

    void executeCMD(uint16_t word, uint16_t second_word);
};
