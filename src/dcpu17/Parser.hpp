#ifndef PARSER_HPP_INCLUDED
#define PARSER_HPP_INCLUDED

#include <iostream>
#include <string>

#include "ParserStream.hpp"

namespace dcpu
{
    /*
        Character sets
    */

    // Defines whitespace characters
    inline bool isWhiteSpace(char c)
    {
        return c == ' ' || c == '\t';
    }

    // Defines litteral characters (used for names)
    inline bool isLitteral(char c)
    {
        return (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || c == '_';
    }

    /*
        Parser
    */

    class Parser
    {
    private :

        // Parsed stream
        ParserStream m_is;

        // Parsing error message (C++ exceptions are not used)
        std::string m_exceptionString;

    public :

        // Constructs a Parser.
        // is should be a binary stream, and not be modified externally
        // while the Parser is used.
        Parser(std::istream & is) : m_is(is)
        {}

        // Access to the stream
        ParserStream & stream() { return m_is; }

        // Getters
        bool isException() const { return !m_exceptionString.empty(); }
        const std::string & getExceptionString() const { return m_exceptionString; }

        // Returns false if the next char is an end of line / end of stream.
        // If so, an exception is also set.
        bool checkNoEndOfLine();

        // Each or the methods below advance m_is cursor,
        // return false if an error occurred and set the exceptionString if so.
        // The result of other parsing operations after an exception is undetermined.
        // Functions beginning by 'parse' expect the stream to be
        // positionned at the beginning of the expected sequence.

        // SKips whitespace until reach non-whitespace or end of line.
        // Returns false if reached end of current line.
        // If endOfLineNotExpected is true, an exception will be set if
        // the end of line is reached.
        bool skipWhiteSpace(bool endOfLineNotExpected);

        // Same as skipWhiteSpace, but also skips empty lines.
        // Returns false if reached end of stream.
        bool skipWhiteSpaceAndEmptyLines();

        // Jumps to the beginning of the next line.
        // Returns false if end of stream.
        bool nextLine();

        // Parses a 16-bit integer using one of the two functions below
        bool parseU16(u16 & value);

        // Parses a 16-bit integer written in decimal form (123456789)
        bool parseDecU16(u16 & value);

        // Parses a 16-bit integer written in hexadecimal form (123456789abcdef)
        bool parseHexU16(u16 & value);

        // Parses a string written between two sep characters.
        bool parseString(std::string & str, char sep = '"');

        // Parses a set of characters included in a-zA-Z_0-9.
        // Param what is the type of name (for debug).
        bool parseName(std::string & name, std::string what = "name");

    protected :

        // Sets the exception message
        void setException(const std::string & msg);

    };

} // namespace dcpu

#endif // PARSER_HPP_INCLUDED



