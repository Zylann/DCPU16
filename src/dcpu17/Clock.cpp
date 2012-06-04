#include <assert.h>
#include "Clock.hpp"

namespace dcpu
{
    void Clock::interrupt()
    {
        #if DCPU_DEBUG == 1
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
            #if DCPU_DEBUG == 1
            std::cout << "E: " << m_name
                << ": received unknown interrupt ("
                << (int)a << ")" << std::endl;
            #endif
            break;
        }
    }

    void Clock::update(float delta)
    {
        #if DCPU_DEBUG == 1
        assert(r_dcpu != 0);
        #else
        if(r_dcpu == 0)
            return;
        #endif

        if(m_timer.GetElapsedTime() >= m_tickInterval)
        {
            m_timer.Reset();
            m_ticks++;
            if(m_interruptMsg)
                r_dcpu->interrupt(m_interruptMsg);
        }
    }

} // namespace dcpu





