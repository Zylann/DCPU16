#include <sstream>

#include "Emulator.hpp"
#include "utility.hpp"

#define DCPU_ASSETS_DIR "assets"
#define DCPU_FRAMERATE 60
#define DCPU_SCREEN_W DCPU_LEM1802_W
#define DCPU_SCREEN_H DCPU_LEM1802_H
#define DCPU_FREQUENCY 100000.f

namespace dcpu
{
    // Loads ressources (images, fonts...).
    // Returns false if it failed.
    bool Emulator::loadContent()
    {
        std::string assetsDir = DCPU_ASSETS_DIR;
        assetsDir += DIR_CHAR;

        std::string assetFilename = assetsDir + "charset.png";
        if(!m_lem.loadFontFromImage(assetFilename))
            return false;

        assetFilename = assetsDir + "Courier_New_Bold.ttf";
        if(!m_font.LoadFromFile(assetFilename))
        {
            std::cout << "Error: couldn't load asset '"
                << assetFilename << "'" << std::endl;
            return false;
        }

        m_cpuStateText.SetFont(m_font);
        m_cpuStateText.SetColor(sf::Color(255,255,255));
        m_cpuStateText.SetScale(0.75, 0.75);

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

        // Connect devices
        m_lem.connect(m_dcpu);
        m_keyboard.connect(m_dcpu);
        m_clock.connect(m_dcpu);

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
        m_keyboard.setInput(&input);
        float delta = 0;

        // Start the main loop
        while(m_win->IsOpened())
        {
            delta = m_win->GetFrameTime();

            // Update the DCPU
            updateCPU();

            // Update hardware devices
            m_lem.update(delta);
            m_clock.update(delta);

            // Process events
            while(m_win->GetEvent(event))
            {
                // Close window : exit
                if(event.Type == sf::Event::Closed)
                    m_win->Close();

                // Print cpu info in the console if F3 is pressed
                if(event.Type == sf::Event::KeyPressed)
                {
                    if(event.Key.Code == sf::Key::F3)
                        m_dcpu.printState(std::cout);
                }

                if(!input.IsKeyDown(sf::Key::Tab))
                    m_keyboard.onEvent(event);
            }

            // Clear window's pixels
            m_win->Clear();

            // Draw virtual screen
            m_lem.render(*m_win);

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

        // Disconnect devices
        m_keyboard.setInput(0);
        m_keyboard.disconnect();
        m_lem.disconnect();
        m_clock.disconnect();

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
        // Expected clock frequency: 100kHz
        const u32 cycles0 = m_dcpu.getCycles();
        #if DCPU_DEBUG == 1
        //const u32 cycles1 = cycles0 + 100.f * m_win->GetFrameTime();
        const u32 cycles1 = cycles0 + DCPU_FREQUENCY * m_win->GetFrameTime();
        #else
        const u32 cycles1 = cycles0 + DCPU_FREQUENCY * m_win->GetFrameTime();
        #endif
        while(m_dcpu.getCycles() < cycles1)
        {
            m_dcpu.step();
            if(m_dcpu.isBroken())
                break;
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

        text += " EX=";
        u16ToHexStr(m_dcpu.getEX(), hex);
        text += hex;

        text += " IA=";
        u16ToHexStr(m_dcpu.getIA(), hex);
        text += hex;

        // Cycles

        text += "\nCycles=";
        std::stringstream ss;
        ss << m_dcpu.getCycles();
        text += ss.str();

        if(m_dcpu.getHaltCycles() > 0)
            text += " on halt...";

        // Draw stuff

        m_cpuStateText.SetText(text);
        m_cpuStateText.SetPosition(10,10);

        const sf::FloatRect & r = m_win->GetView().GetRect();
        m_win->Draw(sf::Shape::Rectangle(r.Left, r.Top, r.Right, r.Bottom, sf::Color(0,0,0,192)));
        m_win->Draw(m_cpuStateText);
    }


} // namespace dcpu





