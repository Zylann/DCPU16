#include <sstream>

#include "Emulator.hpp"
#include "utility.hpp"

#define DCPU_ASSETS_DIR "assets"

#define DCPU_TILE_W 4
#define DCPU_TILE_H 8
#define DCPU_NTILES_X 32
#define DCPU_NTILES_Y 12
#define DCPU_FRAMERATE 60
#define DCPU_VRAM_SIZE DCPU_NTILES_X * DCPU_NTILES_Y + 256

#define DCPU_SCREEN_W DCPU_TILE_W * DCPU_NTILES_X
#define DCPU_SCREEN_H DCPU_TILE_H * DCPU_NTILES_Y

namespace dcpu11
{
    /*
        Emulator
    */

    // Loads ressources (images, fonts...).
    // Returns false if it failed.
    bool Emulator::loadContent()
    {
        std::string assetsDir = DCPU_ASSETS_DIR;
        assetsDir += DIR_CHAR;

        std::string assetFilename = assetsDir + "charset.png";
        if(!m_charset.LoadFromFile(assetFilename))
        {
            std::cout << "Error: couldn't load asset '"
                << assetFilename << "'" << std::endl;
            return false;
        }

        assetFilename = assetsDir + "Courier_New_Bold.ttf";
        if(!m_font.LoadFromFile(assetFilename))
        {
            std::cout << "Error: couldn't load asset '"
                << assetFilename << "'" << std::endl;
            return false;
        }

        m_charset.SetSmooth(false);
        m_cpuStateText.SetFont(m_font);
        m_cpuStateText.SetColor(sf::Color(255,255,255));

        return true;
    }

    // Loads a program into the DCPU16
    bool Emulator::loadProgram(const std::string & filename)
    {
        return dcpu::loadProgram(m_dcpu, filename);
    }

    bool Emulator::dumpMemoryAsText(const std::string & filename)
    {
        return dcpu::dumpAsText(m_dcpu, filename);
    }

    // Runs the emulator
    void Emulator::run()
    {
        std::cout << "Running emulator..." << std::endl;

        m_tileSprite.SetImage(m_charset);

        // Video mode
        int ratio = 4;
        sf::VideoMode videoMode(
            ratio * DCPU_SCREEN_W,
            ratio * DCPU_SCREEN_H);

        // Create window
        if(m_win != 0)
            delete m_win;
        m_win = new sf::RenderWindow(videoMode, "DCPU16 emulator");

        m_win->SetFramerateLimit(DCPU_FRAMERATE);

        // Set view
        sf::View dcpuView(sf::FloatRect(0, 0, DCPU_SCREEN_W, DCPU_SCREEN_H));
        m_win->SetView(dcpuView);

        sf::Event event;
        const sf::Input & input = m_win->GetInput();
        //float delta = 0;

        // Start the main loop
        while(m_win->IsOpened())
        {
            // Update the DCPU
            updateCPU();

            // Process events
            while(m_win->GetEvent(event))
            {
                // Close window : exit
                if(event.Type == sf::Event::Closed)
                    m_win->Close();

                if(event.Type == sf::Event::KeyPressed)
                {
                    if(event.Key.Code == sf::Key::Space)
                        m_dcpu.step();
                }
            }

            // Clear window's pixels
            m_win->Clear();

            // Draw virtual screen
            drawScreen();

            if(input.IsKeyDown(sf::Key::Tab))
            {
                // Draw debug information
                m_win->SetView(m_win->GetDefaultView());
                drawCPUState();
                m_win->SetView(dcpuView);
            }

            // Update the window
            m_win->Display();
        }

        // Delete window
        if(m_win != 0)
        {
            delete m_win;
            m_win = 0;
        }

        std::cout << "Emulator closed." << std::endl;
    }

    void Emulator::updateCPU()
    {
        for(u16 i = 0; i < 1024; i++)
            m_dcpu.step();
    }

    // draws the screen from video RAM
    void Emulator::drawScreen()
    {
        u16 x, y, addr = DCPU11_VRAM_START;
        sf::IntRect subRect;

        for(y = 0; y < DCPU_NTILES_Y; y++)
        for(x = 0; x < DCPU_NTILES_X; x++, addr++)
        {
            u16 word = m_dcpu.getMemory(addr);
            u8 c = word & 0x007f;
            //bool blink = (word & 0x0080) != 0; // TODO handle blink
            //TODO handle back color
            u8 format = ((word & 0xff00) >> 8) & 0x00ff;
            bool bright = (format & 0b10000000) != 0;

            sf::Color clr;
            if(format & 0b01000000) // ForeRed
            {
                clr.r = 128;
                if(bright)
                    clr.r += 127;
            }
            if(format & 0b00100000) // ForeGreen
            {
                clr.g = 128;
                if(bright)
                    clr.g += 127;
            }
            if(format & 0b00010000) // ForeBlue
            {
                clr.b = 128;
                if(bright)
                    clr.b += 127;
            }

            if(clr.r || clr.g || clr.b)
            {
                // Charset position
                u16 cx = DCPU_TILE_W * (c % 32);
                u16 cy = DCPU_TILE_H * (c / 32);

                subRect.Left = cx;
                subRect.Top = cy;
                subRect.Right = cx + DCPU_TILE_W;
                subRect.Bottom = cy + DCPU_TILE_H;

                m_tileSprite.SetSubRect(subRect);
                m_tileSprite.SetPosition(x * DCPU_TILE_W, y * DCPU_TILE_H);
                m_tileSprite.SetColor(clr);

                m_win->Draw(m_tileSprite);
            }
        }
    }

    void Emulator::drawCPUState()
    {
        std::string text;
        char hex[4] = {'0'};

        // Registers

        text += "A=";
        u16ToHexStr(m_dcpu.getRegister(0), hex);
        text += hex;

        text += " B=";
        u16ToHexStr(m_dcpu.getRegister(1), hex);
        text += hex;

        text += " C=";
        u16ToHexStr(m_dcpu.getRegister(2), hex);
        text += hex;

        text += "\nX=";
        u16ToHexStr(m_dcpu.getRegister(3), hex);
        text += hex;

        text += " Y=";
        u16ToHexStr(m_dcpu.getRegister(4), hex);
        text += hex;

        text += " Z=";
        u16ToHexStr(m_dcpu.getRegister(5), hex);
        text += hex;

        text += "\nI=";
        u16ToHexStr(m_dcpu.getRegister(6), hex);
        text += hex;

        text += " J=";
        u16ToHexStr(m_dcpu.getRegister(7), hex);
        text += hex;

        // Variables

        text += "\nPC=";
        u16ToHexStr(m_dcpu.getPC(), hex);
        text += hex;

        text += " SP=";
        u16ToHexStr(m_dcpu.getSP(), hex);
        text += hex;

        text += " OV=";
        u16ToHexStr(m_dcpu.getOV(), hex);
        text += hex;

        // Cycles

        text += "\nCycles=";
        std::stringstream ss;
        ss << m_dcpu.getCycles();
        text += ss.str();

        // Draw stuff

        m_cpuStateText.SetText(text);
        m_cpuStateText.SetPosition(10,10);

        const sf::FloatRect & r = m_win->GetView().GetRect();
        m_win->Draw(sf::Shape::Rectangle(r.Left, r.Top, r.Right, r.Bottom, sf::Color(0,0,0,192)));
        m_win->Draw(m_cpuStateText);
    }


} // namespace dcpu11





