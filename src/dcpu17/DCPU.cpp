#include <cstdio>

#include "DCPU.hpp"
#include "utility.hpp"

namespace dcpu
{

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
	// TODO DCPU: change back to memcpy (previously changed for debug purpose)
	//memcpy(m_ram, ram, DCPU_RAM_SIZE * sizeof(u16));
	for(u32 i = 0; i < DCPU_RAM_SIZE; i++)
		m_ram[i] = ram[i];
}

// Numbers from -1 to 30
static u16 g_lit[0x20] = { 0xffff,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e
};

// Base cost of basic operations
static u32 g_opCost[OP_COUNT] = {
	0, // 0x00
	1, // 0x01 SET
	2, // 0x02 ADD
	2, // 0x03 SUB
	2, // 0x04 MUL
	2, // 0x05 MLI
	3, // 0x06 DIV
	3, // 0x07 DVI
	3, // 0x08 MOD
	3, // 0x09 MDI
	1, // 0x0a AND
	1, // 0x0b BOR
	1, // 0x0c XOR
	1, // 0x0d SHR
	1, // 0x0e ASR
	1, // 0x0f SHL
	2, // 0x10 IFB
	2, // 0x11 IFC
	2, // 0x12 IFE
	2, // 0x13 IFN
	2, // 0x14 IFG
	2, // 0x15 IFA
	2, // 0x16 IFL
	2, // 0x17 IFU
	0, // 0x18
	0, // 0x19
	3, // 0x1a ADX
	3, // 0x1b SBX
	0, // 0x1c
	0, // 0x1d
	2, // 0x1e STI
	2  // 0x1f STD
};

// Base cost of non-basic operations
static u32 g_eopCost[EOP_COUNT] = {
	0, // 0x00
	3, // 0x01 JSR
	0, // 0x02
	0, // 0x03
	0, // 0x04
	0, // 0x05
	0, // 0x06
	0, // 0x07
	4, // 0x08 INT
	1, // 0x09 IAG
	1, // 0x0a IAS
	3, // 0x0b RFI
	2, // 0x0c IAQ
	0, // 0x0d
	0, // 0x0e
	0, // 0x0f
	2, // 0x10 HWN
	4, // 0x11 HWQ
	4, // 0x12 HWI
	0, // 0x13
	0, // 0x14
	0, // 0x15
	0, // 0x16
	0, // 0x17
	0, // 0x18
	0, // 0x19
	0, // 0x1a
	0, // 0x1b
	0, // 0x1c
	0, // 0x1d
	0, // 0x1e
	0  // 0x1f
};

// Evaluates next operand
u16 * DCPU::operand(u16 code, bool isB)
{
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
	// [Register + [PC++]] (reg + nextword)
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x15: case 0x16: case 0x17:
		++m_cycles;
		return m_ram + ((m_r[code & 7] + m_ram[m_pc++]) & 0xffff);

	// (PUSH / [--SP]) if in b, or (POP / [SP++]) if in a
	case 0x18:
		++m_cycles;
		if(isB)
			return m_ram + (--m_sp); // PUSH
		else
			return m_ram + m_sp++; // POP
	// [SP] (PEEK)
	case 0x19:
		return m_ram + m_sp;
	// [SP + next word] (PICK n)
	case 0x1a:
		++m_cycles;
		return m_ram + ((m_sp + m_ram[m_pc++]) & 0xffff);
	// SP
	case 0x1b:
		return &m_sp;
	// PC
	case 0x1c:
		return &m_pc;
	// OV
	case 0x1d:
		return &m_ex;
	// [[PC++]] ([next word])
	case 0x1e:
		return m_ram + m_ram[m_pc++];
	// [PC++] (next word)
	case 0x1f:
		return m_ram + m_pc++;

	default: // 0x20-0x3f
		return g_lit + (code & 0x1f);
	}
}

// Skips one instruction
// (PC is assumed to point an opcode)
void DCPU::skip(bool fromIF)
{
	u16 op = m_ram[m_pc++];

	if(isBasicOp(op))
	{
		if(isOperandAdvancePC(decodeA(op)))
			++m_pc;
		if(isOperandAdvancePC(decodeB(op)))
			++m_pc;
	}
	else
	{
		if(isOperandAdvancePC(decodeExA(op)))
			++m_pc;
	}

	++m_cycles;

	if(fromIF)
	{
		// The branching opcodes take one cycle longer to perform if the test fails
		// When they skip an if instruction, they will skip an additional instruction
		// at the cost of one extra cycle. This lets you easily chain conditionals.
		if(isBranchingOP(decodeOp(op)))
			skip(false);
	}
}

