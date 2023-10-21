#include "config.hpp"
#include "MyDisplay.hpp"
#include "MyCom.hpp"
#include "MyGPS.hpp"

namespace lacar::droid_basestation::firmware
{
    MyDisplay MyDisplay::s_Instance;

    MyDisplay::MyDisplay() : m_Rows(2U), m_Cols(16U),
                             m_LiquidCrystal(0x27, m_Rows, m_Cols),
                             m_State(State::Survey),
                             m_HasState(false)
    {
    }

    void MyDisplay::setup(void) noexcept
    {
        m_LiquidCrystal.init();
        m_LiquidCrystal.backlight();

        transition(State::Survey);
    }

    void MyDisplay::surveyStateEntry(void) noexcept
    {
    }

    void MyDisplay::surveyStateDo(void) noexcept
    {
    }

    void MyDisplay::surveyStateExit(void) noexcept
    {
    }

    void MyDisplay::stateEntry(void) noexcept
    {
        if (!m_HasState) return;
    }

    void MyDisplay::stateDo(void) noexcept
    {
        if (!m_HasState) return;
    }

    void MyDisplay::stateExit(void) noexcept
    {
        if (!m_HasState) return;


    }

    void MyDisplay::transition(State state) noexcept
    {
        stateExit();

        m_State = state;

        stateEntry();
    }

    void MyDisplay::loop(void) noexcept
    {
        stateDo();
    }
}
