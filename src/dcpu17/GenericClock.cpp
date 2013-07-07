#include <assert.h>
#include "GenericClock.hpp"
#include "utility.hpp"

namespace dcpu
{
void GenericClock::interrupt()
{
#ifdef DCPU_DEBUG
	assert(r_dcpu != 0);
#else
	if(r_dcpu == 0)
		return;
#endif

	u16 a = r_dcpu->getRegister(AD_A);
	switch(a)
	{
	case 0:
	{
		const u16 b = r_dcpu->getRegister(AD_B);
		m_tickInterval = 60.f / (float)b;
		m_ticks = 0;
	}
		break;

	case 1:
		r_dcpu->setRegister(AD_C, m_ticks);
		break;

	case 2:
		m_interruptMsg = r_dcpu->getRegister(AD_B);
		break;

	default:
#ifdef DCPU_DEBUG
		std::cout << "E: " << m_name
			<< ": received unknown interrupt code "
			<< FORMAT_HEX(a) << std::endl;
#endif
		break;
	}
}

void GenericClock::update(float delta)
{
#ifdef DCPU_DEBUG
	assert(r_dcpu != 0);
#else
	if(r_dcpu == 0)
		return;
#endif

	if(m_timer.getElapsedTime().asSeconds() >= m_tickInterval)
	{
		m_timer.restart();
		m_ticks++;
		if(m_interruptMsg)
			r_dcpu->interrupt(m_interruptMsg);
	}
}

} // namespace dcpu