// Executes one instruction
void DCPU::step()
{
	++m_steps;

	if(m_broken)
		return;

	if(m_haltCycles > 0)
	{
		--m_haltCycles;
		++m_cycles;
		return;
	}

	// Get next operation
	u16 op = m_ram[m_pc++];

	// Execute instruction
	if(isBasicOp(op))
		basicOp(op);
	else
		extendedOp(op);

	// Perform queued interrupts
	if(!m_intQueueEmpty)
	{
		u16 msg = 0;
		if(popInterrupt(msg))
			interrupt(msg);
	}
}

// Performs the basic operation that have just been read
void DCPU::basicOp(u16 op)
{
	// b is always handled by the processor after a.

	// Handle a
	u16 a_code = decodeA(op);
	u16 * a_addr = operand(a_code, false);
	u16 a = *a_addr;

	// Handle b
	u16 b_code = decodeB(op);
	u16 * b_addr = operand(b_code, true);
	u16 b = *b_addr;

	u8 opcode = decodeOp(op);
	s32 res = 0;
	m_cycles += g_opCost[opcode];

	switch (opcode)
	{
	case OP_SET:
		res = a;
		break;

	case OP_ADD:
		res = b + a;
		m_ex = (res > 0xffff) ? 1 : 0;
		break;

	case OP_SUB:
		res = b - a;
		m_ex = (res < 0) ? 0xffff : 0;
		break;

	case OP_MUL:
		res = b * a;
		m_ex = res >> 16;
		break;

	case OP_MLI:
		// like MUL, but treat b, a as signed
		res = asSigned(b) * asSigned(a);
		m_ex = res >> 16;
		break;

	case OP_DIV:
		if(a)
		{
			res = b / a;
			m_ex = ((b << 16) / a) & 0xffff;
		}
		else
			m_ex = 0;
		break;

	case OP_DVI:
		if(a)
		{
			res = asSigned(b) * asSigned(a);
			m_ex = ((asSigned(b) << 16) / asSigned(a)) & 0xffff;
		}
		else
			m_ex = 0;
		break;

	case OP_MOD:
		if(a)
			res = b % a;
		break;

	case OP_MDI:
		if(a)
			res = asSigned(b) % asSigned(a);
		break;

	case OP_SHL:
		// sets b to b<<a, sets EX to ((b<<a)>>16)&0xffff
		// (logical shift)
		res = b << a;
		m_ex = res >> 16;
		break;

	case OP_ASR:
		// sets b to b>>a, sets EX to ((b<<16)>>>a)&0xffff
		// (arithmetic shift) (treats b as signed).
		// Note:
		// The C language has only one right shift operator, >>.
		// Many C compilers choose which right shift to perform depending
		// on what type of integer is being shifted;
		// often signed integers are shifted using the arithmetic shift,
		// and unsigned integers are shifted using the logical shift.
		res = asSigned(b) >> a;
		m_ex = (asSigned(b) << 16) >> a;
		break;

	case OP_SHR:
		// sets b to b>>>a, sets EX to ((b<<16)>>a)&0xffff
		// (logical shift)
		res = b >> a;
		m_ex = (b << 16) >> a;
		break;

	case OP_AND:
		res = b & a;
		break;

	case OP_BOR:
		res = b | a;
		break;

	case OP_XOR:
		res = b ^ a;
		break;

	case OP_IFE:
		if(b != a)
			skip(true);
		return;

	case OP_IFN:
		if(b == a)
			skip(true);
		return;

	case OP_IFG:
		if(b <= a)
			skip(true);
		return;

	case OP_IFB:
		if((b & a) == 0)
			skip(true);
		return;

	case OP_IFC:
		if((b & a) != 0)
			skip(true);
		return;

	case OP_IFU:
		if(asSigned(b) >= asSigned(a))
			skip(true);
		return;

	case OP_IFL:
		if(b >= a)
			skip(true);
		return;

	case OP_IFA:
		if(asSigned(b) <= asSigned(a))
			skip(true);
		return;

	case OP_ADX:
		// sets b to b+a+EX, sets EX to 0x0001 if there is an over-flow, 0x0 otherwise
		res = b + a + m_ex;
		m_ex = res > 0xffff ? 1 : 0;
		break;

	case OP_SBX:
		// sets b to b-a+EX, sets EX to 0xFFFF if there is an under-flow, 0x0 otherwise
		res = b - a + m_ex;
		m_ex = res < 0 ? 0xffff : 0;
		break;

	case OP_STI:
		res = a;
		++m_r[AD_I];
		++m_r[AD_J];
		break;

	case OP_STD:
		res = a;
		--m_r[AD_I];
		--m_r[AD_J];
		break;

	default :
#ifdef DCPU_DEBUG
		std::cout << "E: Unknown opcode " << FORMAT_HEX(opcode)
				  << " at address " << FORMAT_HEX(m_pc) << std::endl;
		setBroken(true);
#endif
		return;
	}

	if(b_code < 0x1f)
		*b_addr = res & 0xffff;
}

