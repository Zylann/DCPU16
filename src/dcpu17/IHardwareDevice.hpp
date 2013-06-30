#ifndef HEADER_IHARDWAREDEVICE_HPP_INCLUDED
#define HEADER_IHARDWAREDEVICE_HPP_INCLUDED

#include "DCPU.hpp"

namespace dcpu
{
class IHardwareDevice
{
public :

	virtual ~IHardwareDevice() {}

	virtual u32 getHID() const = 0;
	virtual u16 getVersion() const = 0;
	virtual u32 getManufacturerID() const = 0;
	virtual void interrupt() = 0;
};

} // namespace dcpu

#endif // IHARDWAREDEVICE_HPP_INCLUDED
