#pragma once
#include <vector>
#include <string>
#include "Memory.h"


class ProgrammLoader {
public:
    // Обработка аргументов командной строки
    // Возвращает true, если всё OK
    bool processArgs(int argc, char* argv[]);

    // Загрузка и выполнение программы
    void loadAndRun();

private:
    std::string filename;
    uint16_t load_address = 0x1000;

    // Чтение строк из файла
    std::vector<std::string> readCommandLines(const std::string& filename);

    // Загрузка команд в память
    void readLoadCommandsToMemory(Memory& memory);

    int processFlags(int argc, char* argv[]);
};