#ifndef HARDWAREDEVICE_HPP_INCLUDED
#define HARDWAREDEVICE_HPP_INCLUDED

#include "IHardwareDevice.hpp"

namespace dcpu
{
class HardwareDevice : IHardwareDevice
{
public :

	HardwareDevice()
	{
		r_dcpu = 0;
		m_HID = 0;
		m_manufacturerID = 0;
		m_version = 0;
		m_name = "<AbstractHardwareDevice>";
	}

	virtual u32 getHID() const              { return m_HID; }
	virtual u16 getVersion() const          { return m_version; }
	virtual u32 getManufacturerID() const   { return m_manufacturerID; }

	virtual void connect(DCPU & dcpu);
	virtual void disconnect();

	virtual void update(float delta) {}

protected :

	DCPU * r_dcpu;
	u32 m_HID;
	u32 m_manufacturerID;
	u16 m_version;
	std::string m_name;

};

} // namespace dcpu

#endif // HARDWAREDEVICE_HPP_INCLUDED


