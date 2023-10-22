#pragma once

#include <LiquidCrystal_I2C.h>

namespace lacar::droid_basestation::firmware
{
    /// @brief The display of the base station.
    class MyDisplay
    {
    public:
        /// @brief The data of the survey state of the display.
        struct SurveyStateData
        {
        public:
            bool hasPreviousObservation;
            float previousMeanAccuracy;
            uint16_t previousObservationTime;
        };

        /// @brief The data of the overview state of the display.
        struct OverviewStateData
        {
        public:
            bool displayed;
            uint8_t gpsState;
            uint8_t comState;
            uint8_t gpsErrorCause;
            uint8_t comErrorCause;
        };

        /// @brief The state of the display.
        enum class State : uint8_t
        {
            Idle = 0,
            Survey = 1,
            Overview = 2,
        };

    private:
        static MyDisplay s_Instance;

    public:
        /// @brief Gets the current instance.
        /// @return The current instance.
        static inline MyDisplay &getInstance(void) noexcept
        {
            return s_Instance;
        }

    private:
        SurveyStateData surveyStateData_;
        OverviewStateData overviewStateData_;
        uint32_t lastUpdateMillis_;
        uint8_t rows_, cols_;
        LiquidCrystal_I2C peripheral_;
        State state_;

    public:
        /// @brief Constructs a new display instance.
        MyDisplay(void) noexcept;

    private:
        // Disabled state methods.

        /// @brief Entry of the idle state.
        void idleEntry(void) noexcept;

        /// @brief Do of the idle state.
        void idleDo(void) noexcept;

        /// @brief Exit of the idle state.
        void idleExit(void) noexcept;

        // Survey state methods.

        /// @brief Entry of the survey state.
        void surveyEntry(void) noexcept;

        /// @brief Do of the survey state.
        void surveyDo(void) noexcept;

        /// @brief Exit of the survey state.
        void surveyExit(void) noexcept;

        // Overview state methods.

        /// @brief Entry of the overview state.
        void overviewEntry(void) noexcept;

        /// @brief Do of the overview state.
        void overviewDo(void) noexcept;

        /// @brief Exit of the overview state.
        void overviewExit(void) noexcept;

        // Current state methods.

        /// @brief Entry of the current state.
        void currentStateEntry(void) noexcept;

        /// @brief Do of the current state.
        void currentStateDo(void) noexcept;

        /// @brief Exit of the current state.
        void currentStateExit(void) noexcept;

    public:
        /// @brief Transitions the display to the given state.
        /// @param state The state to transition to.
        void transition(State state) noexcept;

        /// @brief Performs the setup of the display.
        void setup(void) noexcept;

        /// @brief Performs the loop of the display.
        void loop(void) noexcept;
    };
}