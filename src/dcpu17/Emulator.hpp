#ifndef HEADER_EMULATOR_HPP_INCLUDED
#define HEADER_EMULATOR_HPP_INCLUDED

#include <SFML/Graphics.hpp>

#include "DCPU.hpp"
#include "LEM1802.hpp"
#include "Keyboard.hpp"
#include "GenericClock.hpp"

namespace dcpu
{

/*
    SFML-based DCPU emulator window
*/
class Emulator
{
private :

	sf::RenderWindow m_win;   // Main window
	sf::Font m_font;            // Font for debug text
	sf::Text m_cpuStateText;  // CPU state display

	// RAMViz (Not implemented yet)
	//sf::Image m_ramViz;         // RAM graphical view (words => pixels)
	//u32 m_ramVizCursor          // Current position of ramViz updater
	//sf::Sprite m_ramVizSprite;  // Sprite used to draw the RAMViz

	// DCPU16 1.7
	DCPU m_dcpu;        // CPU
	// Hardware devices
	LEM1802 m_lem;      // Monitor
	Keyboard m_keyboard;
	GenericClock m_clock;

public :

	// Constructs an emulator with all memories of the CPU set to 0
	Emulator()
	{
//		m_win = 0;
//		m_ramVizCursor = 0;
	}

	~Emulator()
	{
//		if(m_win != 0)
//			delete m_win;
	}

	// Loads ressources (images, fonts...).
	// Returns false if it failed.
	bool loadContent();

	// Loads a program into the DCPU16, returns false if it failed
	bool loadProgram(const std::string & filename);

	// Dumps the DCPU memory as file(s), returns false if it failed
	bool dumpMemory(const std::string & name);

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

#endif // HEADER_EMULATOR_HPP_INCLUDED
