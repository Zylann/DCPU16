#ifndef DCPUASSEMBLER_HPP_INCLUDED
#define DCPUASSEMBLER_HPP_INCLUDED

#include <iostream>
#include <map>
#include <string>
#include <list>

#include "DCPU.hpp"

// SUGG make a preprocessor that handles #define/#include stuff?

namespace dcpu
{
    // Internal use
    struct LabelUse
    {
        std::string name;
        u16 addr;
        u16 row;
        u16 col;

        LabelUse() : addr(0), row(0), col(0)
        {}

        LabelUse(const std::string & name, u16 addr0, u16 row0, u16 col0)
        : name(name), addr(addr0), row(row0), col(col0)
        {}
    };

    /*
        Operand
    */

    struct Operand
    {
        u16 code;
        u16 value;
        u16 addedRegister;
        bool lookup;
        bool add;
        bool isLabel;
        bool isValue;
        bool isRegister;
        LabelUse labelUse;

        Operand()
            : code(0), value(0), addedRegister(0),
            lookup(false), add(false), isLabel(false), isValue(false), isRegister(false)
        {}
    };

    // Internal use
    struct Variable
    {
        u16 addr;
        bool isRegister;

        Variable() : addr(0), isRegister(false)
        {}

        Variable(u16 a, bool ir = false) : addr(a), isRegister(ir)
        {}
    };

    class Assembler
    {
    private :

        u16 m_ram[DCPU_RAM_SIZE];
        std::string m_line;
        u32 m_pos;
        u32 m_row;
        u32 m_addr;
        std::map<std::string, u16> m_basicOpcodes; // name, code
        std::map<std::string, u16> m_extendedOpcodes; // name, code
        std::map<std::string, Variable> m_variables; // name, var
        std::map<std::string, u16> m_labels; // name, address
        std::map<std::string, std::list<LabelUse> > m_labelUses; // name, uses
        std::string m_exceptionString;

    public :

        Assembler();

        // Resets the assembler and leaves it ready to process a new stream
        void init();

        // Assembles a program from a stream (usually a file stream).
        // Returns false if an error occurred. If so, the error message
        // should be stored in the exception string.
        bool assembleStream(std::istream & is);

        const u16 * getAssembly() const { return m_ram; }

        const std::string & getExceptionString() const
        { return m_exceptionString; }

        bool isException() const
        { return !m_exceptionString.empty(); }

    private :

        void setException(const std::string & msg);

        /* Parsing */

        // SKips whitespace like ' ' or '\t' (see isWhiteSpace(char c)).
        // Returns false if reached end of current line.
        bool skipWhiteSpace();

        // Parses the next operand for an operation
        bool parseNextOperand(Operand & op);

        // Parses the next operand without '[...]'
        bool parseNextOperandNoLookup(Operand & op);

        // Parses a set of characters included in a-zA-Z_0-9.
        // Param what is the type of name (for debug).
        bool parseName(std::string & name, std::string what = "name");

        // Parses an integer using one of the two functions below
        bool parseU16(u16 & value);

        // Parses an integer written in decimal form (123456789)
        bool parseDecU16(u16 & value);

        // Parses an integer written in hexadecimal form (0x123456789abcdef)
        bool parseHexU16(u16 & value);

        /* Assembling */

        // Keep track of a label use in order to write their adresses
        // later in the assembly (assembling is done in two passes)
        void addLabelUse(const LabelUse & lu);

        //--- Methods below return false if an exception occurred.

        // Computes the code of the given operand.
        // This code will be the one included into the assembled operation word.
        bool getOperandCode(const Operand & op, u16 & code, bool isB);

        // Parses and assembles one line of code
        bool assembleLine(const std::string & str);

        // Parses and assembles data values from code (DAT keyword)
        bool assembleData();

        // Parses and assembles a string from code.
        // A string is defined between quotes '"'.
        bool assembleString();

        // Assembles a basic operation
        bool assembleBasicOp(u16 opcode, Operand b, Operand a);

        // Assembles a non-basic operation
        bool assembleExtendedOp(u16 opcode, Operand a);

        // Assembles a full-word operand
        bool assembleFullWordOperand(const Operand & op);

        // Called after first assembling pass :
        // write label adresses where they are needed
        bool assembleLabels();
    };

} // namespace dcpu

#endif // DCPUASSEMBLER_HPP_INCLUDED
