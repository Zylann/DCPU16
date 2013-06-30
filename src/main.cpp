#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "dcpu17/Emulator.hpp"
#include "dcpu17/utility.hpp"

using namespace dcpu;

int main(int argc, char * argv[])
{
    std::cout << "Program begin" << std::endl;

    /* Handle command line arguments */

    std::string programFileName;// = "dasm/text_editor.dasm";
    if(argc == 2)
    {
        programFileName = argv[1];

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
    }
    else if(argc == 4)
    {
        std::string cmd = argv[1];
        if(cmd == "-cvf")
        {
            /* Convert image to DASM font */

            std::string inputImageFilename = argv[2];
            std::string outputFilename = argv[3];
            if(!convertImageToDASMFont(inputImageFilename, outputFilename))
                return -1;
        }
        else if(cmd == "-pp")
        {
            /* Preprocess a file */

            programFileName = argv[2];
            std::string outputFilename = argv[3];
            if(!preprocessFile(programFileName, outputFilename))
                return -1;
        }
        else
        {
            std::cout << "E: unrecognized command, or syntax error." << std::endl;
            return -1;
        }
    }
    else
    {
        /* Default: print help */

        // TODO main: print help
        std::cout << "E: bad arguments" << std::endl;
        return -1;
    }

    std::cout << "Program end" << std::endl;
    std::cout << "Press a key...";
    std::getchar();

    return 0;
}





