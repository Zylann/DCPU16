#ifndef DCPU_HPP_INCLUDED
#define DCPU_HPP_INCLUDED

/*
    16 bits CPU emulation inspired on the DCPU-16 1.7 specification
    (DCPU-16 has been created by Mojang for the game 0x10c).
*/

#include <string.h> // for memset
#include <iostream>
#include <vector>

namespace dcpu
{
    typedef unsigned long u32;
    typedef unsigned short u16;
    typedef unsigned char u8;
    typedef long s32;
    typedef short s16;
}

#include "IHardwareDevice.hpp"

// Enable debug messages + behavior
// 1 : on
// 0 : off
#define DCPU_DEBUG 1

#define DCPU_REG_COUNT 8
#define DCPU_RAM_SIZE 65536
#define DCPU_INTQ_SIZE 256
#define DCPU_MAX_HD 65535

namespace dcpu
{
    // NOTE 1.7: testing 1.7 features
    // TODO 1.7: test interrupts

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

        AD_PUSH_POP  = 0x0018,
        AD_PEEK = 0x0019,
        AD_PICK = 0x001a,
        AD_SP   = 0x001b,
        AD_PC   = 0x001c,
        AD_EX   = 0x001d,
        AD_NEXTWORD_LOOKUP = 0x001e, // [[PC++]]
        AD_NEXTWORD = 0x001f, // [PC++]

