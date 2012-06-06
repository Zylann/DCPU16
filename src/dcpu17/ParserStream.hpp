#ifndef PARSERSTREAM_HPP_INCLUDED
#define PARSERSTREAM_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include "common.hpp"

namespace dcpu
{
    // Defines end-of-line chars
    inline bool isEndOfLine(char c)
    {
        return c == '\n' || c == '\r';
    }

    /*
        This is a std::istream wrapper that can count line pos and number.
    */

    class ParserStream
    {
    private :

        std::istream & r_is; // Parsed stream
        char m_lastChar; // Last read char
        u32 m_col; // line pos
        u32 m_row; // line number

    public :

        // Constructs a ParserStream.
        // is should be a binary stream, and not be modified externally
        // while the ParserStream is used.
        ParserStream(std::istream & is) : r_is(is)
        {
            m_col = 0;
            m_row = 0;
            m_lastChar = 0;
        }

        /* Accessors */

        u32 getCol() const { return m_col; }
        u32 getRow() const { return m_row; }
        char getLastChar() const {return m_lastChar; }

        // Puts read characters into os, starting from startPos to endPos.
        // Does nothing if pos >= tellg().
        // if endPos > tellg(), it is changed to tellg().
        void getReadData(
            std::ostream & os,
            std::streampos startPos,
            std::streampos endPos);

        /* istream wrapper methods */

        // Same as istream::peek
        char peek() const { return r_is.peek(); }

        // Same as istream::get, also count newlines and line pos.
        char get();

        // Same as istream::eof
        bool eof() const { return r_is.eof(); }

        // Same ase istream::tellg
        std::streampos tellg() const { return r_is.tellg(); }

    };

} // namespace dcpu

#endif // PARSERSTREAM_HPP_INCLUDED



