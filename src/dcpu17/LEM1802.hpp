#ifndef HEADER_LEM1802_HPP_INCLUDED
#define HEADER_LEM1802_HPP_INCLUDED

#define DCPU_LEM1802_HID                0x7349f615
#define DCPU_LEM1802_MANUFACTURER_ID    0x1c6c8b36
#define DCPU_LEM1802_VERSION            0x1802

#define DCPU_LEM1802_TILE_W             4
#define DCPU_LEM1802_TILE_H             8
#define DCPU_LEM1802_NTILES_X           32
#define DCPU_LEM1802_NTILES_Y           12

#define DCPU_LEM1802_CHARSET_W 			32
#define DCPU_LEM1802_CHARSET_H 			4

#define DCPU_LEM1802_VRAM_SIZE          DCPU_NTILES_X * DCPU_NTILES_Y
#define DCPU_LEM1802_W                  DCPU_LEM1802_TILE_W * DCPU_LEM1802_NTILES_X
#define DCPU_LEM1802_H                  DCPU_LEM1802_TILE_H * DCPU_LEM1802_NTILES_Y

#include <SFML/Graphics.hpp>

#include "HardwareDevice.hpp"

namespace dcpu
{
class LEM1802 : public HardwareDevice
{
public :

	enum InterruptCodes
	{
		MEM_MAP_SCREEN = 0,
		MEM_MAP_FONT,       // 1
		MEM_MAP_PALETTE,    // 2
		SET_BORDER_COLOR,   // 3
		MEM_DUMP_FONT,      // 4
		MEM_DUMP_PALETTE    // 5
	};

	LEM1802() : HardwareDevice()
	{
		m_vramAddr = 0;
		m_fontAddr = 0;
		m_name = "LEM1802";
		m_HID = DCPU_LEM1802_HID;
		m_manufacturerID = DCPU_LEM1802_MANUFACTURER_ID;
		m_version = DCPU_LEM1802_VERSION;
		m_borderColor = sf::Color(0,0,128);

		loadDefaultPalette();

		m_fontPixels.create(
			DCPU_LEM1802_CHARSET_W * DCPU_LEM1802_TILE_W,
			DCPU_LEM1802_CHARSET_H * DCPU_LEM1802_TILE_H);

		m_fontSprite.setTexture(m_font);

		initDefaultPalette();
	}

	virtual void connect(DCPU & dcpu);
	virtual void disconnect();

	virtual void interrupt();
	virtual void update(float delta);

	void intMapScreen();
	void intMapFont();
	void intMapPalette();
	void intSetBorderColor();
	void intDumpFont();
	void intDumpPalette();

	bool loadDefaultFontFromImage(const std::string & filename);
	void render(sf::RenderWindow & win);

private :

	void initDefaultPalette();
	void loadDefaultPalette();

	sf::Texture m_font;
	sf::Image m_fontPixels;
	sf::Image m_defaultFontPixels;
	sf::Sprite m_fontSprite;
	sf::Color m_borderColor;

	u16 m_vramAddr;
	u16 m_fontAddr;
	sf::Color m_palette[16];
	sf::Color m_defaultPalette[16];

};

} // namespace dcpu

#endif // LEM1802_HPP_INCLUDED