// Performs the extended operation that have just been read
void DCPU::extendedOp(u16 op)
{
	//u16 a = decodeExA(op);
	u16 * a_addr = operand(decodeExA(op), false);
	u16 a = *a_addr;
	u8 exOpcode = decodeExOp(op);

	m_cycles += g_eopCost[exOpcode];

	switch (exOpcode)
	{
	case EOP_JSR:
		// pushes the address of the next instruction to the stack,
		// then sets PC to a
		m_ram[--m_sp] = m_pc;
		m_pc = a;
		return;

	case EOP_INT:
		// triggers a software interrupt with message a
		interrupt(a);
		return;

	case EOP_IAG:
		*a_addr = m_ia;
		return;

	case EOP_IAS:
		m_ia = a;
		return;

	case EOP_RFI:
		// Interrupt handlers should end with RFI, which will disable interrupt queueing
		// and pop A and PC from the stack as a single atomic instruction.
		m_intQueueing = false;
		*a_addr = m_ram[m_sp++];
		m_pc = m_ram[m_sp++];
		return;

	case EOP_IAQ:
		// If a is nonzero, interrupts will be added to the queue
		// instead of triggered. if a is zero, interrupts will be
		// triggered as normal again
		m_intQueueing = a != 0;
		return;

	case EOP_HWN:
		// Sets a to number of connected hardware devices
		*a_addr = m_hardwareDevices.size();
		return;

	case EOP_HWQ:
		// Sets A, B, C, X, Y registers to information about hardware a
		// A+(B<<16) is a 32 bit word identifying the hardware id
		// C is the hardware version
		// X+(Y<<16) is a 32 bit word identifying the manufacturer
		if(a < m_hardwareDevices.size())
		{
			IHardwareDevice * hd = m_hardwareDevices[a];
			const u32 hid = hd->getHID();
			const u32 mid = hd->getManufacturerID();
			m_r[AD_A] = hid & 0x0000ffff;
			m_r[AD_B] = (hid >> 16) & 0x0000ffff;
			m_r[AD_C] = hd->getVersion();
			m_r[AD_X] = mid & 0x0000ffff;
			m_r[AD_Y] = (mid >> 16) & 0x0000ffff;
		}
		else
		{
#ifdef DCPU_DEBUG
			std::cout << "E: Failed to read HD info (" << a << ") " << std::endl;
#endif
			m_r[AD_A] = 0;
			m_r[AD_B] = 0;
			m_r[AD_C] = 0;
			m_r[AD_X] = 0;
			m_r[AD_Y] = 0;
		}
		return;

	case EOP_HWI:
		// Sends an interrupt to hardware a
		if(a < m_hardwareDevices.size())
			m_hardwareDevices[a]->interrupt();
#ifdef DCPU_DEBUG
		else
			std::cout << "E: Failed to send an interrupt to hardware device "
				<< FORMAT_HEX(a) << ", which is not connected." << std::endl;
#endif
		return;

	default:
#ifdef DCPU_DEBUG
		std::cout << "E: Unknown non-basic opcode " << FORMAT_HEX(exOpcode)
			<< " at address " << FORMAT_HEX(m_pc) << std::endl;
		setBroken(true);
#endif
		return;
	}
}

