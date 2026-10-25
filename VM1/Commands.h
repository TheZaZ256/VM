#pragma once
#include <iostream>
#include "InstructionDecoder.h"  


class Processor;

class ICommand // Интерфейс
{
protected:
    ICommand() = default; // Защищенный конструктор

public:

    virtual ~ICommand() = default; //деструктор виртуальный

    virtual void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word = 0) = 0;
    //виртуальная перегруженная функция operator // для функторов

private:

};
// ============================================================================
// Целая арифметика (opcode 0x01-0x05)
// ============================================================================
class AddIntCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class SubIntCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class MulIntCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class DivIntCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class ModIntCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

// ============================================================================
// Дробная арифметика
// ============================================================================
class AddFloatCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class SubFloatCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class MulFloatCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class DivFloatCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

// ============================================================================
// Сравнения (opcode 0x0A-0x0B)
// ============================================================================
class CmpIntCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class CmpFloatCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

// ============================================================================
// Битовые операции (opcode 0x0C-0x12)
// ============================================================================
class AndCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class OrCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class XorCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class NotCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class ShlCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class ShrCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class SarCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

// ============================================================================
// Переходы (opcode 0x13-0x16)
// ============================================================================
class RelJumpCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class CondJumpCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class IndJumpCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class ReturnCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

// ============================================================================
// Ввод-вывод (opcode 0x17-0x1A)
// ============================================================================
class InIntCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class OutIntCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class InFloatCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class OutFloatCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

// ============================================================================
// 32-битные команды (opcode 0x40-0x43)
// ============================================================================
class MoveCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class JumpDirectCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class JumpIndexCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class CallCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class MovRegRegCommand : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};

class CondJump32Command : public ICommand {
public:
    void operator()(Processor& proc, const DecodedCommand& cmd, uint16_t second_word) override;
};
