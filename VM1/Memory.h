#pragma once
#include <cstdint>
#include <vector>

class Memory {
private:
    std::vector<uint16_t> memory;  // Память как вектор слов (16 бит)

    uint16_t loadedStart;
    uint16_t loadedEnd;
    bool hasLoadedData;

public:
    Memory();
    ~Memory();

    // ★★★★ Основные операции со словами ★★★★
    uint16_t read(uint16_t address) const;      // Чтение слова
    void write(uint16_t address, uint16_t value); // Запись слова

    // ★★★★ Вспомогательные методы ★★★★
    void readAndPrint(uint16_t address) const;  // Чтение с выводом
    void clear();                               // Очистка памяти

    // ★★★★ Проверка диапазонов ★★★★
    void verifyRange(uint16_t start_addr, uint16_t end_addr) const;
    void verifyRange(uint16_t start_addr) const;

    // ★★★★ Работа с блоками ★★★★
    std::vector<uint16_t> readBlock(uint16_t start_addr, uint16_t count) const;
    void writeBlock(uint16_t start_addr, const std::vector<uint16_t>& data);

    // ★★★★ Информация о памяти ★★★★
    uint16_t getSizeInWords() const;
    uint32_t getSizeInBytes() const;

    // ★★★★ Отладочные методы ★★★★
    void hexDump(uint16_t start_addr, uint16_t word_count) const;

    void setLoadedRange(uint16_t start, uint16_t end);
    void verifyLoadedRange() const;
    void clearLoadedRange();
    uint16_t getLoadedStart() const;
    uint16_t getLoadedEnd() const;
    bool hasLoadedData_() const;

private:
    // Вспомогательные методы
    //bool isZeroBlock(uint16_t start_addr, uint16_t count) const;
};