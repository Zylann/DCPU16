#include <fstream>
#include <SFML/Graphics.hpp> // for sf::Image

#include "utility.hpp"
#include "Assembler.hpp"
#include "Preprocessor.hpp"

namespace dcpu
{
/*
	For DCPU
*/

bool convertImageToDASMFont(
	const std::string & inputFilename,
	const std::string & outputFilename)
{
	/* Load image */
	sf::Image img;
	if(!img.loadFromFile(inputFilename))
	{
		std::cout << "E: Couldn't open image '"
			<< inputFilename << "'" << std::endl;
		return false;
	}

	/* Check image */
	if(img.getSize().x < 128 || img.getSize().y < 32)
	{
		std::cout << "E: The input image is too small." << std::endl;
		return false;
	}

	/* Create output file */
	std::ofstream ofs(
		outputFilename.c_str(),
		std::ios::out|std::ios::binary|std::ios::trunc);

	/* Check output file */
	if(!ofs.good())
	{
		std::cout << "E: Couldn't create file '"
			<< outputFilename << "'" << std::endl;
		ofs.close();
		return false;
	}

	/* Convert */

	u16 cx, cy, k = 0;
	char hex[4] = {'0'};
	// For each glyph
	for(cy = 0; cy < 4; cy++)
	for(cx = 0; cx < 32; cx++, k++)
	{
		// Glyph pos in pixels
		u16 x = cx * 4;
		u16 y = cy * 8;

		u32 fontcode = 0;
		u32 mask = 0x80000000;

		// For each pixel of the glyph
		for(u16 i = 0; i < 4; i++)
		for(u16 j = 0; j < 8; j++)
		{
			sf::Color pix = img.getPixel(x + i, y + 7 - j);
			if(pix == sf::Color(255,255,255))
				fontcode |= mask;
			mask = mask >> 1; // mask is unsigned, then this is a logical shift
		}

		// Split fontcode in two 16-bit words
		u16 w1 = fontcode >> 16;
		u16 w2 = fontcode & 0x0000ffff;

		// Write DASM code
		ofs << "dat 0x";
		u16ToHexStr(w1, hex);
		ofs << hex << ", 0x";
		u16ToHexStr(w2, hex);
		ofs << hex;
		if(k >= 0x20 && k <= 0x7e)
			ofs << " ; '" << (char)k << "'";
		ofs << "\n";
	}

	std::cout << "I: Image converted." << std::endl;
	ofs.close();
	return true;
}

bool loadProgram(DCPU & cpu, const std::string & filename)
{
	std::ifstream ifs(filename.c_str(), std::ios::binary|std::ios::in);
	if(!ifs.good())
	{
		ifs.close();
		std::cout << "Error: cannot open file '" << filename << "'" << std::endl;
		return false;
	}

	Assembler assembler;
	std::cout << "Start assembling..." << std::endl;
	bool res = assembler.assembleStream(ifs);
	if(!res)
	{
		std::cout << "Error: " << assembler.getExceptionString() << std::endl;
		std::cout << "Assembling failed." << std::endl;
	}
	else
	{
		cpu.setMemory(assembler.getAssembly());
		std::cout << "Assembling finished." << std::endl;
	}

	ifs.close();
	return res;
}

bool dumpAsText(DCPU & cpu, const std::string & filename)
{
	std::ofstream ofs(filename.c_str(), std::ios::binary|std::ios::out|std::ios::trunc);
	if(!ofs.good())
	{
		ofs.close();
		std::cout << "Error: cannot create file '" << filename << "'" << std::endl;
		return false;
	}

	std::cout << "Dumping CPU memory..." << std::endl;

	char str[4];
	for(u32 i = 0; i < DCPU_RAM_SIZE; i++)
	{
		if(i % 8 == 0)
		{
			u16ToHexStr(i, str);
			if(i != 0)
				ofs << std::endl;
			ofs << str << ": ";
		}
		u16ToHexStr(cpu.getMemory(i), str);
		ofs << str << " ";
	}

	std::cout << "Dumping finished." << std::endl;

	ofs.close();
	return true;
}

bool preprocessFile(
	const std::string & inputFilename,
	const std::string & outputFilename)
{
	std::ifstream ifs(inputFilename.c_str(), std::ios::in|std::ios::binary);
	if(!ifs.good())
	{
		std::cout << "E: couldn't open file '"
			<< inputFilename << "'" << std::endl;
		return false;
	}

	std::ofstream ofs(outputFilename.c_str(),
		std::ios::out|std::ios::trunc|std::ios::binary);
	if(!ofs.good())
	{
		std::cout << "E: couldn't create file '"
			<< outputFilename << "'" << std::endl;
		return false;
	}

	Preprocessor preprocessor(ifs, ofs);
	if(!preprocessor.process())
	{
		std::cout << "E: Preprocessor: "
			<< preprocessor.getExceptionString() << std::endl;
		return false;
	}

	ifs.close();
	ofs.close();
	return true;
}

/*
	General purpose
*/

char u4ToHexChar(u8 n)
{
	n &= 0xf;
	if(n < 10)
		return '0' + n;
	return 'a' + (n - 10);
}

void u16ToHexStr(u16 n, char str[4])
{
	u16 m = 0x8000; // bit mask 1000 0000 0000 0000
	u8 d = 0;
	u8 di = 0;

	for(u8 i = 0; i < 16; i++)
	{
		if((n & m) != 0)
			d |= 1;

		if((i+1) % 4 == 0)
		{
			str[di] = u4ToHexChar(d);
			d = 0;
			di++;
		}

		m >>= 1;
		d <<= 1;
	}
}


} // namespace dcpu


