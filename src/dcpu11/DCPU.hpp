#ifndef DCPU11DCPU_HPP_INCLUDED
#define DCPU11DCPU_HPP_INCLUDED

/*
    16 bits CPU emulation inspired on the DCPU-16 1.1 specification
    (DCPU-16 has been created by Mojang for the game 0x10c).
    
    NOTE from <zylann_sagas_mp3@hotmail.fr> :
    	this is an outdated DCPU16 version, but I keep it for compatibility stuff.
    	I won't update it anymore, now I'm working on the current version.
*/

#include <string.h> // for memset
#include <iostream>

// Enable debug messages
// 1 : on
// 0 : off
#define DCPU_DEBUG 0

#define DCPU_REG_COUNT 8
#define DCPU_RAM_SIZE 65536

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

namespace dcpu11
{
    // TODO move to 1.7 specification (interruptions support, more opcodes etc)

    enum ValAddresses
    {
        // Registers
        AD_A = 0x0000,
        AD_B = 0x0001,
        AD_C = 0x0002,
        AD_X = 0x0003,
        AD_Y = 0x0004,
        AD_Z = 0x0005,
        AD_I = 0x0006,
        AD_J = 0x0007,

        AD_REG_LOOKUP = 0x0008, // [register]
        AD_NEXTWORD_REG_ADD_LOOKUP = 0x00010, // [nextword + register]

        AD_POP  = 0x0018,
        AD_PEEK = 0x0019,
        AD_PUSH = 0x001a,
        AD_SP   = 0x001b,
        AD_PC   = 0x001c,
        AD_OV   = 0x001d,
        AD_NEXTWORD_LOOKUP = 0x001e, // [[PC++]]
        AD_NEXTWORD = 0x001f, // [PC++]

        AD_LIT = 0x20 // Values 0x00-0x1f
    };

    enum BasicOpCodes
    {
        OP_EXTENDED = 0x0,

        OP_SET, // 0x1
        OP_ADD, // 0x2
        OP_SUB, // 0x3
        OP_MUL, // 0x4
        OP_DIV, // 0x5
        OP_MOD, // 0x6
        OP_SHL, // 0x7
        OP_SHR, // 0x8
        OP_AND, // 0x9
        OP_BOR, // 0xa
        OP_XOR, // 0xb
        OP_IFE, // 0xc
        OP_IFN, // 0xd
        OP_IFG, // 0xe
        OP_IFB, // 0xf

        OP_COUNT // 0x10
    };

    enum ExtendedOpCodes
    {
        EOP_EXTENDED = 0x00,

        EOP_JSR, // 0x01

        EOP_COUNT = 0x100
    };

    /*
        Opcodes :
        Basic    = bbbb bbaa aaaa oooo
        Extended = aaaa aaoo oooo 0000
    */

    inline bool isBasicOp(u16 op)
    {
        return (op & 0xf) != 0;
    }

    inline u8 decodeA(u16 op)
    {
        return (op >> 4) & 0x3f;
    }

    inline u8 decodeB(u16 op)
    {
        return (op >> 10) & 0x3f;
    }

    // For extended ops
    inline u8 decodeExA(u16 op)
    {
        return (op >> 10) & 0x3f;
    }

    inline u8 decodeExOp(u16 op)
    {
        return (op >> 4) & 0x3f;
    }

    inline u16 encodeA(u16 op, u16 a)
    {
        // 1111 1100 0000 1111
        return (op & 0xfc0f) | ((a << 4) & 0x03f0);
    }

    inline u16 encodeB(u16 op, u16 b)
    {
        // 0000 0011 1111 1111
        return (op & 0x03ff) | ((b << 10) & 0xfc00);
    }

    inline u16 encodeExOp(u16 op, u16 opcode)
    {
        // 1111 1100 0000 0000
        return (op & 0xfc00) | ((opcode << 4) & 0x03f0);
    }

    inline u16 encodeExA(u16 op, u16 a)
    {
        // 0000 0011 1111 1111
        return (op & 0x03ff) | ((a << 10) & 0xfc00);
    }

    // Nextword-using operand?
    inline bool isAdvancePC(u8 code)
    {
        // Operand forms that advance pc :
        // 0x08-0x17, 0x1e, 0x1f
        return (code >= 0x10 && code <= 0x17) ||
            (code == 0x1e || code == 0x1f);
    }


    class DCPU
    {
    private :

        u16 m_ram[DCPU_RAM_SIZE];   // Memory
        u16 m_r[8];         // Registers
        u16 m_sp;           // Stack pointer
        u16 m_pc;           // Program counter
        u16 m_ov;           // Overflow
        u32 m_steps;        // Number of steps
        u32 m_cycles;       // Number of cycles

    public :

        // Constructs a DCPU with all memories set to zero
        DCPU()
        {
            memset(m_ram, 0, DCPU_RAM_SIZE * sizeof(u16));
            memset(m_r, 0, 8 * sizeof(u16));
            m_sp = 0;
            m_pc = 0;
            m_ov = 0;
            m_steps = 0;
            m_cycles = 0;
        }

        // Executes one instruction
        void step();

        u16 getMemory(u16 addr) const;
        const u16 * getMemory() const { return m_ram; }
        void setMemory(u16 addr, u16 val);
        void setMemory(const u16 ram[DCPU_RAM_SIZE]);

        u16 getRegister(u8 i) const { return m_r[i]; }

        u16 getSP() const { return m_sp; }
        u16 getPC() const { return m_pc; }
        u16 getOV() const { return m_ov; }
        u32 getSteps() const { return m_steps; }
        u32 getCycles() const { return m_cycles; }

        // Prints CPU state as text in a stream
        void printState(std::ostream & os);

    private :

        // Evaluates next operand
        u16 * operand(u16 code);

        // Skips one instruction
        void skip();

        // Performs the basic operation that have just been read
        void basicOp(u16 op);

        // Performs the extended operation that have just been read
        void extendedOp(u16 op);

    };

} // namespace dcpu11

#endif // DCPU_HPP_INCLUDED
