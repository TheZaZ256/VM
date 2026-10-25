#include "GenerateCode.h"
#include <iomanip>


// Функция для создания тестового файла
void GenerateCode::createTestFile() {
    std::ofstream file("test_program.txt");

    // MOV R0, #42
    file << "80 10 00 2A\n";        // MOV R0, #42

    // MOV R1, R0 (регистр-регистр) - opcode=0x20
    // word = (0x20<<9) | (1<<6) | (0<<3) | 0 = 0x4000 | 0x40 = 0x4040
    file << "40 40\n";              // MOV R1, R0

    // OUT_INT R1
    file << "30 10\n";              // OUT_INT R1

    file << "00 00\n";              // HALT
   
    file.close();
}