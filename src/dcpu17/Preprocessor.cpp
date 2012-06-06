#include <fstream>
#include <sstream>

#include "Preprocessor.hpp"

namespace dcpu
{
    bool Preprocessor::process()
    {
        std::cout << "Preprocessing..." << std::endl;

        while(skipWhiteSpaceAndEmptyLines())
        {
            std::streampos posBeforeCommand = stream().tellg();

            if(stream().get() == '#')
            {
                std::string cmd;
                if(!parseName(cmd, "preprocessor command"))
                    return false;

                // Write previously read characters to output
                outputReadChars(posBeforeCommand);

                int res = processCommand(cmd);
                if(res < 0)
                {
                    if(res == -1)
                        setException("Unrecognized preprocessor command");
                    return false;
                }

                // Set cursor just after the command, then at the next
                // call to outputReadChar the command will not be written to
                // the output.
                m_readCharsStartPos = stream().tellg();
            }

            #if DCPU_DEBUG == 1
            std::cout << "Preprocessor: nextline..." << std::endl;
            #endif

            nextLine();
        }

        outputReadChars(stream().tellg());
        std::cout << "Preprocessing finished." << std::endl;
        return true;
    }

    int Preprocessor::processCommand(const std::string & cmd)
    {
        if(cmd == "include")
        {
            if(!processInclude())
                return -2;
        }
        else if(cmd == "define")
        {
            if(!processDefine())
                return -2;
        }
        else
            return -1;
        return 0;
    }

    void Preprocessor::outputReadChars(std::streampos endPos)
    {
        stream().getReadData(r_os, m_readCharsStartPos, endPos);
    }

    bool Preprocessor::processInclude()
    {
        #if DCPU_DEBUG == 1
        std::cout << "Processing #include..." << std::endl;
        #endif

        if(!skipWhiteSpace(true))
            return false;

        std::string filename;
        if(!parseString(filename, '"'))
            return false;

        std::ifstream ifs(filename.c_str(),
            std::ios::in|std::ios::binary);
        if(!ifs.good())
        {
            std::stringstream ss;
            ss << "#include \"" << filename << "\": " << "couldn't open file";
            setException(ss.str());
            ifs.close();
            return false;
        }

        // TODO Preprocessor: recursive #include command
        // Currently one-level
        // Write file content to output
        char c = 0;
        while(!ifs.eof())
        {
            c = ifs.get();
            if(!ifs.eof())
                r_os.put(c);
        }

        ifs.close();
        return true;
    }

    bool Preprocessor::processDefine()
    {
        #if DCPU_DEBUG == 1
        std::cout << "Found #define" << std::endl;
        #endif

        // TODO Preprocessor: processDefine
        setException("Not implemented yet");
        return false;
    }

} // namespace dcpu


