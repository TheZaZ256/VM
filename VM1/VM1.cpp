#include <fstream> 
#include "Memory.h"
#include "processor.h"
#include "ProgrammLoader.h"
#include "assembler.h"  
#include "GenerateCode.h"


void runVM(Memory& memory, uint16_t start_address)
{
    memory.verifyLoadedRange();
    Processor processor(&memory);
    processor.setIP(start_address);
    processor.run();
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    ProgrammLoader loader;

    if (!loader.processArgs(argc, argv)) {
        std::cerr << "\nИспользование: " << argv[0]
            << " <файл.asm|файл.txt> [адрес_загрузки]" << std::endl;
        std::cerr << "Пример: " << argv[0] << " program.asm 0x1000" << std::endl;
        std::cerr << "        " << argv[0] << " program.txt" << std::endl;
        std::cerr << "        " << argv[0] << "                 (тестовый пример)" << std::endl;
        return 1;
    }

    loader.loadAndRun();

    return 0;
}