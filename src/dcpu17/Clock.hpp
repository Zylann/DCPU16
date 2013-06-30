#ifndef HEADER_CLOCK_HPP_INCLUDED
#define HEADER_CLOCK_HPP_INCLUDED

#include <SFML/System.hpp>
#include "HardwareDevice.hpp"

#define DCPU_GENERIC_CLOCK_MANUFACTURER_ID 0x1c6c8b36
#define DCPU_GENERIC_CLOCK_VERSION 1
#define DCPU_GENERIC_CLOCK_HID 0x12d0b402

namespace dcpu
{
class Clock : public HardwareDevice
{
protected :

	sf::Clock m_timer;
	float m_tickInterval;
	u16 m_ticks;
	u16 m_interruptMsg;

public :

	Clock() : HardwareDevice()
	{
		m_name = "GenericClock";
		m_HID = DCPU_GENERIC_CLOCK_HID;
		m_manufacturerID = DCPU_GENERIC_CLOCK_MANUFACTURER_ID;
		m_version = DCPU_GENERIC_CLOCK_VERSION;

		m_tickInterval = 0;
		m_ticks = 0;
		m_interruptMsg = 0;
	}

	virtual void interrupt();
	virtual void update(float delta);
};

} // namespace dcpu

#endif // HEADER_CLOCK_HPP_INCLUDED
