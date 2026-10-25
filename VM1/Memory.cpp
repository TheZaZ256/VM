#include "Memory.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Config.h"

Memory::Memory() {
    // По ТЗ: память из слов по 16 бит
    // 16-битный адрес = 65536 возможных адресов слов
    memory.resize(65536, 0);  // Инициализируем всю память нулями
    loadedStart = 0;
    loadedEnd = 0;
    hasLoadedData = false;
}

Memory::~Memory() {
}

// ★★★★ Чтение СЛОВА (16 бит) ★★★★
uint16_t Memory::read(uint16_t address) const {
    // address - это адрес СЛОВА (0-65535)
    if (address >= memory.size()) {
        // Для безопасности, хотя память уже инициализирована
        return 0;
    }
    return memory[address];
}

// ★★★★ Запись СЛОВА (16 бит) ★★★★
void Memory::write(uint16_t address, uint16_t value) {
    // address - это адрес СЛОВА (0-65535)
    if (address >= memory.size()) {
        std::cerr << "ОШИБКА: Запись за границы памяти (адрес 0x"
            << std::hex << address << ")" << std::endl;
        return;
    }

    if (Config::verbose) {

        std::cout << "  Запись СЛОВА: адрес 0x" << std::hex << std::setw(4) << std::setfill('0')
            << address << " = 0x" << std::setw(4) << value << std::endl;
    }

    memory[address] = value;
}

// ★★★★ Чтение с выводом ★★★★
void Memory::readAndPrint(uint16_t address) const {
    uint16_t value = read(address);

    std::cout << "  Адрес 0x" << std::hex << std::setw(4) << std::setfill('0') << address
        << " = 0x" << std::setw(4) << value
        << " (" << std::dec << value << ")" << std::endl;
}

// ★★★★ Очистка памяти ★★★★
void Memory::clear() {
    // Заполняем всю память нулями
    std::fill(memory.begin(), memory.end(), 0);
    std::cout << "Память очищена (65536 слов по 16 бит = 128KB)" << std::endl;
}

// ★★★★ Простая проверка диапазона ★★★★
// Перегрузка 1: с указанием конечного адреса
void Memory::verifyRange(uint16_t start_addr, uint16_t end_addr) const
{
    std::cout << "\nПроверка памяти (слова) 0x" << std::hex << std::setw(4)
        << std::setfill('0') << start_addr
        << " - 0x" << std::setw(4) << end_addr << ":" << std::endl;

    for (uint16_t addr = start_addr; addr <= end_addr; addr++) {
        readAndPrint(addr);
    }
}

// ★★★★ Перегрузка 2: без указания конечного адреса ★★★★
void Memory::verifyRange(uint16_t start_addr) const
{
    std::cout << "\nПроверка памяти с 0x" << std::hex << std::setw(4)
        << std::setfill('0') << start_addr
        << " до первого нулевого слова:" << std::endl;

    uint16_t addr = start_addr;
    int consecutive_zeros = 0;
    const int ZERO_THRESHOLD = 4; // Сколько подряд нулевых слов считать концом

    while (addr < memory.size()) {
        readAndPrint(addr);

        uint16_t value = read(addr);

        // Считаем подряд идущие нулевые слова
        if (value == 0x0000) {
            consecutive_zeros++;
            if (consecutive_zeros >= ZERO_THRESHOLD) {
                std::cout << "... обнаружено " << ZERO_THRESHOLD
                    << " подряд нулевых слов, остановка" << std::endl;
                break;
            }
        }
        else {
            consecutive_zeros = 0;
        }

        addr++;

        // Защита от переполнения
        if (addr == 0) {
            std::cout << "Достигнут конец адресного пространства" << std::endl;
            break;
        }
    }
}

// ★★★★ Метод для чтения последовательности слов ★★★★
std::vector<uint16_t> Memory::readBlock(uint16_t start_addr, uint16_t count) const
{
    std::vector<uint16_t> result;

    for (uint16_t i = 0; i < count; i++) {
        uint16_t addr = start_addr + i;
        if (addr >= memory.size()) {
            break;
        }
        result.push_back(read(addr));
    }

    return result;
}

// ★★★★ Метод для записи блока слов ★★★★
void Memory::writeBlock(uint16_t start_addr, const std::vector<uint16_t>& data)
{
    for (size_t i = 0; i < data.size(); i++) {
        uint16_t addr = start_addr + static_cast<uint16_t>(i);
        if (addr < memory.size()) {
            write(addr, data[i]);
        }
    }
}

// ★★★★ Получение размера памяти в словах ★★★★
uint16_t Memory::getSizeInWords() const
{
    return static_cast<uint16_t>(memory.size());
}

// ★★★★ Получение размера памяти в байтах ★★★★
uint32_t Memory::getSizeInBytes() const
{
    return static_cast<uint32_t>(memory.size()) * 2;
}

// ★★★★ Дамп памяти в шестнадцатеричном виде ★★★★
void Memory::hexDump(uint16_t start_addr, uint16_t word_count) const
{
    std::cout << "\nДамп памяти (шестнадцатеричный):" << std::endl;

    for (uint16_t i = 0; i < word_count; i += 8) {
        // Адрес строки
        std::cout << std::hex << std::setw(4) << std::setfill('0')
            << (start_addr + i) << ": ";

        // 8 слов в строке
        for (uint16_t j = 0; j < 8 && (i + j) < word_count; j++) {
            uint16_t addr = start_addr + i + j;
            if (addr < memory.size()) {
                std::cout << std::setw(4) << read(addr) << " ";
            }
            else {
                std::cout << "???? ";
            }
        }
        std::cout << std::endl;
    }
}

void Memory::setLoadedRange(uint16_t start, uint16_t end) {
    loadedStart = start;
    loadedEnd = end;
    hasLoadedData = true;
    std::cout << "Установлен диапазон загруженной памяти: 0x" << std::hex << start
        << " - 0x" << end << std::dec << std::endl;
}

void Memory::verifyLoadedRange() const {
    if (!hasLoadedData) {
        std::cout << "Нет загруженных данных в памяти" << std::endl;
        return;
    }

    std::cout << "\n=== ПРОВЕРКА ЗАГРУЖЕННОЙ ОБЛАСТИ ===" << std::endl;
    std::cout << "Диапазон: 0x" << std::hex << loadedStart
        << " - 0x" << loadedEnd << std::dec << std::endl;

    int nonZeroCount = 0;

    for (uint16_t addr = loadedStart; addr <= loadedEnd; addr++) {
        uint16_t value = read(addr);
        if (value != 0) {
            nonZeroCount++;
            std::cout << "  Адрес 0x" << std::hex << std::setw(4) << std::setfill('0') << addr
                << " = 0x" << std::setw(4) << value
                << " (" << std::dec << value << ")" << std::endl;
        }
    }

    std::cout << "\nЗагружено слов: " << nonZeroCount << std::endl;
}

void Memory::clearLoadedRange() {
    hasLoadedData = false;
    loadedStart = 0;
    loadedEnd = 0;
}

uint16_t Memory::getLoadedStart() const {
    return loadedStart;
}

uint16_t Memory::getLoadedEnd() const {
    return loadedEnd;
}

bool Memory::hasLoadedData_() const {
    return hasLoadedData;
}