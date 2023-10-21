#pragma once

#include <LiquidCrystal_I2C.h>

namespace lacar::droid_basestation::firmware
{
    class MyDisplay
    {
    public:
        enum class State : uint8_t
        {
            Survey = 0,
        };

    private:
        static MyDisplay s_Instance;

    public:
        static inline MyDisplay &getInstance(void) noexcept
        {
            return s_Instance;
        }

    private:
        uint8_t m_Rows, m_Cols;
        LiquidCrystal_I2C m_LiquidCrystal;
        State m_State;
        bool m_HasState;

    public:
        MyDisplay();

    private:
        void surveyStateEntry(void) noexcept;

        void surveyStateDo(void) noexcept;

        void surveyStateExit(void) noexcept;

        void stateEntry(void) noexcept;

        void stateDo(void) noexcept;

        void stateExit(void) noexcept;

    public:
        void transition(State state) noexcept;

        void setup(void) noexcept;

        void loop(void) noexcept;
    };
}