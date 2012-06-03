#ifndef DCPU11EMULATOR_HPP_INCLUDED
#define DCPU11EMULATOR_HPP_INCLUDED

#include <SFML/Graphics.hpp>

#include "DCPU.hpp"

#define DCPU11_VRAM_START 0x8000 // Video
#define DCPU11_KBRAM_START 0x9000 // Keyboard

/*
    DCPU emulator window
*/

namespace dcpu11
{
    class Emulator
    {
    private :

        sf::RenderWindow * m_win;   // Main window
        sf::Font m_font;            // Font for debug text
        sf::String m_cpuStateText;  // CPU state display

        // RAMViz (Not implemented yet)
        //sf::Image m_ramViz;         // RAM graphical view (words => pixels)
        //u32 m_ramVizCursor          // Current position of ramViz updater
        //sf::Sprite m_ramVizSprite;  // Sprite used to draw the RAMViz

        // DCPU16 1.1
        DCPU m_dcpu;                // CPU

        // LEM1802
        sf::Image m_charset;        // Character glyphs
        sf::Sprite m_tileSprite;    // Sprite used to draw screen's tiles
        u16 m_vramAddr;             // Adress of VRAM in the DCPU16 RAM

    public :

        // Constructs an emulator with all memories of the CPU set to 0
        Emulator()
        {
            m_win = 0;
            m_vramAddr = DCPU11_VRAM_START;
            //m_ramVizCursor = 0;
        }

        ~Emulator()
        {
            if(m_win != 0)
                delete m_win;
        }

        // Loads ressources (images, fonts...).
        // Returns false if it failed.
        bool loadContent();

        // Loads a program into the DCPU16
        bool loadProgram(const std::string & filename);

        // Dumps the DCPU memory as text
        bool dumpMemoryAsText(const std::string & filename);

        // Runs the emulator
        void run();

    private :

        // Updates the screen from video RAM
        void drawScreen();

        void drawCPUState();

        // Updates the DCPU16
        void updateCPU();

        void updateRamViz();
        void drawRamViz();
    };

} // namespace dcpu11

#endif // DCPU11EMULATOR_HPP_INCLUDED