        AD_LIT = 0x0020 // Values 0x00-0x1f
    };

    enum BasicOpCodes
    {
        OP_EXTENDED = 0x00,

        OP_SET, // 0x01
        OP_ADD, // 0x02
        OP_SUB, // 0x03
        OP_MUL, // 0x04
        OP_MLI, // 0x05
        OP_DIV, // 0x06
        OP_DVI, // 0x07
        OP_MOD, // 0x08
        OP_MDI, // 0x09
        OP_AND, // 0x0a
        OP_BOR, // 0x0b
        OP_XOR, // 0x0c
        OP_SHL, // 0x0d
        OP_ASR, // 0x0e
        OP_SHR, // 0x0f
        OP_IFB, // 0x10
        OP_IFC, // 0x11
        OP_IFE, // 0x12
        OP_IFN, // 0x13
        OP_IFG, // 0x14
        OP_IFA, // 0x15
        OP_IFL, // 0x16
        OP_IFU, // 0x17
        OP_0x18,
        OP_0x19,
        OP_ADX, // 0x1a
        OP_SBX, // 0x1b
        OP_0x1c,
        OP_0x1d,
        OP_STI, // 0x1e
        OP_STD, // 0x1f

        OP_COUNT // 0x10
    };

    enum ExtendedOpCodes
    {
        EOP_EXTENDED = 0x00,

        EOP_JSR = 0x01,

        EOP_INT = 0x08,
        EOP_IAG = 0x09,
        EOP_IAS = 0x0a,
        EOP_RFI = 0x0b,
        EOP_IAQ = 0x0c,

        EOP_HWN = 0x10,
        EOP_HWQ = 0x11,
        EOP_HWI = 0x12,

        EOP_COUNT = 0x20
    };

    /*
        1.1 Opcodes :
        Basic    = bbbb bbaa aaaa oooo
        Extended = aaaa aaoo oooo 0000
        1.7 Opcodes :
        Basic    = aaaa aabb bbbo oooo
        Extended = aaaa aaoo ooo0 0000
    */

    inline bool isBasicOp(u16 op)
    {
        //return (op & 0xf) != 0;
        return (op & 0x1f) != 0;
    }

    inline u8 decodeA(u16 op)
    {
        //return (op >> 4) & 0x3f;
        return (op >> 10) & 0x3f;
    }

    inline u8 decodeB(u16 op)
    {
        //return (op >> 10) & 0x3f;
        return (op >> 5) & 0x1f;
    }

    inline u8 decodeOp(u16 op)
    {
        return op & 0x1f;
    }

    // For extended ops
    inline u8 decodeExA(u16 op)
    {
        return (op >> 10) & 0x3f;
    }

    inline u8 decodeExOp(u16 op)
    {
        //return (op >> 4) & 0x3f;
        return (op >> 5) & 0x1f;
    }

    inline u16 encodeA(u16 op, u16 a)
    {
        //return (op & 0xfc0f) | ((a << 4) & 0x03f0);
        return (op & 0x03ff) | ((a << 10) & 0xfc00);
    }

    inline u16 encodeB(u16 op, u16 b)
    {
        //return (op & 0x03ff) | ((b << 10) & 0xfc00);
        return (op & 0b1111110000011111) | ((b << 5) & 0b0000001111100000);
    }

    inline u16 encodeExOp(u16 op, u16 opcode)
    {
        //return (op & 0xfc00) | ((opcode << 4) & 0x03f0);
        return (op & 0b1111110000011111) | ((opcode << 5) & 0b0000001111100000);
    }

    inline u16 encodeExA(u16 op, u16 a)
    {
        //return (op & 0x03ff) | ((a << 10) & 0xfc00);
        return (op & 0x03ff) | ((a << 10) & 0xfc00);
    }

    // Nextword-using operand?
    inline bool isOperandAdvancePC(u8 code)
    {
        // Operand forms that advance pc :
        // 0x08-0x17, 0x1a, 0x1e, 0x1f
        return (code >= 0x10 && code <= 0x17) ||
            (code == 0x1a || code == 0x1e || code == 0x1f);
    }

    // Is the opcode an IF-like ?
    inline bool isBranchingOP(u8 opcode)
    {
        return opcode >= OP_IFB && opcode <= OP_IFU;
    }


    class IHardwareDevice;

    class DCPU
    {
    private :

        u16 m_ram[DCPU_RAM_SIZE];   // Memory
        u16 m_r[DCPU_REG_COUNT];    // Registers
        u16 m_sp;       // Stack pointer
        u16 m_pc;       // Program counter
        u16 m_ex;       // Overflow
        u16 m_ia;       // Interrupt adress

        u32 m_steps;        // Number of steps
        u32 m_cycles;       // Number of cycles
        u32 m_haltCycles;   // Sleep cycles

        bool m_intQueueing;             // Is interrupt queueing enabled?
        u16 m_intQueue[DCPU_INTQ_SIZE]; // Interrupts queue
        u16 m_intQueuePos;              // Current position in intQueue
        bool m_intQueueEmpty;

        bool m_broken;  // True if the CPU cannot work (step() will do nothing)

        std::vector<IHardwareDevice*> m_hardwareDevices;

    public :

        // Constructs a DCPU with all memories set to zero
        DCPU()
        {
            memset(m_ram, 0, DCPU_RAM_SIZE * sizeof(u16));
            memset(m_r, 0, DCPU_REG_COUNT * sizeof(u16));
            m_sp = 0;
            m_pc = 0;
            m_ex = 0;
            m_ia = 0;
            m_steps = 0;
            m_cycles = 0;
            m_haltCycles = 0;
            m_intQueueing = false;
            memset(m_intQueue, 0, DCPU_INTQ_SIZE * sizeof(u16));
            m_intQueuePos = 0;
            m_intQueueEmpty = true;
            m_broken = false;
        }

        // Executes one instruction
        void step();

        // RAM access
        u16 getMemory(u16 addr) const;
        const u16 * getMemory() const { return m_ram; }
        void setMemory(u16 addr, u16 val);
        void setMemory(const u16 ram[DCPU_RAM_SIZE]);

        // Getters
        u16 getRegister(u8 i) const { return m_r[i]; }
        u16 getSP() const { return m_sp; }
        u16 getPC() const { return m_pc; }
        u16 getEX() const { return m_ex; }
        u16 getIA() const { return m_ia; }
        u32 getSteps() const { return m_steps; }
        u32 getCycles() const { return m_cycles; }
        u32 getHaltCycles() const { return m_haltCycles; }
        u16 getHDCount() const { return m_hardwareDevices.size(); }

        bool isBroken() const { return m_broken; }

        // Setters
        void setBroken(bool b);
        void setRegister(u8 i, u16 value) { m_r[i] = value; }

        // Prints CPU state as text in a stream
        void printState(std::ostream & os);

        // Triggers in interrupt with message msg
        void interrupt(u16 msg);

        // Halts the DCPU for ncycles
        void halt(u32 ncycles) { m_haltCycles += ncycles; }

        // Connects a hardware device and returns its index.
        // Does nothing if it is already connected.
        u16 connectHardware(IHardwareDevice * hd);

        // Disconnects a connected hardware device.
        // Does nothing if it is not connected.
        void disconnectHardware(IHardwareDevice * hd);

    private :

        // Evaluates next operand
        u16 * operand(u16 code, bool isB);

        // Skips one instruction
        void skip(bool fromIF);

        // Performs the basic operation that have just been read
        void basicOp(u16 op);

        // Performs the extended operation that have just been read
        void extendedOp(u16 op);

        // Push one interrupt to the queue. Returns false if overflow.
        bool pushInterrupt(u16 msg);

        // Pops one interrupt from the queue. Returns false if the queue is empty.
        bool popInterrupt(u16 & msg);

    };

} // namespace dcpu

#endif // DCPU_HPP_INCLUDED
