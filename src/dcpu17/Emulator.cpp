#include <sstream>

#include "Emulator.hpp"
#include "utility.hpp"

#define DCPU_EMU_FRAMERATE 60
#define DCPU_EMU_SCREEN_W DCPU_LEM1802_W
#define DCPU_EMU_SCREEN_H DCPU_LEM1802_H
#define DCPU_EMU_FREQUENCY DCPU_STANDARD_FREQUENCY

namespace dcpu
{
// Loads ressources (images, fonts...).
// Returns false if it failed.
bool Emulator::loadContent()
{
	std::string assetsDir = DCPU_ASSETS_DIR;
	assetsDir += '/';

	std::string assetFilename = assetsDir + "lem1802/charset.png";
	if(!m_lem.loadDefaultFontFromImage(assetFilename))
		return false;

	assetFilename = assetsDir + "emulator/Courier_New_Bold.ttf";
	if(!m_font.loadFromFile(assetFilename))
	{
		std::cout << "Error: couldn't load asset '"
			<< assetFilename << "'" << std::endl;
		return false;
	}

	m_cpuStateText.setFont(m_font);
	m_cpuStateText.setColor(sf::Color(255,255,255));
	m_cpuStateText.setScale(0.75, 0.75);

	return true;
}

// Loads a program into the DCPU16
bool Emulator::loadProgram(const std::string & filename)
{
	return dcpu::loadProgram(m_dcpu, filename);
}

bool Emulator::dumpMemory(const std::string & name)
{
	return dcpu::dumpAsText(m_dcpu, name + ".txt")
		&& dcpu::dumpAsImage(m_dcpu, name + ".png");
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
	int k = 4;
	sf::VideoMode videoMode(
		k * (DCPU_EMU_SCREEN_W+2),
		k * (DCPU_EMU_SCREEN_H+2));

	m_win.create(videoMode, "DCPU16 Emulator");
	m_win.setFramerateLimit(DCPU_EMU_FRAMERATE);

	// Set view
	sf::View dcpuView(sf::FloatRect(-1, -1, DCPU_EMU_SCREEN_W+2, DCPU_EMU_SCREEN_H+2));
	m_win.setView(dcpuView);

	sf::Event event;
	sf::Clock timer;
	float delta = 1.f / 60.f;

	// Start the main loop
	while(m_win.isOpen())
	{
		// Update the DCPU
		updateCPU();

		// Update hardware devices
		m_lem.update(delta);
		m_clock.update(delta);

		// Process events
		while(m_win.pollEvent(event))
		{
			// Close window : exit
			if(event.type == sf::Event::Closed)
				m_win.close();

			// Print cpu info in the console if F3 is pressed
			if(event.type == sf::Event::KeyPressed)
			{
				if(event.key.code == sf::Keyboard::Key::F3)
					m_dcpu.printState(std::cout);
			}

			if(!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab))
				m_keyboard.onEvent(event);
		}

		// Clear window's pixels
		m_win.clear();

		// Draw virtual screen
		m_lem.render(m_win);

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab))
		{
			// Draw debug information
			m_win.setView(m_win.getDefaultView());
			drawCPUState();
			m_win.setView(dcpuView);
		}

		// Update the window
		m_win.display();
	}

	// Disconnect devices
	m_keyboard.disconnect();
	m_lem.disconnect();
	m_clock.disconnect();

	std::cout << "Emulator closed." << std::endl;
}

void Emulator::updateCPU()
{
	float frameTime = 1.f / 60.f;

	// Expected clock frequency: 100kHz
	const u32 cycles0 = m_dcpu.getCycles();
	const u32 cycles1 = cycles0 + DCPU_EMU_FREQUENCY * frameTime;

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
	char hex[5] = {'0','0','0','0', 0};

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

	m_cpuStateText.setString(text);
	m_cpuStateText.setPosition(10,10);

	sf::RectangleShape rect(m_win.getView().getSize());
	rect.setFillColor(sf::Color(0,0,0,192));
	m_win.draw(rect);
	m_win.draw(m_cpuStateText);
}


} // namespace dcpu





