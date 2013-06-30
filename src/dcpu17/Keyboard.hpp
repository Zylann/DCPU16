#ifndef DCPUKEYBOARD_HPP_INCLUDED
#define DCPUKEYBOARD_HPP_INCLUDED

#define DCPU_GENERIC_KEYBOARD_HID                0x30cf7406
#define DCPU_GENERIC_KEYBOARD_MANUFACTURER_ID    0x1c6c8b36
#define DCPU_GENERIC_KEYBOARD_VERSION            0x0001

#define DCPU_GENERIC_KEYBOARD_BUFSIZE 16

#include <SFML/Window.hpp>

#include "HardwareDevice.hpp"

namespace dcpu
{

class Keyboard : public HardwareDevice
{
public :

	Keyboard() : HardwareDevice()
	{
		m_HID = DCPU_GENERIC_KEYBOARD_HID;
		m_manufacturerID = DCPU_GENERIC_KEYBOARD_MANUFACTURER_ID;
		m_version = DCPU_GENERIC_KEYBOARD_VERSION;
		m_name = "GenericKeyboard";
		m_interruptMsg = 0;
		clearBuffer();
	}

	void onEvent(const sf::Event & e);
	virtual void interrupt();

private:

	void pushEvent(u16 k);
	u16 nextEvent();
	bool isKeyPressed(u16 k);
	void clearBuffer();

	// Attributes

	u16 m_buffer[DCPU_GENERIC_KEYBOARD_BUFSIZE]; // cyclic buffer
	u16 m_bufferWritePos;
	u16 m_bufferReadPos;
	u16 m_interruptMsg;

};

} // namespace dcpu

#endif // DCPUKEYBOARD_HPP_INCLUDED


