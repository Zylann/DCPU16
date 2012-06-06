#ifndef PREPROCESSOR_HPP_INCLUDED
#define PREPROCESSOR_HPP_INCLUDED

#include <iostream>
#include "Parser.hpp"

namespace dcpu
{
    /*
        Generic preprocessor.
        should be subclassed for adding more DASM-specific commands.
    */

    class Preprocessor : public Parser
    {
    private :

        std::ostream & r_os;
        std::streampos m_readCharsStartPos;

    public :

        Preprocessor(std::istream & is, std::ostream & os)
        : Parser(is), r_os(os)
        {
            m_readCharsStartPos = 0;
        }

        // Processes #commands from the input stream and
        // put the resulting stream in r_os.
        // Returns false if an error occurred.
        bool process();

    protected :

        // Processes a command.
        // Returns 0 if success.
        // Returns -1 if command not found.
        // Returns -2 if command error.
        virtual int processCommand(const std::string & cmd);

    private :

        void outputReadChars(std::streampos endPos);

        bool processInclude();
        bool processDefine();

    };

} // namespace dcpu

#endif // PREPROCESSOR_HPP_INCLUDED
