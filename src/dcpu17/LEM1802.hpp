#ifndef LEM1802_HPP_INCLUDED
#define LEM1802_HPP_INCLUDED

#define DCPU_LEM1802_HID                0x7349f615
#define DCPU_LEM1802_MANUFACTURER_ID    0x1c6c8b36
#define DCPU_LEM1802_VERSION            0x1802

#define DCPU_LEM1802_TILE_W             4
#define DCPU_LEM1802_TILE_H             8
#define DCPU_LEM1802_NTILES_X           32
#define DCPU_LEM1802_NTILES_Y           12

#define DCPU_LEM1802_VRAM_SIZE          DCPU_NTILES_X * DCPU_NTILES_Y
#define DCPU_LEM1802_W                  DCPU_LEM1802_TILE_W * DCPU_LEM1802_NTILES_X
#define DCPU_LEM1802_H                  DCPU_LEM1802_TILE_H * DCPU_LEM1802_NTILES_Y

#include <SFML/Graphics.hpp>

#include "IHardwareDevice.hpp"

namespace dcpu
{
    class LEM1802 : public IHardwareDevice
    {
    private :

        sf::Image m_defaultFont;
        sf::Image m_font;
        sf::Sprite m_fontSprite;

        DCPU * r_dcpu;
        u16 m_vramAddr;
        u16 m_fontAddr;
        u16 m_paletteAddr;


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

        LEM1802()
        {
            r_dcpu = 0;
            m_vramAddr = 0;
            m_fontAddr = 0;
            m_paletteAddr = 0;

            m_fontSprite.SetImage(m_font);
        }

        virtual u32 getHID() const
        { return DCPU_LEM1802_HID; }

        virtual u16 getVersion() const
        { return DCPU_LEM1802_VERSION; }

        virtual u32 getManufacturerID() const
        { return DCPU_LEM1802_MANUFACTURER_ID; }

        void connect(DCPU & dcpu);
        void disconnect();

        virtual void interrupt();

        void intMapScreen();
        void intMapFont();
        void intMapPalette();
        void intSetBorderColor();
        void intDumpFont();
        void intDumpPalette();

        bool loadFontFromImage(const std::string & filename);
        void update(float delta);
        void render(sf::RenderWindow & win);

    };

} // namespace dcpu

#endif // LEM1802_HPP_INCLUDED