// Triggers in interrupt with message msg
void DCPU::interrupt(u16 msg)
{
	// When IA is set to something other than 0, interrupts triggered on the DCPU-16
	// will turn on interrupt queueing, push PC to the stack, followed by pushing A to
	// the stack, then set the PC to IA, and A to the interrupt message.
	if(m_ia == 0)
		return; // Interrupts are disabled
	if(m_intQueueing)
	{
		// Interrupts are queued
		if(!pushInterrupt(msg))
			return; // Error
	}
	else
	{
		// Perform interrupt
#ifdef DCPU_DEBUG
		std::cout << "I: Interrupt triggered " << FORMAT_HEX(msg) << std::endl;
#endif
		m_intQueueing = true;
		m_ram[--m_sp] = m_pc;
		m_ram[--m_sp] = m_r[AD_A];
		m_pc = m_ia;
		m_r[AD_A] = msg;
	}
}

// Push one interrupt to the queue. Returns false if overflow.
bool DCPU::pushInterrupt(u16 msg)
{
	if(m_intQueueEmpty)
	{
		m_intQueueEmpty = false;
		m_intQueuePos = 0;
	}
	else
		++m_intQueuePos;

	if(m_intQueuePos == DCPU_INTQ_SIZE)
	{
#ifdef DCPU_DEBUG
		std::cout << "E: Interrupts queue overflow" << std::endl;
#endif
		setBroken(true);
		return false;
	}
	msg = m_intQueue[m_intQueuePos];
	return true;
}

// Pops one interrupt from the queue. Returns false if the queue is empty.
bool DCPU::popInterrupt(u16 & msg)
{
	if(m_intQueueEmpty)
		return false;

	msg = m_intQueue[m_intQueuePos];
	m_intQueue[m_intQueuePos] = 0;

	if(m_intQueuePos == 0)
		m_intQueueEmpty = true;
	else
		--m_intQueuePos;

	return true;
}

// Connects a hardware device and returns its index.
// Does nothing if it is already connected.
u16 DCPU::connectHardware(IHardwareDevice * hd)
{
	if(m_hardwareDevices.size() == DCPU_MAX_HD)
	{
#ifdef DCPU_DEBUG
		std::cout << "E: can't connect another hardware device, "
			"the limit has been reached" << std::endl;
#endif
		return m_hardwareDevices.size();
	}
	for(u16 i = 0; i < m_hardwareDevices.size(); ++i)
	{
		if(m_hardwareDevices[i] == hd)
			return i;
	}
	m_hardwareDevices.push_back(hd);
	return m_hardwareDevices.size() - 1;
}

// Disconnects a connected hardware device. Does nothing if it is not connected.
void DCPU::disconnectHardware(IHardwareDevice * hd)
{
	u16 i;
	for(i = 0; i < m_hardwareDevices.size(); ++i)
	{
		if(m_hardwareDevices[i] == hd)
			break;
	}
	if(i < m_hardwareDevices.size())
		m_hardwareDevices.erase(m_hardwareDevices.begin() + i);
}

void DCPU::setBroken(bool b)
{
	m_broken = b;
	if(m_broken)
		std::cout << "I: The DCPU is now broken." << std::endl;
}

// Prints CPU state as text in a stream
void DCPU::printState(std::ostream & os)
{
	os << "A  = " << FORMAT_HEX(m_r[0]) << "\n";
	os << "B  = " << FORMAT_HEX(m_r[1]) << "\n";
	os << "C  = " << FORMAT_HEX(m_r[2]) << "\n";
	os << "X  = " << FORMAT_HEX(m_r[3]) << "\n";
	os << "Y  = " << FORMAT_HEX(m_r[4]) << "\n";
	os << "Z  = " << FORMAT_HEX(m_r[5]) << "\n";
	os << "I  = " << FORMAT_HEX(m_r[6]) << "\n";
	os << "J  = " << FORMAT_HEX(m_r[7]) << "\n";
	os << "PC = " << FORMAT_HEX(m_pc) << "\n";
	os << "SP = " << FORMAT_HEX(m_sp) << "\n";
	os << "EX = " << FORMAT_HEX(m_ex) << "\n";
	os << "IA = " << FORMAT_HEX(m_ia) << "\n";
	os << "Queued interrupts = " << m_intQueuePos << "\n";
	os << "Connected HDs = " << m_hardwareDevices.size() << "\n";
	os << "Steps = " << m_steps << "\n";
	os << "Cycles = " << m_cycles << "\n";
	os << "Broken = " << m_broken << "\n";
}

} // namespace dcpu


