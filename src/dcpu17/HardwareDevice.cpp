#include <assert.h>
#include "HardwareDevice.hpp"

namespace dcpu
{
void HardwareDevice::connect(DCPU & dcpu)
{
	if(r_dcpu != 0)
		disconnect();
	r_dcpu = &dcpu;
	r_dcpu->connectHardware(this);
#ifdef DCPU_DEBUG
	std::cout << "I: " << m_name << ": connected" << std::endl;
#endif
}

void HardwareDevice::disconnect()
{
	assert(r_dcpu != 0);
	r_dcpu->disconnectHardware(this);
	r_dcpu = 0;
#ifdef DCPU_DEBUG
	std::cout << "I: " << m_name << ": disconnected" << std::endl;
#endif
}

} // namespace dcpu

