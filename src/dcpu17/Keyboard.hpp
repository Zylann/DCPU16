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
    private :

        u16 m_buffer[DCPU_GENERIC_KEYBOARD_BUFSIZE]; // cyclic buffer
        u16 m_bufferPos;
        u16 m_interruptMsg;

        const sf::Input * r_input;

    public :

        Keyboard() : HardwareDevice()
        {
            m_HID = DCPU_GENERIC_KEYBOARD_HID;
            m_manufacturerID = DCPU_GENERIC_KEYBOARD_MANUFACTURER_ID;
            m_version = DCPU_GENERIC_KEYBOARD_VERSION;
            m_name = "GenericKeyboard";
            m_interruptMsg = 0;
            r_input = 0;
            clearBuffer();
        }

        void setInput(const sf::Input * input) { r_input = input; }
        void onEvent(const sf::Event & e);
        void pushEvent(u16 k);
        u16 popEvent();
        bool isKeyPressed(u16 k);
        void clearBuffer();
        virtual void interrupt();
    };

} // namespace dcpu

#endif // DCPUKEYBOARD_HPP_INCLUDED
