#include <assert.h>
#include "LEM1802.hpp"

namespace dcpu
{
    void LEM1802::connect(DCPU & dcpu)
    {
        HardwareDevice::connect(dcpu);
        m_vramAddr = 0;
        m_fontAddr = 0;
        m_paletteAddr = 0;
    }

    void LEM1802::disconnect()
    {
        HardwareDevice::disconnect();
        m_vramAddr = 0;
        m_fontAddr = 0;
        m_paletteAddr = 0;
    }

    void LEM1802::interrupt()
    {
        assert(r_dcpu != 0);
        u16 a = r_dcpu->getRegister(AD_A);

        #if DCPU_DEBUG == 1
        std::cout << "I: LEM1802: received interrupt (" << a << ")" << std::endl;
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
            #if DCPU_DEBUG == 1
            std::cout << "E: LEM1802: received an invalid intCode (" << a << ")" << std::endl;
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

        #if DCPU_DEBUG == 1
        std::cout << "I: LEM1802: screen mapped to addr=" << (int)m_vramAddr << std::endl;
        #endif

        // TODO LEM1802: 1s delay if b goes from 0 to any other value

        if(b == 0)
            disconnect();
    }

    void LEM1802::intMapFont()
    {
        // Reads the B register, and maps the font ram to DCPU-16 ram starting
        // at address B. See below for a description of font ram.
        // If B is 0, the default font is used instead.
        assert(r_dcpu != 0);
        // TODO LEM1802: mapFont
    }

    void LEM1802::intMapPalette()
    {
        // Reads the B register, and maps the palette ram to DCPU-16 ram starting
        // at address B. See below for a description of palette ram.
        // If B is 0, the default palette is used instead.
        assert(r_dcpu != 0);
        // TODO LEM1802: handle palette
    }

    void LEM1802::intSetBorderColor()
    {
        // Reads the B register, and sets the border color to palette index B&0xF
        assert(r_dcpu != 0);
        // TODO LEM1802: handle border color
    }

    void LEM1802::intDumpFont()
    {
        // Reads the B register, and writes the default font data to DCPU-16 ram
        // starting at address B.
        // Halts the DCPU-16 for 256 cycles
        assert(r_dcpu != 0);
        // TODO LEM1802: dumpFont
    }

    void LEM1802::intDumpPalette()
    {
        // Reads the B register, and writes the default palette data to DCPU-16
        // ram starting at address B.
        // Halts the DCPU-16 for 16 cycles
        assert(r_dcpu != 0);
        // TODO LEM1802: dumpPalette
    }

    bool LEM1802::loadFontFromImage(const std::string & filename)
    {
        if(!m_defaultFont.LoadFromFile(filename))
        {
            std::cout << "Error: couldn't load asset '"
                << filename << "'" << std::endl;
            return false;
        }
        // Replace non-white by alpha 0
        for(u32 y = 0; y < m_defaultFont.GetHeight(); y++)
        for(u32 x = 0; x < m_defaultFont.GetWidth(); x++)
        {
            const sf::Color & pix = m_defaultFont.GetPixel(x, y);
            if(pix.r != 255 && pix.g != 255 && pix.b != 255)
                m_defaultFont.SetPixel(x, y, sf::Color(0,0,0,0));
        }

        m_defaultFont.SetSmooth(false);
        m_font = m_defaultFont;
        return true;
    }

    void LEM1802::update(float delta)
    {
        // TODO LEM1802: update
    }

    void LEM1802::render(sf::RenderWindow & win)
    {
        #if DCPU_DEBUG == 1
        assert(r_dcpu != 0);
        #else
        if(r_dcpu == 0)
            return;
        #endif

        if(m_vramAddr == 0)
            return;

        u16 x, y, addr = m_vramAddr;
        sf::IntRect subRect;

        for(y = 0; y < DCPU_LEM1802_NTILES_Y; y++)
        for(x = 0; x < DCPU_LEM1802_NTILES_X; x++, addr++)
        {
            u16 word = r_dcpu->getMemory(addr);
            u8 c = word & 0x007f;
            //bool blink = (word & 0x0080) != 0; // TODO LEM1802: handle blink
            u8 format = ((word & 0xff00) >> 8) & 0x00ff;
            bool fbright = (format & 0b10000000) != 0;
            bool bbright = (format & 0b00001000) != 0;

            sf::Color fclr, bclr;
            // Foreground
            if(format & 0b01000000) // ForeRed
            {
                fclr.r = 128;
                if(fbright)
                    fclr.r += 127;
            }
            if(format & 0b00100000) // ForeGreen
            {
                fclr.g = 128;
                if(fbright)
                    fclr.g += 127;
            }
            if(format & 0b00010000) // ForeBlue
            {
                fclr.b = 128;
                if(fbright)
                    fclr.b += 127;
            }
            // Background
            if(format & 0b00000100) // BackRed
            {
                bclr.r = 128;
                if(bbright)
                    bclr.r += 127;
            }
            if(format & 0b00000010) // BackGreen
            {
                bclr.g = 128;
                if(bbright)
                    bclr.g += 127;
            }
            if(format & 0b00000001) // BackBlue
            {
                bclr.b = 128;
                if(bbright)
                    bclr.b += 127;
            }

            // Draw background
            if(bclr.r || bclr.g || bclr.b)
            {
                win.Draw(sf::Shape::Rectangle(
                    x * DCPU_LEM1802_TILE_W, y * DCPU_LEM1802_TILE_H,
                    (x+1) * DCPU_LEM1802_TILE_W, (y+1) * DCPU_LEM1802_TILE_H,
                    bclr
                ));
            }

            // Draw foreground
            if(fclr.r || fclr.g || fclr.b)
            {
                // Charset position
                u16 cx = DCPU_LEM1802_TILE_W * (c % 32);
                u16 cy = DCPU_LEM1802_TILE_H * (c / 32);

                subRect.Left = cx;
                subRect.Top = cy;
                subRect.Right = cx + DCPU_LEM1802_TILE_W;
                subRect.Bottom = cy + DCPU_LEM1802_TILE_H;

                m_fontSprite.SetSubRect(subRect);
                m_fontSprite.SetPosition(x * DCPU_LEM1802_TILE_W, y * DCPU_LEM1802_TILE_H);
                m_fontSprite.SetColor(fclr);

                win.Draw(m_fontSprite);
            }
        }
    }

} // namespace dcpu

