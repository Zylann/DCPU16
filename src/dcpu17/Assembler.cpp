#include <sstream>

#include "Assembler.hpp"
#include "utility.hpp"

// TODO Assembler: set an exception if we define a label from a reserved name
#define addLabel(__name, __addr) \
    m_labels.insert(std::pair<std::string,u16>(__name, __addr)).second

#define CHECK_END_OF_LINE \
    if(m_pos >= m_line.size()) { \
        setException("Unexpected end of line"); \
        return false; }

namespace dcpu
{
// -----------------------------------------------------------------------------
//	Character sets
// -----------------------------------------------------------------------------

// Defines whitespace characters
inline bool isWhiteSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Defines litteral characters
inline bool isLitteral(char c)
{
	return (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z')
		|| c == '_';
}

// -----------------------------------------------------------------------------
//	Assembler
//
//	First pass :
//		parse, assemble code and leave label uses empty.
//		After this, label addresses will be known.
//	Second pass :
//		set label adresses where they are needed.
// -----------------------------------------------------------------------------

Assembler::Assembler()
{
	init();

	// Basic opcodes
	m_basicOpcodes["SET"] = OP_SET;
	m_basicOpcodes["ADD"] = OP_ADD;
	m_basicOpcodes["SUB"] = OP_SUB;
	m_basicOpcodes["MUL"] = OP_MUL;
	m_basicOpcodes["MLI"] = OP_MLI;
	m_basicOpcodes["DIV"] = OP_DIV;
	m_basicOpcodes["DVI"] = OP_DVI;
	m_basicOpcodes["MOD"] = OP_MOD;
	m_basicOpcodes["MDI"] = OP_MDI;
	m_basicOpcodes["SHL"] = OP_SHL;
	m_basicOpcodes["ASR"] = OP_ASR;
	m_basicOpcodes["SHR"] = OP_SHR;
	m_basicOpcodes["AND"] = OP_AND;
	m_basicOpcodes["BOR"] = OP_BOR;
	m_basicOpcodes["XOR"] = OP_XOR;
	m_basicOpcodes["IFC"] = OP_IFC;
	m_basicOpcodes["IFA"] = OP_IFA;
	m_basicOpcodes["IFE"] = OP_IFE;
	m_basicOpcodes["IFN"] = OP_IFN;
	m_basicOpcodes["IFG"] = OP_IFG;
	m_basicOpcodes["IFB"] = OP_IFB;
	m_basicOpcodes["IFL"] = OP_IFL;
	m_basicOpcodes["IFU"] = OP_IFU;
	m_basicOpcodes["ADX"] = OP_ADX;
	m_basicOpcodes["SBX"] = OP_SBX;
	m_basicOpcodes["STI"] = OP_STI;
	m_basicOpcodes["STD"] = OP_STD;

	// Non-basic opcodes
	m_extendedOpcodes["JSR"] = EOP_JSR;
	m_extendedOpcodes["INT"] = EOP_INT;
	m_extendedOpcodes["IAG"] = EOP_IAG;
	m_extendedOpcodes["IAS"] = EOP_IAS;
	m_extendedOpcodes["RFI"] = EOP_RFI;
	m_extendedOpcodes["IAQ"] = EOP_IAQ;
	m_extendedOpcodes["HWN"] = EOP_HWN;
	m_extendedOpcodes["HWQ"] = EOP_HWQ;
	m_extendedOpcodes["HWI"] = EOP_HWI;

	// Registers
	m_variables["A"] = Variable(AD_A, true);
	m_variables["B"] = Variable(AD_B, true);
	m_variables["C"] = Variable(AD_C, true);
	m_variables["X"] = Variable(AD_X, true);
	m_variables["Y"] = Variable(AD_Y, true);
	m_variables["Z"] = Variable(AD_Z, true);
	m_variables["I"] = Variable(AD_I, true);
	m_variables["J"] = Variable(AD_J, true);

	// Other variables
	m_variables["PUSH"] = Variable(AD_PUSH_POP);
	m_variables["POP"] =  Variable(AD_PUSH_POP);
	m_variables["PEEK"] = Variable(AD_PEEK);
	m_variables["PICK"] = Variable(AD_PICK);
	m_variables["SP"] =   Variable(AD_SP);
	m_variables["PC"] =   Variable(AD_PC);
	m_variables["EX"] =   Variable(AD_EX);
}

// Resets the assembler and leaves it ready to process a new stream
void Assembler::init()
{
	memset(m_ram, 0, DCPU_RAM_SIZE * sizeof(u16));

	m_pos = 0;
	m_row = 0;
	m_addr = 0;

	m_labels.clear();
	m_labelUses.clear();

	m_exceptionString.clear();
}

bool Assembler::skipWhiteSpace()
{
	while(m_pos < m_line.size())
	{
		if(isWhiteSpace(m_line[m_pos]))
			++m_pos;
		else
			break;
	}

	return m_pos < m_line.size();
}

void Assembler::setException(const std::string & msg)
{
	std::stringstream ss;
	ss << msg << ", at line " << m_row+1 << ", col " << m_pos+1;
	if(!m_line.empty())
		ss << ", near '" << m_line << "'";
	m_exceptionString = ss.str();
}

bool Assembler::assembleStream(std::istream & is)
{
	while(!is.eof())
	{
		// Get line
		std::string line;
		char c = 0;
		// TODO Assembler: make the newline char work on other OS than Linux
		// TODO Assembler: use getline. Seriously.
		while(c != '\n')
		{
			if(is.eof())
				break;
			c = is.get(); // Note : 'is >>' is a formatting operator, should not be used here
			if(is.eof())
				break;
			if(c != '\n')
				line += c;
		}

#ifdef DCPU_DEBUG
		std::cout << "\nAssembling line " << m_row << "..." << std::endl;
		std::cout << "'" << line << "'" << std::endl;
#endif

		// Assemble line
		if(!assembleLine(line))
		{
			if(!isException())
				setException("Unknown error"); // should not occur
			return false;
		}
		++m_row;
	}

	return assembleLabels();
}

bool Assembler::assembleLine(const std::string & str)
{
	m_line = str;
	m_pos = 0;

	if(!skipWhiteSpace())
		return true; // the line is empty

	// Comment

	if(m_line[m_pos] == ';')
	{
#ifdef DCPU_DEBUG
		std::cout << "Skipping comment..." << std::endl;
#endif
		return true;
	}

	// TODO Assembler: check the last interpreted character of a line to be ';' or a newline

	//
	// Label definition
	//

	// TODO Assembler: allow ":label <op stuff...>" syntax

	if(m_line[m_pos] == ':')
	{
#ifdef DCPU_DEBUG
		std::cout << "Label definition..." << std::endl;
#endif
		++m_pos;
		CHECK_END_OF_LINE
		std::string labelName;
		if(parseName(labelName, "label name"))
		{
			if(addLabel(labelName, m_addr))
				return true;
			else
			{
				setException("Label '" + labelName + "' defined twice");
				return false;
			}
		}
		else
			return false;
	}

	//
	// Operations
	//

	std::string opname;
	if(!parseName(opname, "opname"))
		return false;

	strToUpper(opname);

#ifdef DCPU_DEBUG
	std::cout << "Opname : " << opname << " : ";
#endif

	// Basic operation

	std::map<std::string,u16>::iterator it = m_basicOpcodes.find(opname);
	if(it != m_basicOpcodes.end())
	{
#ifdef DCPU_DEBUG
		std::cout << "Basic operation" << std::endl;
#endif
		u16 opcode = it->second;
		Operand b, a;

		if(!parseNextOperand(b))
			return false;

		skipWhiteSpace();
		CHECK_END_OF_LINE
		if(m_line[m_pos] != ',')
		{
			setException("Expected ',' after operand");
			return false;
		}

		++m_pos; // skip the comma
		CHECK_END_OF_LINE

		if(!parseNextOperand(a))
			return false;

		assembleBasicOp(opcode, b, a);
		return true;
	}

	// Extended operation

	it = m_extendedOpcodes.find(opname);
	if(it != m_extendedOpcodes.end())
	{
#ifdef DCPU_DEBUG
		std::cout << "Non-basic operation" << std::endl;
#endif
		u16 opcode = it->second;

		Operand a;
		if(opname != "RFI") // RFI's arg has no effect
		{
			if(!parseNextOperand(a))
				return false;
		}

		assembleExtendedOp(opcode, a);
		return true;
	}

	// Assembly operation

#ifdef DCPU_DEBUG
	std::cout << "Assembly operation" << std::endl;
#endif
	if(opname == "DAT")
	{
		if(!assembleData())
			return false;
		return true;
	}

	// Unrecognized

	setException("Unrecognized command '" + opname + "'");
	return false;
}

bool Assembler::parseNextOperandNoLookup(Operand & op)
{
	skipWhiteSpace();
	CHECK_END_OF_LINE

	const char c = m_line[m_pos];
	if(isdigit(c))
	{
		// Value
		#ifdef DCPU_DEBUG
		std::cout << "Found value" << std::endl;
		#endif
		// TODO Assembler: parse expressions like 10+6*32/4 etc...
		// TODO Assembler: accept ASCII values ('a', '0' etc)
		op.isValue = true;
		if(!parseU16(op.value))
			return false;
	}
	else if(isLitteral(c))
	{
		// Variable or label ?

		std::string name;
		if(!parseName(name))
			return false;

		std::map<std::string,Variable>::iterator it = m_variables.find(name);
		if(it == m_variables.end())
		{
			// If the variable was not found, retry with its uppercase version
			std::string uprName = name;
			strToUpper(uprName);
			it = m_variables.find(uprName);
		}
		if(it != m_variables.end())
		{
			// Variable
#ifdef DCPU_DEBUG
			std::cout << "Found variable" << std::endl;
#endif
			op.value = it->second.addr;
			op.isRegister = it->second.isRegister;
		}
		else
		{
			// Label
#ifdef DCPU_DEBUG
			std::cout << "Found label" << std::endl;
#endif
			LabelUse u;
			u.name = name;
			u.row = m_row;
			u.col = m_pos;
			// Note: we know the label use adress on assembling
			op.labelUse = u;
			op.isLabel = true;
		}
	}
	else
	{
		setException("Expected operand");
		return false;
	}
	return true;
}

bool Assembler::parseNextOperand(Operand & op)
{
#ifdef DCPU_DEBUG
	std::cout << "Parsing next operand..." << std::endl;
#endif

	skipWhiteSpace();
	CHECK_END_OF_LINE

	char c = m_line[m_pos];
	if(c == '[')
	{
		// Lookup

#ifdef DCPU_DEBUG
		std::cout << "Found '['" << std::endl;
#endif

		op.lookup = true;
		++m_pos;
		skipWhiteSpace();
		CHECK_END_OF_LINE

		if(!parseNextOperandNoLookup(op))
			return false;

		skipWhiteSpace();
		CHECK_END_OF_LINE

		if(m_line[m_pos] == '+')
		{
#ifdef DCPU_DEBUG
			std::cout << "Found '+'" << std::endl;
#endif
			// Syntax of [nextword + register]
			m_pos++;
			Operand op2;
			if(!parseNextOperandNoLookup(op2))
				return false;

			if(op.isValue)
			{
				if(op2.isValue)
					op.value += op2.value;
				else if(op2.isRegister)
				{
					op.add = true;
					op.addedRegister = op2.value;
				}
				else if(op2.isLabel)
				{
					setException("Unsupported offset (value+label)");
					return false;
				}
			}
			else if(op.isRegister)
			{
				if(op2.isRegister)
				{
					setException("Unsupported offset (register+register)");
					return false;
				}
				op.addedRegister = op.value;
				op.value = op2.value;
				op.add = true;
			}
			else if(op.isLabel) // op is label
			{
				if(op2.isValue)
				{
					setException("Unsupported offset (label+value)");
					return false;
				}
				else if(op2.isRegister)
				{
					op.add = true;
					op.addedRegister = op2.value;
				}
				else if(op2.isLabel) // op2 is label
				{
					setException("Unsupported offset (label+label)");
					return false;
				}
				else
				{
					setException("Unknown error (op2 not a label)");
					return false;
				}
			}
			else
			{
				setException("Unknown error (op not a label)");
				return false;
			}
		}

		skipWhiteSpace();
		CHECK_END_OF_LINE

		c = m_line[m_pos];
		if(c == ']')
		{
			++m_pos;
			return true;
		}
		else
		{
			std::stringstream ss;
			ss << "Expected ']' at end of operand, got '" << c << "'";
			setException(ss.str());
			return false;
		}
	}
	else
	{
		// No lookup
		if(!parseNextOperandNoLookup(op))
			return false;
	}

	return true;
}

// Parses a set of characters included in a-zA-Z_0-9.
// Param what is the type of name (for debug).
bool Assembler::parseName(std::string & name, std::string what)
{
	// The current pos must be vallid
	if(m_pos >= m_line.size())
	{
		setException("Expected name, got nothing");
		return false;
	}

	// The first letter must be a litteral
	char c = m_line[m_pos];
	if(!isLitteral(c))
	{
		if(what.empty())
			what = "name";
		std::stringstream ss;
		ss << "The first letter of a" << (isVowel(what[0]) ? "n " : " ")
			<< what << " must be a litteral, got '"
			<< c << "' (" << (int)c << ")";
		setException(ss.str());
		return false;
	}

	while(m_pos < m_line.size())
	{
		c = m_line[m_pos];
		if(!isLitteral(c) && !isdigit(c))
			break;
		if(c == ';')
			break;
		name += c;
		++m_pos;
	}

	return true;
}

bool Assembler::parseU16(u16 & value)
{
	if(m_pos >= m_line.size())
	{
		setException("Expected numeric, got nothing");
		return false;
	}

	if(!isdigit(m_line[m_pos]))
	{
		setException("Expected numeric");
		return false;
	}

	if(m_pos + 1 >= m_line.size())
	{
		// One decimal digit
		value = m_line[m_pos] - '0';
		return true;
	}

	if(m_line[m_pos] == '0')
	{
		++m_pos;
		CHECK_END_OF_LINE
		char c = m_line[m_pos];
		if(c == 'x')
		{
			// Hex value
			++m_pos; // skip the 'x'
			CHECK_END_OF_LINE
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

bool Assembler::parseHexU16(u16 & value)
{
	if(m_pos >= m_line.size())
	{
		setException("Expected numeric, got nothing");
		return false;
	}

	if(!isHexDigit(m_line[m_pos]))
	{
		std::stringstream ss;
		ss << "Expected hex digit, got '" << m_line[m_pos] << "'";
		setException(ss.str());
		return false;
	}

	value = 0;
	char c;
	for(u8 d = 0; d < 4 && m_pos < m_line.size(); d++, m_pos++)
	{
		c = m_line[m_pos];
		if(!isHexDigit(c))
			break;
		value <<= 4;
		value += hexDigitToInt(c);
	}

	return true;
}

bool Assembler::parseDecU16(u16 & value)
{
	if(m_pos >= m_line.size())
	{
		setException("Expected numeric, got nothing");
		return false;
	}

	if(!isdigit(m_line[m_pos]))
	{
		std::stringstream ss;
		ss << "Expected digit, got '" << m_line[m_pos] << "'";
		setException(ss.str());
		return false;
	}

	u32 readValue = 0;
	char c;
	for(u8 d = 0; d < 5 && m_pos < m_line.size(); ++d, ++m_pos)
	{
		c = m_line[m_pos];
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

bool Assembler::assembleString()
{
	if(!m_line[m_pos] == '"')
		return false;

	++m_pos;
	CHECK_END_OF_LINE;
	// TODO Assembler: handle escape chars
	for(; m_line[m_pos] != '"'; ++m_pos, ++m_addr)
	{
		if(m_addr >= DCPU_RAM_SIZE)
		{
			setException("Out of memory");
			return false;
		}
		m_ram[m_addr] = m_line[m_pos];
	}
	++m_pos;
	return true;
}

bool Assembler::assembleData()
{
	u16 counter = 256;
	while(counter > 0)
	{
		--counter;

		skipWhiteSpace();
		CHECK_END_OF_LINE

		char c = m_line[m_pos];
		if(c == '"')
		{
#ifdef DCPU_DEBUG
			std::cout << "Found string" << std::endl;
#endif
			// String
			if(!assembleString())
				return false;
		}
		else if(isdigit(c))
		{
#ifdef DCPU_DEBUG
			std::cout << "Found word" << std::endl;
#endif
			// Word
			if(m_addr >= DCPU_RAM_SIZE)
			{
				setException("Out of memory");
				return false;
			}
			u16 word = 0;
			if(!parseU16(word))
				return false;
			m_ram[m_addr] = word;
			++m_addr;
		}
		else
		{
			setException("Unknown data");
			return false;
		}

		skipWhiteSpace();
		if(m_pos >= m_line.size())
			break;

		if(m_line[m_pos] == ',')
			++m_pos;
		else
			break;
	}

	if(counter == 0)
	{
		setException("Max count of data elements reached");
		return false;
	}

	return true;
}

bool Assembler::getOperandCode(const Operand & op, u16 & code, bool isB)
{
	if(op.lookup) // [...]
	{
		if(op.add) // [value + register]
		{ code = AD_NEXTWORD_REG_ADD_LOOKUP + op.addedRegister; return true; }

		if(op.isLabel) // [label]
		{ code = AD_NEXTWORD_LOOKUP; return true; }

		if(op.value <= 0x07) // [register]
		{ code = AD_REG_LOOKUP + op.value; return true; }

		if(isB)
		{
			if(op.value > 0x1f) // [address]
			{ code = AD_NEXTWORD_LOOKUP; return true; }
		}
		else
		{
			if(op.value > 0x3f) // [address]
			{ code = AD_NEXTWORD_LOOKUP; return true; }
		}

		setException("Not allowed, or not implemented");
		return false;
	}
	else
	{
		if(op.isLabel) // label
		{ code = AD_NEXTWORD; return true; }

		if(op.isValue)
		{
			// Note: since 1.7, b can't hold AD_LIT + value.
			// Note2: litterals cover -1 to 30 values
			if(!isB && (op.value <= 0x1e || op.value == 0xffff)) // litteral value
			{
#ifdef DCPU_DEBUG
				std::cout << "op.value=" << op.value << std::endl;
#endif
				// +1 because AD_LIT point to the 0xffff litteral
				code = AD_LIT + 1 + op.value; return true;
			}
		}
		else
		{
			if(op.value <= 0x3f) // value
			{ code = op.value; return true; }
		}

		code = AD_NEXTWORD;
		return true;
	}
}

// Keep track of a label use in order to write their adresses
// later in the assembly (assembling is done in two passes)
void Assembler::addLabelUse(const LabelUse & lu)
{
	std::map<
		std::string,
		std::list<LabelUse> >::iterator it = m_labelUses.find(lu.name);

	if(it != m_labelUses.end())
		it->second.push_back(lu);
	else
	{
		std::pair<std::string, std::list<LabelUse> > p;
		p.first = lu.name;
		p.second.push_back(lu);
		m_labelUses.insert(p);
	}
}

bool Assembler::assembleFullWordOperand(const Operand & op)
{
	if(m_addr > 0xffff)
	{
		setException("Out of memory");
		return false;
	}
	if(!op.isLabel)
		m_ram[m_addr] = op.value;
	else
	{
		LabelUse labelUse = op.labelUse;
		labelUse.addr = m_addr;
		addLabelUse(labelUse);
	}
	++m_addr;
	return true;
}

bool Assembler::assembleBasicOp(u16 opcode, Operand b, Operand a)
{
	if(!getOperandCode(a, a.code, false)) return false;
	if(!getOperandCode(b, b.code, true)) return false;

	std::cout << "opcode=" << opcode << ", b=" << b.code << ", a=" << a.code << std::endl;

	u16 op = opcode; // operation word
	op = encodeA(op, a.code);
	op = encodeB(op, b.code);
	m_ram[m_addr] = op;
	++m_addr;

	if(isOperandAdvancePC(a.code) && !assembleFullWordOperand(a))
		return false;
	if(isOperandAdvancePC(b.code) && !assembleFullWordOperand(b))
		return false;

	return true;
}

bool Assembler::assembleExtendedOp(u16 opcode, Operand a)
{
	if(!getOperandCode(a, a.code, false)) return false;

	u16 op = 0; // operation word
	op = encodeExOp(op, opcode);
	op = encodeExA(op, a.code);
	m_ram[m_addr] = op;
	++m_addr;

	if(isOperandAdvancePC(a.code) && !assembleFullWordOperand(a))
		return false;

	return true;
}

// Called after first assembling pass :
// write label adresses where they are needed
bool Assembler::assembleLabels()
{
#ifdef DCPU_DEBUG
	std::cout << "Assembling labels..." << std::endl;
#endif

	if(m_labelUses.empty())
	{
#ifdef DCPU_DEBUG
		std::cout << "No labels." << std::endl;
		return true;
#endif // DCPU_DEBUG
	}

	std::map<std::string, std::list<LabelUse> >::iterator usesIt;
	std::map<std::string, u16>::iterator labelIt;
	std::list<LabelUse>::const_iterator useListIt;

	for(usesIt = m_labelUses.begin(); usesIt != m_labelUses.end(); usesIt++)
	{
		const std::list<LabelUse> & useList = usesIt->second;
		for(useListIt = useList.begin(); useListIt != useList.end(); useListIt++)
		{
			const LabelUse & use = *useListIt;

			labelIt = m_labels.find(use.name);
			if(labelIt != m_labels.end())
			{
				m_ram[use.addr] = labelIt->second;
			}
			else
			{
				m_pos = use.col;
				m_row = use.row;
				std::stringstream ss;
				ss << "Undefined label '" << use.name << "'";
				if(use.name == "o" || use.name == "O")
				{
					ss << " (this is the old name for overflow, maybe you should use EX instead?)";
				}
				setException(ss.str());
				return false;
			}
		}
	}

	return true;
}

} // namespace dcpu







