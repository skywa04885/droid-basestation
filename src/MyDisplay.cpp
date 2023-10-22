#include "config.hpp"
#include "MyDisplay.hpp"
#include "MyCom.hpp"
#include "MyGPS.hpp"

namespace lacar::droid_basestation::firmware
{
    MyDisplay MyDisplay::s_Instance;

    /// @brief Constructs a new display instance.
    MyDisplay::MyDisplay(void) noexcept
        : surveyStateData_(),
          overviewStateData_(),
          lastUpdateMillis_(),
          rows_(2U), cols_(16U),
          peripheral_(0x27, rows_, cols_),
          state_(State::Idle)
    {
    }

    // Disabled state methods.

    /// @brief Entry of the idle state.
    void MyDisplay::idleEntry(void) noexcept
    {
    }

    /// @brief Do of the idle state.
    void MyDisplay::idleDo(void) noexcept
    {
    }

    /// @brief Exit of the idle state.
    void MyDisplay::idleExit(void) noexcept
    {
    }

    // Survey state methods.

    /// @brief Entry of the survey state.
    void MyDisplay::surveyEntry(void) noexcept
    {
        this->surveyStateData_.hasPreviousObservation = false;
    }

    /// @brief Do of the survey state.
    void MyDisplay::surveyDo(void) noexcept
    {
        // Gets the enabling state data.
        const MyGPS::EnablingStateData &enablingStateData =
            MyGPS::getInstance().getEnablingStateData();

        // Don't do anything if nothing changed.
        if (this->surveyStateData_.hasPreviousObservation && this->surveyStateData_.previousMeanAccuracy == enablingStateData.meanAccuracy &&
            this->surveyStateData_.previousObservationTime == enablingStateData.elapsedObservationTime)
            return;

        // Clears the display.
        this->peripheral_.clear();

        // Show that we're surveying.
        this->peripheral_.setCursor(0U, 0U);
        this->peripheral_.print(F("Enabling!"));

        // Show the mean accuracy and the elapsed observation time.
        this->peripheral_.setCursor(0U, 1U);
        this->peripheral_.print(F("E:"));
        this->peripheral_.print(min(enablingStateData.meanAccuracy, 999.9), 2);
        this->peripheral_.print(F(" T:"));
        this->peripheral_.print(min(enablingStateData.elapsedObservationTime, 99999));

        // Stores the current observation as the previous observation.
        this->surveyStateData_.hasPreviousObservation = true;
        this->surveyStateData_.previousMeanAccuracy = enablingStateData.meanAccuracy;
        this->surveyStateData_.previousObservationTime = enablingStateData.elapsedObservationTime;
    }

    /// @brief Exit of the survey state.
    void MyDisplay::surveyExit(void) noexcept
    {
    }

    // Survey state methods.

    /// @brief Entry of the overview state.
    void MyDisplay::overviewEntry(void) noexcept
    {
        // Clears the screen.
        this->peripheral_.clear();

        // Sets displayed to false.
        this->overviewStateData_.displayed = false;
    }

    /// @brief Do of the overview state.
    void MyDisplay::overviewDo(void) noexcept
    {
        char buffer[17] = {0x00};

        // Do not update the display if nothing has changed.
        if (this->overviewStateData_.displayed == true &&
            this->overviewStateData_.gpsState == static_cast<uint8_t>(MyGPS::getInstance().getState()) &&
            this->overviewStateData_.comState == static_cast<uint8_t>(MyCom::getInstance().getState()) &&
            this->overviewStateData_.gpsErrorCause == static_cast<uint8_t>(MyGPS::getInstance().getErrorCause()) &&
            this->overviewStateData_.comErrorCause == static_cast<uint8_t>(MyCom::getInstance().getErrorCause()))
            return;

        // Writes the first line of the LCD.
        this->peripheral_.setCursor(0U, 0U);
        this->peripheral_.print("COM|GPS");

        // Writes the second line containing the task status codes.
        sprintf(buffer, "%01d%02d|%01d%02d", 
                static_cast<uint8_t>(MyCom::getInstance().getState()),
                static_cast<uint8_t>(MyCom::getInstance().getErrorCause()),
                static_cast<uint8_t>(MyGPS::getInstance().getState()),
                static_cast<uint8_t>(MyGPS::getInstance().getErrorCause()));
        this->peripheral_.setCursor(0U, 1U);
        this->peripheral_.print(buffer);

        // Sets displayed to true.
        this->overviewStateData_.displayed = true;

        // Updates the previous values.
        this->overviewStateData_.gpsState = static_cast<uint8_t>(MyGPS::getInstance().getState());
        this->overviewStateData_.comState = static_cast<uint8_t>(MyCom::getInstance().getState());
        this->overviewStateData_.gpsErrorCause = static_cast<uint8_t>(MyGPS::getInstance().getErrorCause());
        this->overviewStateData_.comErrorCause = static_cast<uint8_t>(MyCom::getInstance().getErrorCause());
    }

    /// @brief Exit of the overview state.
    void MyDisplay::overviewExit(void) noexcept
    {
    }

    // Current state methods.

    /// @brief Entry of the current state.
    void MyDisplay::currentStateEntry(void) noexcept
    {
        switch (this->state_)
        {
        case State::Idle:
            this->idleEntry();
            break;
        case State::Survey:
            this->surveyEntry();
            break;
        case State::Overview:
            this->overviewEntry();
            break;
        default:
            break;
        }
    }

    /// @brief Do of the current state.
    void MyDisplay::currentStateDo(void) noexcept
    {
        switch (this->state_)
        {
        case State::Idle:
            this->idleDo();
            break;
        case State::Survey:
            this->surveyDo();
            break;
        case State::Overview:
            this->overviewDo();
            break;
        default:
            break;
        }
    }

    /// @brief Exit of the current state.
    void MyDisplay::currentStateExit(void) noexcept
    {
        switch (this->state_)
        {
        case State::Idle:
            this->idleExit();
            break;
        case State::Survey:
            this->surveyExit();
            break;
        case State::Overview:
            this->overviewExit();
            break;
        default:
            break;
        }
    }

    /// @brief Transitions the display to the given state.
    /// @param state The state to transition to.
    void MyDisplay::transition(State state) noexcept
    {
        this->currentStateExit();
        this->state_ = state;
        this->currentStateEntry();
    }

    /// @brief Performs the setup of the display.
    void MyDisplay::setup(void) noexcept
    {
        // Initializes the peripheral and enables the backlight.
        this->peripheral_.init();
        this->peripheral_.backlight();

        // Enters the initial state.
        this->currentStateEntry();
    }

    /// @brief Performs the loop of the display.
    void MyDisplay::loop(void) noexcept
    {
        const uint32_t currentMillis = millis();

        if (currentMillis - this->lastUpdateMillis_ < 500)
            return;

        this->currentStateDo();

        this->lastUpdateMillis_ = currentMillis;
    }
}
