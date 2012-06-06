#ifndef UTILITY_HPP_INCLUDED
#define UTILITY_HPP_INCLUDED

#include <cstdio>
#include <cstdlib>

#include "DCPU.hpp"

#ifndef DIR_CHAR
    #ifdef WINDOWS
        #define DIR_CHAR '\\'
    #else // Assumed POSIX
        #define DIR_CHAR '/'
    #endif
#endif

namespace dcpu
{
    /*
        For DCPU
    */

    // Loads, assembles and installs a DCPU program
    // Returns false if an error occurred.
    bool loadProgram(DCPU & cpu, const std::string & filename);

    // Dumps DCPU memory to a file
    // Returns false if an error occurred.
    bool dumpAsText(DCPU & cpu, const std::string & filename);

    // Converts an image to DASM "DAT" font code
    // Returns false if an error occurred.
    bool convertImageToDASMFont(
        const std::string & inputFilename,
        const std::string & outputFilename);

    // Performs a preprocessing pass on a file, then saves the result in another file.
    // Returns false if an error occurred.
    bool preprocessFile(
        const std::string & inputFilename,
        const std::string & outputFilename);

    /*
        General purpose
    */

    // Clears the console
    inline void clearConsole()
    {
        #ifdef WINDOWS
        std::system("CLS");
        #else
        // Assume POSIX
        std::system("clear");
        #endif
    }

    // Converts a 4-bit integer into its ASCII hexadecimal digit
    char u4ToHexChar(u8 n);

    // Converts a 16-bit integer into its ASCII hexadecimal digits
    void u16ToHexStr(u16 n, char str[4]);

    // Returns true if the given character is a hexadecimal digit
    inline bool isHexDigit(char c)
    {
        return isdigit(c) ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F');
    }

    // Converts an character representing a hexadecimal digit into an integer
    inline u8 hexDigitToInt(char c)
    {
        if(isdigit(c))
            return c - '0';
        else if(c >= 'a' && c <= 'f')
            return 10 + c - 'a';
        else if(c >= 'A' && c <= 'F')
            return 10 + c - 'A';
        else
            return 0;
    }

    // Uppercases a string
    inline void strToUpper(std::string & str)
    {
        for(u32 i = 0; i < str.size(); i++)
            str[i] = std::toupper(str[i]);
    }

    // Returns true if c is a vowel
    inline bool isVowel(char c)
    {
        return c == 'a' || c == 'e' || c == 'i' ||
                c == 'o' || c == 'u' || c == 'y';
    }

    // Interprets an unsigned number as a signed number
    inline s32 toSigned(u16 n)
    {
        return n & 0x8000 ? n - 0x10000 : n;
    }

} // namespace dcpu


#endif // UTILITY_HPP_INCLUDED
