#include <assert.h>
#include "Keyboard.hpp"

namespace dcpu
{
enum KeyboardCodes
{
	KB_BACKSPACE = 0x10,
	KB_RETURN = 0x11,
	KB_INSERT = 0x12,
	KB_DELETE = 0x13,
	KB_ASCII_BEG = 0x20,
	KB_ASCII_END = 0x7f,
	KB_UP = 0x80,
	KB_DOWN = 0x81,
	KB_LEFT = 0x82,
	KB_RIGHT = 0x83,
	KB_SHIFT = 0x90,
	KB_CONTROL = 0x91
};

void Keyboard::onEvent(const sf::Event & e)
{
	const u32 unicode = e.text.unicode;
	if(e.type == sf::Event::TextEntered && unicode >= 0x20 && unicode < 0x7f)
	{
#ifdef DCPU_DEBUG
		std::cout << "I: GenericKeyboard: text entered (" << unicode << ")" << std::endl;
#endif
		pushEvent((u16)unicode);
	}
	else if(
		e.type == sf::Event::KeyPressed /*||
		e.type == sf::Event::KeyReleased*/)
	{
#ifdef DCPU_DEBUG
		std::cout << "I: GenericKeyboard: key pressed/released" << std::endl;
#endif

		u16 k = 0;

		switch(e.key.code)
		{
		case sf::Keyboard::Key::BackSpace: k = KB_BACKSPACE; break;
		case sf::Keyboard::Key::Return:    k = KB_RETURN; break;
		case sf::Keyboard::Key::Insert:    k = KB_INSERT; break;
		case sf::Keyboard::Key::Delete:    k = KB_DELETE; break;
		case sf::Keyboard::Key::Up:        k = KB_UP; break;
		case sf::Keyboard::Key::Down:      k = KB_DOWN; break;
		case sf::Keyboard::Key::Left:      k = KB_LEFT; break;
		case sf::Keyboard::Key::Right:     k = KB_RIGHT; break;
		case sf::Keyboard::Key::LShift:    k = KB_SHIFT; break;
		case sf::Keyboard::Key::RShift:    k = KB_SHIFT; break;
		case sf::Keyboard::Key::LControl:  k = KB_CONTROL; break;
		case sf::Keyboard::Key::RControl:  k = KB_CONTROL; break;
		default: break;
		}

		if(k != 0)
			pushEvent(k);
	}
}

void Keyboard::pushEvent(u16 k)
{
	m_buffer[m_bufferWritePos] = k;

	m_bufferWritePos++;
	if(m_bufferWritePos >= DCPU_GENERIC_KEYBOARD_BUFSIZE)
		m_bufferWritePos = 0;

	if(m_interruptMsg)
	{
		if(r_dcpu == 0)
			return;
		r_dcpu->interrupt(m_interruptMsg);
	}
}

u16 Keyboard::nextEvent()
{
	const u16 k = m_buffer[m_bufferReadPos];
	m_buffer[m_bufferReadPos] = 0;

	++m_bufferReadPos;
	if(m_bufferReadPos >= DCPU_GENERIC_KEYBOARD_BUFSIZE)
		m_bufferReadPos = 0;

	return k;
}

#define IKD(__key) sf::Keyboard::isKeyPressed(sf::Keyboard::Key::__key)

bool Keyboard::isKeyPressed(u16 k)
{
	bool res = false;

	switch(k)
	{
	case KB_BACKSPACE:  res = IKD(BackSpace); break;
	case KB_RETURN:     res = IKD(Return); break;
	case KB_INSERT:     res = IKD(Insert); break;
	case KB_DELETE:     res = IKD(Delete); break;
	case KB_UP:         res = IKD(Up); break;
	case KB_DOWN:       res = IKD(Down); break;
	case KB_LEFT:       res = IKD(Left); break;
	case KB_RIGHT:      res = IKD(Right); break;
	case KB_SHIFT:      res = IKD(LShift) || IKD(RShift); break;
	case KB_CONTROL:    res = IKD(LControl) || IKD(RControl); break;

	// TODO Keyboard: isKeyPressed for certain ASCII values
	// Note: the compiler didn't accepted r_input->IsKeyDown(k - '0'), because it expects an enum...

	case 0x20:  res = IKD(Space); break;

	case 0x30:  res = IKD(Num0) || IKD(Numpad0); break;
	case 0x31:  res = IKD(Num1) || IKD(Numpad1); break;
	case 0x32:  res = IKD(Num2) || IKD(Numpad2); break;
	case 0x33:  res = IKD(Num3) || IKD(Numpad3); break;
	case 0x34:  res = IKD(Num4) || IKD(Numpad4); break;
	case 0x35:  res = IKD(Num5) || IKD(Numpad5); break;
	case 0x36:  res = IKD(Num6) || IKD(Numpad6); break;
	case 0x37:  res = IKD(Num7) || IKD(Numpad7); break;
	case 0x38:  res = IKD(Num8) || IKD(Numpad8); break;
	case 0x39:  res = IKD(Num9) || IKD(Numpad9); break;

	case 0x41: case 0x61:   res = IKD(A); break;
	case 0x42: case 0x62:   res = IKD(B); break;
	case 0x43: case 0x63:   res = IKD(C); break;
	case 0x44: case 0x64:   res = IKD(D); break;
	case 0x45: case 0x65:   res = IKD(E); break;
	case 0x46: case 0x66:   res = IKD(F); break;
	case 0x47: case 0x67:   res = IKD(G); break;
	case 0x48: case 0x68:   res = IKD(H); break;
	case 0x49: case 0x69:   res = IKD(I); break;
	case 0x4a: case 0x6a:   res = IKD(J); break;
	case 0x4b: case 0x6b:   res = IKD(K); break;
	case 0x4c: case 0x6c:   res = IKD(L); break;
	case 0x4d: case 0x6d:   res = IKD(M); break;
	case 0x4e: case 0x6e:   res = IKD(N); break;
	case 0x4f: case 0x6f:   res = IKD(O); break;
	case 0x50: case 0x70:   res = IKD(P); break;
	case 0x51: case 0x71:   res = IKD(Q); break;
	case 0x52: case 0x72:   res = IKD(R); break;
	case 0x53: case 0x73:   res = IKD(S); break;
	case 0x54: case 0x74:   res = IKD(T); break;
	case 0x55: case 0x75:   res = IKD(U); break;
	case 0x56: case 0x76:   res = IKD(V); break;
	case 0x57: case 0x77:   res = IKD(W); break;
	case 0x58: case 0x78:   res = IKD(X); break;
	case 0x59: case 0x79:   res = IKD(Y); break;
	case 0x5a: case 0x7a:   res = IKD(Z); break;

	default: break;
	}

	return res;
}

void Keyboard::clearBuffer()
{
	m_bufferWritePos = 0;
	m_bufferReadPos = 0;
	memset(m_buffer, 0, DCPU_GENERIC_KEYBOARD_BUFSIZE * sizeof(u16));
}

void Keyboard::interrupt()
{
	//Interrupts do different things depending on contents of the A register:
	//
	// A | BEHAVIOR
	//---+----------------------------------------------------------------------------
	// 0 | Clear keyboard buffer
	// 1 | Store next key typed in C register, or 0 if the buffer is empty
	// 2 | Set C register to 1 if the key specified by the B register is pressed, or
	//   | 0 if it's not pressed
	// 3 | If register B is non-zero, turn on interrupts with message B. If B is zero,
	//   | disable interrupts
	//---+----------------------------------------------------------------------------

	// Note for 2: this might be confusing because there is no
	// dedicated keys for all characters

	if(r_dcpu == 0)
		return;

	const u16 r = r_dcpu->getRegister(AD_A);
	switch(r)
	{
	case 0:
		clearBuffer();
		break;

	case 1:
		r_dcpu->setRegister(AD_C, nextEvent());
		break;

	case 2:
		r_dcpu->setRegister(AD_C, isKeyPressed(r_dcpu->getRegister(AD_B)));
		break;

	case 3:
		m_interruptMsg = r_dcpu->getRegister(AD_B);
#ifdef DCPU_DEBUG
		std::cout << "I: GenericKeyboard: set interruptMsg to "
			<< m_interruptMsg << std::endl;
#endif
		break;

	default:
#ifdef DCPU_DEBUG
		std::cout << "E: GenericKeyboard: received unknown interrupt ("
			<< r << ")" << std::endl;
#endif
		break;
	}
}

} // namespace dcpu



