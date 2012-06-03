#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "dcpu17/Emulator.hpp"
#include "dcpu17/utility.hpp"

using namespace dcpu;

int main(int argc, char * argv[])
{
    /* Handle command line arguments */
    // TODO main: test input program filename
    std::string programFileName = "dasm/text_editor.dasm";
    if(argc == 2)
    {
        programFileName = argv[1];
    }
    else if(argc >= 2)
    {
        std::cout << "E: Too much arguments." << std::endl;
        return -1;
    }

    std::cout << "Program begin" << std::endl;

    /* Run emulator */

    Emulator emulator;

    if(!emulator.loadContent())
        return -1;

    if(emulator.loadProgram(programFileName))
    {
        emulator.dumpMemoryAsText("dump0.txt");
        emulator.run();
        emulator.dumpMemoryAsText("dump1.txt");
    }

    std::cout << "Program end" << std::endl;
    std::cout << "Press a key...";
    std::getchar();

    return 0;
}





