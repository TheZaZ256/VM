#include "ProgrammLoader.h"
#include <fstream>
#include <iostream>
#include <string>       
#include "InstructionDecoder.h"
#include "GenerateCode.h"
#include "processor.h"
#include "Assembler.h"
#include "Config.h"

// Внешние функции
extern void runVM(Memory& memory, uint16_t start_address); 

// ============================================================================
// Функция ассемблирования
// ============================================================================
void checkAndAssemble(const std::string& filename, uint16_t load_address) {
    if (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".asm") {
        std::cout << "Обнаружен .asm файл. Ассемблирую..." << std::endl;
        Assembler assembler;
        assembler.assemble(filename, load_address);
        assembler.saveToText("program.txt");
    }
}

// ============================================================================
// Чтение строк из файла
// ============================================================================
std::vector<std::string> ProgrammLoader::readCommandLines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл '" << filename << "'" << std::endl;
        return lines;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Удаляем комментарии (всё после ';')
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Удаляем пробелы в начале и конце
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    file.close();
    return lines;
}
// ============================================================================
// Обработка флагов командной строки (-v, --verbose)
// ============================================================================
int ProgrammLoader::processFlags(int argc, char* argv[]) {
    int newArgc = argc;

    for (int i = 1; i < newArgc; i++) {
        std::string arg = argv[i];

        if (arg == "--verbose" || arg == "-v") {
            Config::verbose = true;
            std::cout << "Режим verbose включен (отладочный вывод)\n" << std::endl;

            // Сдвигаем оставшиеся аргументы влево
            for (int j = i; j < newArgc - 1; j++) {
                argv[j] = argv[j + 1];
            }
            newArgc--;
            i--;  // Проверяем тот же индекс снова (там теперь новый аргумент)
        }
    }

    return newArgc;
}

// ============================================================================
// Загрузка команд в память
// ============================================================================
void ProgrammLoader::readLoadCommandsToMemory(Memory& memory) {
    std::vector<std::string> command_lines = readCommandLines(filename);

    if (command_lines.empty()) {
        std::cerr << "Загрузчик: файл пуст или не содержит команд" << std::endl;
        return;
    }

    std::cout << "Загрузчик: прочитано " << command_lines.size()
        << " команд из файла " << filename << std::endl;

    uint16_t current_addr = load_address;
    uint16_t first_addr = load_address;

    std::cout << "\n=== ЗАГРУЗКА КОМАНД ===\n" << std::endl;

    for (const auto& line : command_lines) {
        if (Config::verbose) {

            std::cout << "\nКоманда: " << line << std::endl;
        }

        DecodedCommand decoded = InstructionDecoder::decodeCommand(line);
        bool is32bit = InstructionDecoder::is32BitCommand(decoded.opcode);
        if (Config::verbose) {

            std::cout << "Тип: " << (is32bit ? "32-bit (2 слова)" : "16-bit (1 слово)") << std::endl;
        }

        InstructionDecoder::loadCommandToMemory(decoded, memory, current_addr, is32bit);
    }

    memory.setLoadedRange(first_addr, current_addr - 1);
}

// ============================================================================
// Обработка аргументов командной строки
// ============================================================================
bool ProgrammLoader::processArgs(int argc, char* argv[]) {
    // Сначала обрабатываем флаги
    int newArgc = processFlags(argc, argv);

    // ------------------------------------------------------------------------
    // Нет аргументов (или только флаги) - генерируем тестовый файл
    // ------------------------------------------------------------------------
    if (newArgc < 2) {
        std::cout << "Аргументы не указаны. Попытка открыть need.asm..." << std::endl;

        std::string filePath = "need.asm";
        std::ifstream file(filePath);

        // Если файл СУЩЕСТВУЕТ и успешно ОТКРЫЛСЯ
        if (file.is_open()) {
            std::cout << "Файл успешно открыт. Содержимое:" << std::endl;
            std::cout << "--------------------------------" << std::endl;

            std::string line;
            while (std::getline(file, line)) {
                std::cout << line << std::endl;
            }
            file.close();

            // Задаем имя считанного файла и адрес (если это необходимо для дальнейшей логики)
            filename = filePath;
            load_address = 0x1000;
            return true;
        }
        // Если файла need.txt НЕТ, управление переходит сюда (вызов генерации)
        else {
            std::cout << "Файл need.txt не найден. Запуск генерации..." << std::endl;

            std::ifstream test_file("test_program.txt");
            if (!test_file.is_open()) {
                std::cout << "Создаю тестовую программу test_program.txt..." << std::endl;
                GenerateCode generator;
                generator.createTestFile();
            }
            else {
                test_file.close();
            }

            filename = "test_program.txt";
            load_address = 0x1000;
            std::cout << "Используется тестовый файл: " << filename << std::endl;
            return true;
        }
    }

    // ------------------------------------------------------------------------
    // Есть аргументы - проверяем файл (сюда попадаем только если newArgc >= 2)
    // ------------------------------------------------------------------------
    filename = argv[1];

    std::ifstream test_file(filename);
    if (!test_file.is_open()) {
        std::cerr << "Ошибка: файл '" << filename << "' не найден!" << std::endl;
        return false;
    }
    test_file.close();

    // ------------------------------------------------------------------------
    // Адрес загрузки
    // ------------------------------------------------------------------------
    if (newArgc >= 3) {
        load_address = std::stoi(argv[2], nullptr, 0);
    }

    // ------------------------------------------------------------------------
    // Проверка расширения
    // ------------------------------------------------------------------------
    std::string ext;
    if (filename.length() >= 4) {
        ext = filename.substr(filename.length() - 4);
    }

    if (ext != ".asm" && ext != ".txt") {
        std::cerr << "Ошибка: неподдерживаемый формат файла '" << ext << "'" << std::endl;
        std::cerr << "Поддерживаются: .asm, .txt" << std::endl;
        return false;
    }

    std::cout << "Загрузка из файла: " << filename << std::endl;

    // ------------------------------------------------------------------------
    // Ассемблирование (если .asm)
    // ------------------------------------------------------------------------
    if (ext == ".asm") {
        std::cout << "Обнаружен .asm файл. Ассемблирую..." << std::endl;
        checkAndAssemble(filename, load_address);
        filename = "program.txt";
        std::cout << "Меняем на сгенерированный: " << filename << std::endl;
    }
    else {
        std::cout << "Обычный .txt файл (байт-код)" << std::endl;
    }

    return true;
}
// ============================================================================
// Загрузка и выполнение
// ============================================================================
void ProgrammLoader::loadAndRun() {
    Memory vm_memory;

    std::cout << "\n=== ЗАГРУЗКА ПРОГРАММЫ В ПАМЯТЬ ===\n" << std::endl;
    readLoadCommandsToMemory(vm_memory);

    std::cout << "\n=== ЗАПУСК ВИРТУАЛЬНОЙ МАШИНЫ ===\n" << std::endl;
    runVM(vm_memory, load_address);
}