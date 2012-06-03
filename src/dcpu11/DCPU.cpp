#include <cstdio>
#include "DCPU.hpp"

namespace dcpu11
{
    // TODO count DCPU cycles

    u16 DCPU::getMemory(u16 addr) const
    {
        return m_ram[addr];
    }

    void DCPU::setMemory(u16 addr, u16 val)
    {
        m_ram[addr] = val;
    }

    void DCPU::setMemory(const u16 ram[DCPU_RAM_SIZE])
    {
        // TODO back to memcpy
        //memcpy(m_ram, ram, DCPU_RAM_SIZE * sizeof(u16));
        for(u32 i = 0; i < DCPU_RAM_SIZE; i++)
            m_ram[i] = ram[i];
    }

    static u16 lit[0x20] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };

    // Base cost of basic operations
    static u32 opCost[OP_COUNT] = { 0,
        1, // SET
        2, // ADD
        2, // SUB
        2, // MUL
        3, // DIV
        3, // MOD
        2, // SHL
        2, // SHR
        1, // AND
        1, // BOR
        1, // XOR
        2, // IFE
        2, // IFN
        2, // IFG
        2  // IFB
    };

    // Base cost of non-basic operations
    static u32 eopCost[EOP_COUNT] = { 0,
        3 // JSR
    };

    // Evaluates next operand
    u16 * DCPU::operand(u16 code)
    {
        if(isAdvancePC(code))
            m_cycles++;

        switch (code)
        {
        // Register
        case 0x00: case 0x01: case 0x02: case 0x03:
        case 0x04: case 0x05: case 0x06: case 0x07:
            return m_r + code;
        // [Register]
        case 0x08: case 0x09: case 0x0a: case 0x0b:
        case 0x0c: case 0x0d: case 0x0e: case 0x0f:
            return m_ram + m_r[code & 7];
        // [Register + [PC++]]
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
            return m_ram + ((m_r[code & 7] + m_ram[m_pc++]) & 0xffff);
        // [SP++]
        case 0x18:
            return m_ram + m_sp++;
        // [SP]
        case 0x19:
            return m_ram + m_sp;
        // [--SP]
        case 0x1a:
            return m_ram + (--m_sp);
        // SP
        case 0x1b:
            return &m_sp;
        // PC
        case 0x1c:
            return &m_pc;
        // OV
        case 0x1d:
            return &m_ov;
        // [[PC++]] ([next word])
        case 0x1e:
            return m_ram + m_ram[m_pc++];
        // [PC++] (next word)
        case 0x1f:
            return m_ram + m_pc++;

        default:
            return lit + (code & 0x1F);
        }
    }

    // Skips one instruction
    // (PC is assumed to point an opcode)
    void DCPU::skip()
    {
        u16 op = m_ram[m_pc++];

        if(isBasicOp(op))
        {
            if(isAdvancePC(decodeA(op)))
                m_pc++;
            if(isAdvancePC(decodeB(op)))
                m_pc++;
        }
        else
        {
            if(isAdvancePC(decodeExA(op)))
                m_pc++;
        }

        m_cycles++;
    }

    // Executes one instruction
    void DCPU::step()
    {
        // Get next operation
        u16 op = m_ram[m_pc++];

        // Execute instruction
        if(isBasicOp(op))
            basicOp(op);
        else
            extendedOp(op);

        m_steps++;
    }

    // Performs the basic operation that have just been read
    void DCPU::basicOp(u16 op)
    {
        // Get a value and adress
        u16 a_code = decodeA(op);
        u16 * a_addr = operand(a_code);
        u16 a = *a_addr;

        // Get b value
        u16 b = *operand(decodeB(op));

        u8 opcode = op & 0xf;
        u32 res = 0;

        switch (opcode)
        {
        case OP_SET:
            res = b;
            break;

        case OP_ADD:
            res = a + b;
            m_ov = res >> 16;
            break;

        case OP_SUB:
            res = a - b;
            m_ov = res >> 16;
            break;

        case OP_MUL:
            res = a * b;
            m_ov = res >> 16;
            break;

        case OP_DIV:
            if(b)
                res = a / b;
            m_ov = res >> 16;
            break;

        case OP_MOD:
            if(b)
                res = a % b;
            break;

        case OP_SHL:
            res = a << b;
            m_ov = res >> 16;
            break;

        case OP_SHR:
            res = a >> b;
            m_ov = res >> 16;
            break;

        case OP_AND:
            res = a & b;
            break;

        case OP_BOR:
            res = a | b;
            break;

        case OP_XOR:
            res = a ^ b;
            break;

        case OP_IFE:
            if(a != b)
                skip();
            return;

        case OP_IFN:
            if(a == b)
                skip();
            return;

        case OP_IFG:

            if(a <= b)
                skip();
            return;

        case OP_IFB:
            if((a & b) == 0)
                skip();
            return;

        default :
            #if DCPU_DEBUG == 1
            //std::cout << "E: Unknown opcode (" << (u32)opcode << ")" << std::endl;
            #endif
            return;
        }

        if(a_code < 0x1f)
            *a_addr = res;

        m_cycles += opCost[opcode];
    }

    // Performs the extended operation that have just been read
    void DCPU::extendedOp(u16 op)
    {
        //u16 a = decodeExA(op);
        u16 a = *operand(decodeExA(op));
        u8 exOpcode = decodeExOp(op);

        switch (exOpcode)
        {
        case EOP_JSR:
            m_ram[--m_sp] = m_pc;
            m_pc = a;
            return;

        default:
            #if DCPU_DEBUG == 1
            //std::cout << "E: Unknown ex. opcode (" << (u32)exOpcode << ")" << std::endl;
            #endif
            return;
        }

        m_cycles += eopCost[exOpcode];
    }

    // Prints CPU state as text in a stream
    void DCPU::printState(std::ostream & os)
    {
        os << "A  = " << m_r[0] << "\n";
        os << "B  = " << m_r[1] << "\n";
        os << "C  = " << m_r[2] << "\n";
        os << "X  = " << m_r[3] << "\n";
        os << "Y  = " << m_r[4] << "\n";
        os << "Z  = " << m_r[5] << "\n";
        os << "I  = " << m_r[6] << "\n";
        os << "J  = " << m_r[7] << "\n";
        os << "PC = " << m_pc << "\n";
        os << "SP = " << m_sp << "\n";
        os << "OV = " << m_ov << "\n";
        os << "Steps = " << m_steps << "\n";
    }

} // namespace dcpu11


