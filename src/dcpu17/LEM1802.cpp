#include <assert.h>
#include "LEM1802.hpp"
#include "utility.hpp"

namespace dcpu
{

void LEM1802::connect(DCPU & dcpu)
{
	HardwareDevice::connect(dcpu);
	m_vramAddr = 0;
	m_fontAddr = 0;
}

void LEM1802::disconnect()
{
	//m_fontPixels.saveToFile("font.png");

	HardwareDevice::disconnect();
	m_vramAddr = 0;
	m_fontAddr = 0;
}

void LEM1802::initDefaultPalette()
{
	for(u8 i = 0; i < 16; ++i)
	{
		bool bright = i & 0b1000;
		sf::Color c(0,0,0,255);

		if(i & 0b0100)
		{
			c.r = 127;
			if(bright)
				c.r += 128;
		}
		if(i & 0b0010)
		{
			c.g = 127;
			if(bright)
				c.g += 128;
		}
		if(i & 0b0001)
		{
			c.b = 127;
			if(bright)
				c.b += 128;
		}

		m_defaultPalette[i] = c;
	}
}

void LEM1802::loadDefaultPalette()
{
	memcpy(m_palette, m_defaultPalette, 16 * sizeof(sf::Color));
}

void LEM1802::interrupt()
{
	assert(r_dcpu != 0);
	u16 a = r_dcpu->getRegister(AD_A);

#ifdef DCPU_DEBUG
	std::cout << "I: " << m_name << ": received interrupt code " << FORMAT_HEX(a) << std::endl;
#endif

	switch(a)
	{
	case MEM_MAP_SCREEN:
		intMapScreen();
		break;

	case MEM_MAP_FONT:
		intMapFont();
		break;

	case MEM_MAP_PALETTE:
		intMapPalette();
		break;

	case SET_BORDER_COLOR:
		intSetBorderColor();
		break;

	case MEM_DUMP_FONT:
		intDumpFont();
		break;

	case MEM_DUMP_PALETTE:
		intDumpPalette();
		break;

	default:
#ifdef DCPU_DEBUG
		std::cout << "E: " << m_name << ": received an invalid interrupt code "
	              << FORMAT_HEX(a) << std::endl;
#endif
		break;
	}
}

void LEM1802::intMapScreen()
{
	// Reads the B register, and maps the video ram to DCPU-16 ram starting
	// at address B. If B is 0, the screen is disconnected.
	// When the screen goes from 0 to any other value, the LEM1802 takes
	// about one second to start up. Other interrupts sent during this time
	// are still processed.
	assert(r_dcpu != 0);
	u16 b = r_dcpu->getRegister(AD_B);
	m_vramAddr = b;

#ifdef DCPU_DEBUG
	std::cout << "I: " << m_name
		<< ": screen mapped to addr=" << FORMAT_HEX(m_vramAddr) << std::endl;
#endif

	// TODO LEM1802: 1s delay if b goes from 0 to any other value

	if(b == 0)
		disconnect();
}

void LEM1802::intMapFont()
{
	// Reads the B register, and maps the font ram to DCPU-16 ram starting
	// at address B.
	// If B is 0, the default font is used instead.
#ifdef DCPU_DEBUG
	assert(r_dcpu != 0);
#else
	if(r_dcpu == 0)
		return;
#endif

	u16 addr = r_dcpu->getRegister(AD_B);

	if(addr == 0)
	{
#ifdef DCPU_DEBUG
		std::cout << "I: " << m_name << ": Mapping default font" << std::endl;
#endif
		// TODO: LEM1802: this font should be preloaded
		std::string path = DCPU_ASSETS_DIR;
		loadDefaultFontFromImage(path + "/lem1802/charset.png");
		return;
	}

	if(DCPU_RAM_SIZE - addr < 256)
	{
		std::cout << "E: " << m_name
			<< ": Can't map font from adress "
			<< FORMAT_HEX(addr) << ", not enough space" << std::endl;
		return;
	}

#ifdef DCPU_DEBUG
	std::cout << "I: " << m_name << ": Mapping font to addr=" << FORMAT_HEX(addr) << std::endl;
#endif

	// For each glyph
	for(u16 k = 0; k < 128; ++k, addr += 2)
	{
		// Get fontcode
		u32 w1 = r_dcpu->getMemory(addr);
		u32 w2 = r_dcpu->getMemory(addr+1);
		// Font example with letter 'F':
		// word0 = 11111111 /
		//         00001001
		// word1 = 00001001 /
		//         00000000
		u32 fontcode = (w1 << 16) | w2; // Note : '<<' is always a logical shift

		// Get glyph pos
		u16 cx = (k % DCPU_LEM1802_CHARSET_W) * DCPU_LEM1802_TILE_W;
		u16 cy = (k / DCPU_LEM1802_CHARSET_W) * DCPU_LEM1802_TILE_H;

		// Decode fontcode
		for(u16 i = 0; i < DCPU_LEM1802_TILE_W; ++i)
		for(u16 j = 0; j < DCPU_LEM1802_TILE_H; ++j)
		{
			u16 x = cx + i;
			u16 y = cy + DCPU_LEM1802_TILE_H - 1 - j;

			if(fontcode > 0x7fffffff)
				m_fontPixels.setPixel(x, y, sf::Color(255,255,255,255));
			else
				m_fontPixels.setPixel(x, y, sf::Color(0,0,0,0));

			fontcode <<= 1;
		}
	}

	m_font.loadFromImage(m_fontPixels);

	r_dcpu->halt(256);
}

void LEM1802::intMapPalette()
{
	// Reads the B register, and maps the palette ram to DCPU-16 ram starting
	// at address B.
	// If B is 0, the default palette is used instead.

	assert(r_dcpu != 0);

	u16 paletteAddr = r_dcpu->getRegister(AD_B);

	if(paletteAddr + 16 > DCPU_RAM_SIZE)
	{
#ifdef DCPU_DEBUG
		std::cout << "E: " << m_name << ": intMapPalette: palette address "
			<< FORMAT_HEX(paletteAddr) << " is out of bounds." << std::endl;
#endif // DCPU_DEBUG
		return;
	}

#ifdef DCPU_DEBUG
	std::cout << "I: " << m_name
		<< ": Mapping palette to address " << FORMAT_HEX(paletteAddr) << std::endl;
#endif
	for(u16 i = 0; i < 16; ++i)
	{
		u16 w = r_dcpu->getMemory(paletteAddr+i);
		m_palette[i].r = ((w >> 8) & 0xf) << 4;
		m_palette[i].g = ((w >> 4) & 0xf) << 4;
		m_palette[i].b = (w & 0xf) << 4;
	}
}

void LEM1802::intSetBorderColor()
{
	// Reads the B register, and sets the border color to palette index B&0xF
	assert(r_dcpu != 0);
	// TODO LEM1802: handle border color
#ifdef DCPU_DEBUG
	std::cout << "E: " << m_name << ": intSetBorderColor: not implemented yet" << std::endl;
#endif
}

void LEM1802::intDumpFont()
{
	// Reads the B register, and writes the default font data to DCPU-16 ram
	// starting at address B.
	// Halts the DCPU-16 for 256 cycles

	assert(r_dcpu != 0);

	u16 addr = r_dcpu->getRegister(AD_B);

	// Check if the address is valid
	if(addr + 256 > DCPU_RAM_SIZE)
	{
#ifdef DCPU_DEBUG
		std::cout << "E: " << m_name << ": intDumpFont: "
			<< "space at address " << FORMAT_HEX(addr) << "doesn't fits" << std::endl;
#endif
		return;
	}

#ifdef DCPU_DEBUG
	std::cout << "I: " << m_name
		<< ": Dumping default font to address " << FORMAT_HEX(addr) << std::endl;
#endif

	// For each glyph
	for(u16 k = 0; k < 128; ++k, addr += 2)
	{
		// Font example with letter 'F':
		// word0 = 11111111 /
		//         00001001
		// word1 = 00001001 /
		//         00000000

		u16 cx = (k % DCPU_LEM1802_CHARSET_W) * DCPU_LEM1802_TILE_W;
		u16 cy = (k / DCPU_LEM1802_CHARSET_W) * DCPU_LEM1802_TILE_H;

		// Encode fontcode
		u32 fontcode=0;
		for(u16 i = 0; i < DCPU_LEM1802_TILE_W; ++i)
		for(u16 j = 0; j < DCPU_LEM1802_TILE_H; ++j)
		{
			u32 x = cx + i;
			u32 y = cy + DCPU_LEM1802_TILE_H - 1 - j;

			sf::Color c = m_defaultFontPixels.getPixel(x, y);

			fontcode <<= 1;
			if(c.r != 0)
				fontcode |= 1;
		}

		// Set fontcode
		r_dcpu->setMemory(addr, (fontcode >> 16) & 0xffff);
		r_dcpu->setMemory(addr+1, fontcode & 0xffff);
	}

	r_dcpu->halt(256);
}

void LEM1802::intDumpPalette()
{
	// Reads the B register, and writes the default palette data to DCPU-16
	// ram starting at address B.
	// Halts the DCPU-16 for 16 cycles

	assert(r_dcpu != 0);

	u16 paletteAddr = r_dcpu->getRegister(AD_B);

	// Check address
	if(paletteAddr + 16 > DCPU_RAM_SIZE)
	{
#ifdef DCPU_DEBUG
		std::cout << "E: " << m_name << ": intDumpPalette: palette address "
			<< FORMAT_HEX(paletteAddr) << " is out of bounds." << std::endl;
#endif // DCPU_DEBUG
		return;
	}

#ifdef DCPU_DEBUG
	std::cout << "I: " << m_name
		<< ": Dumping palette to address " << FORMAT_HEX(paletteAddr) << std::endl;
#endif
	for(u16 i = 0; i < 16; ++i)
	{
		u16 w = m_defaultPalette[i].b;
		w |= m_defaultPalette[i].g << 4;
		w |= m_defaultPalette[i].r << 8;
		r_dcpu->setMemory(paletteAddr+i, w);
	}

	r_dcpu->halt(16);
}

bool LEM1802::loadDefaultFontFromImage(const std::string & filename)
{
	if(!m_defaultFontPixels.loadFromFile(filename))
	{
		std::cout << "E: " << m_name << ": couldn't load font asset '" << filename << "'" << std::endl;
		return false;
	}
	// Replace non-white by alpha 0
	for(u32 y = 0; y < m_defaultFontPixels.getSize().y; ++y)
	for(u32 x = 0; x < m_defaultFontPixels.getSize().x; ++x)
	{
		const sf::Color & pix = m_defaultFontPixels.getPixel(x, y);
		if(pix.r != 255 && pix.g != 255 && pix.b != 255)
		{
			m_defaultFontPixels.setPixel(x, y, sf::Color(0,0,0,0));
		}
	}

	m_font.loadFromImage(m_defaultFontPixels);
	m_font.setSmooth(false);

	return true;
}

void LEM1802::update(float delta)
{
	// TODO LEM1802: update
}

void LEM1802::render(sf::RenderWindow & win)
{
	if(r_dcpu == 0)
		return;

	if(m_vramAddr == 0)
		return;

	u16 x, y, addr = m_vramAddr;
	sf::IntRect subRect;
	sf::RectangleShape bgRect(sf::Vector2f(DCPU_LEM1802_TILE_W, DCPU_LEM1802_TILE_H));

	for(y = 0; y < DCPU_LEM1802_NTILES_Y; ++y)
	for(x = 0; x < DCPU_LEM1802_NTILES_X; ++x, ++addr)
	{
		u16 word = r_dcpu->getMemory(addr);
		u8 c = word & 0x007f;
		//bool blink = (word & 0x0080) != 0; // TODO LEM1802: handle blink

		const sf::Color & fclr = m_palette[(word >> 12) & 0xf]; // Foreground
		const sf::Color & bclr = m_palette[(word >> 8) & 0xf]; // Background

		// Draw background
		if(bclr.r || bclr.g || bclr.b)
		{
			bgRect.setPosition(x * DCPU_LEM1802_TILE_W, y * DCPU_LEM1802_TILE_H);
			bgRect.setFillColor(bclr);
			win.draw(bgRect);
		}

		// Draw foreground

		// Charset position
		u16 cx = DCPU_LEM1802_TILE_W * (c % 32);
		u16 cy = DCPU_LEM1802_TILE_H * (c / 32);

		subRect.left = cx;
		subRect.top = cy;
		subRect.width = DCPU_LEM1802_TILE_W;
		subRect.height = DCPU_LEM1802_TILE_H;

		m_fontSprite.setTextureRect(subRect);
		m_fontSprite.setPosition(x * DCPU_LEM1802_TILE_W, y * DCPU_LEM1802_TILE_H);
		m_fontSprite.setColor(fclr);

		win.draw(m_fontSprite);
	}
}

} // namespace dcpu

