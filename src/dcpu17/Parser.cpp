#include <sstream>

#include "Parser.hpp"
#include "utility.hpp"

namespace dcpu
{
    // Returns false if the next char is an end of line / end of stream.
    // If so, an exception is also set.
    bool Parser::checkNoEndOfLine()
    {
        if(isEndOfLine(m_is.peek()))
        {
            setException("Unexpected end of line");
            return false;
        }
        else if(m_is.eof())
        {
            setException("Unexpected end of stream");
            return false;
        }
        return true;
    }

    // SKips whitespace until reach non-whitespace or end of line.
    // Returns false if reached end of current line or end of stream.
    // If endOfLineNotExpected is true, an exception will be thrown if
    // the end of line is reached.
    bool Parser::skipWhiteSpace(bool endOfLineNotExpected)
    {
        char c = 0;
        while(!m_is.eof())
        {
            c = m_is.peek();
            if(isEndOfLine(c))
            {
                if(endOfLineNotExpected)
                    setException("Unexpected end of line");
                return false; // found end of line, stopping
            }
            else if(!isWhiteSpace(c))
                return true; // found non-whitespace, stopping
            m_is.get();
        }
        if(endOfLineNotExpected)
            setException("Unexpected end of stream");
        return false; // end of stream
    }

    // Same as skipWhiteSpace, but also skips empty lines.
    // Returns false if reached end of stream.
    bool Parser::skipWhiteSpaceAndEmptyLines()
    {
        while(!skipWhiteSpace(false))
        {
            if(m_is.eof())
                return false;
#ifdef DCPU_DEBUG
            std::cout << "Skipping line..." << std::endl;
#endif
            nextLine();
        }
        return true;
    }

    // Jumps to the beginning of the next line.
    // Returns false if end of stream.
    bool Parser::nextLine()
    {
        char c = m_is.get();
        while(!m_is.eof())
        {
            if(isEndOfLine(c))
            {
#ifdef WINDOWS // CR+LF
                m_is.get(); // skip the LF
#endif
                return true;
            }
            c = m_is.get();
        }
        return false;
    }

    // Parses a 16-bit integer using one of the two functions below
    bool Parser::parseU16(u16 & value)
    {
        if(m_is.eof())
        {
            setException("Expected numeric, got nothing");
            return false;
        }

        if(!isdigit(m_is.peek()))
        {
            setException("Expected numeric");
            return false;
        }

        char c = m_is.get(); // c is a digit
        if(isEndOfLine(m_is.peek()))
        {
            value = c - '0';
            return true;
        }

        if(m_is.getLastChar() == '0') // Not a base 10 form ?
        {
            c = m_is.get();
            if(!checkNoEndOfLine())
                return false;

            if(c == 'x')
            {
                // Hex value
                m_is.get(); // skip the 'x'
                if(!checkNoEndOfLine())
                    return false;
                return parseHexU16(value);
            }
            else if(isLitteral(c))
            {
                setException("Unrecognized litteral used in numeric");
                return false;
            }
            else
            {
                // assume this is just a 0
                value = 0;
                return true;
            }
        }

        return parseDecU16(value);
    }

    // Parses a 16-bit integer written in decimal form (123456789)
    bool Parser::parseDecU16(u16 & value)
    {
        if(!checkNoEndOfLine())
            return false;

        if(!isdigit(m_is.peek()))
        {
            std::stringstream ss;
            ss << "Expected digit, got '" << m_is.peek() << "'";
            setException(ss.str());
            return false;
        }

        u32 readValue = 0;
        char c;
        for(u8 d = 0; d < 5; d++)
        {
            c = m_is.get();
            if(!isdigit(c))
                break;
            readValue *= 10;
            readValue += c - '0';
        }

        if(readValue > 0xffff)
        {
            setException("16-bit invalid value, max is 0xffff (65535)");
            return false;
        }

        value = readValue;
        return true;
    }

    // Parses a 16-bit integer written in hexadecimal form (123456789abcdef)
    bool Parser::parseHexU16(u16 & value)
    {
        if(!checkNoEndOfLine())
            return false;

        if(!isHexDigit(m_is.peek()))
        {
            std::stringstream ss;
            ss << "Expected hex digit, got '" << m_is.peek() << "'";
            setException(ss.str());
            return false;
        }

        // Note: there is exactly 4 max digits,
        // no need to check max value after parsing.
        value = 0;
        char c;
        for(u8 d = 0; d < 4; d++)
        {
            c = m_is.get();
            if(!isHexDigit(c))
                break;
            value <<= 4;
            value += hexDigitToInt(c);
        }

        return true;
    }

    // Parses a string written between two sep characters.
    bool Parser::parseString(std::string & str, char sep)
    {
        if(!checkNoEndOfLine())
            return false;

        if(m_is.peek() != sep)
        {
            std::stringstream ss;
            ss << "Expected string start ('" << sep << "'), got " << m_is.peek();
            setException(ss.str());
            return false;
        }

        // Skip first separator
        m_is.get();

        char c = 0;
        while(!m_is.eof())
        {
            if(!checkNoEndOfLine())
                return false;
            c = m_is.get();
            if(c == sep)
                break;
            str += c;
        }

        if(c != sep)
        {
            setException("Unexpected end of stream");
            return false;
        }

        return true;
    }

    // Parses a set of characters included in a-zA-Z_0-9.
    // Param what is the type of name (for debug).
    bool Parser::parseName(std::string & name, std::string what)
    {
        // The current pos must be vallid
        if(!checkNoEndOfLine())
            return false;

        // The first letter must be a litteral
        if(!isLitteral(m_is.peek()))
        {
            if(what.empty())
                what = "name";
            std::stringstream ss;
            ss << "The first letter of a" << (isVowel(what[0]) ? "n " : " ")
                << what << " must be a litteral, got '"
                << m_is.peek() << "' (" << (int)(m_is.peek()) << ")";
            setException(ss.str());
            return false;
        }

        char c = 0;
        while(!m_is.eof())
        {
            c = m_is.peek();
            if(!isLitteral(c) && !isdigit(c))
                break;
            name += c;
            m_is.get();
        }

        return true;
    }

    // Sets the exception message
    void Parser::setException(const std::string & msg)
    {
        std::stringstream ss;
        ss << msg << ", at line " << m_is.getRow() + 1 << ", col " << m_is.getCol() + 1;
        // TODO Parser: add support for line rewriting in error messages
        m_exceptionString = ss.str();
    }

} // namespace DCPU




