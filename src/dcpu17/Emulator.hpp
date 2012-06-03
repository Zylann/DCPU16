#ifndef EMULATOR_HPP_INCLUDED
#define EMULATOR_HPP_INCLUDED

#include <SFML/Graphics.hpp>

#include "DCPU.hpp"
#include "LEM1802.hpp"
#include "Keyboard.hpp"

// Old DCPU standards
#define DCPU11_VRAM_START 0x8000 // Video
#define DCPU11_KBRAM_START 0x9000 // Keyboard

/*
    SFML-based DCPU emulator window
*/

namespace dcpu
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

        // DCPU16 1.7
        DCPU m_dcpu;        // CPU
        // Hardware devices
        LEM1802 m_lem;      // Monitor
        Keyboard m_keyboard;

    public :

        // Constructs an emulator with all memories of the CPU set to 0
        Emulator()
        {
            m_win = 0;
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

        // Loads a program into the DCPU16, returns false if it failed
        bool loadProgram(const std::string & filename);

        // Dumps the DCPU memory as text, returns false if it failed
        bool dumpMemoryAsText(const std::string & filename);

        // Runs the emulator
        void run();

    private :

        // Draws an overlay with information about the CPU
        void drawCPUState();

        // Updates the DCPU16
        void updateCPU();

        //void updateRamViz();
        //void drawRamViz();
    };

} // namespace dcpu

#endif // EMULATOR_HPP_INCLUDED
