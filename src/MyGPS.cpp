#include "MyCom.hpp"
#include "config.hpp"
#include "MyGPS.hpp"
#include "MyDisplay.hpp"

namespace lacar::droid_basestation::firmware
{
    /// @brief Constructs a new UBLOX GNSS driver extension instance.
    /// @param processRTCMCallback the callback to call for RTCM processing.
    /// @param processRTCMCallbackUserData the user data for the RTCM processing callback.
    SFE_UBLOX_GNSS_Ext::SFE_UBLOX_GNSS_Ext(ProcessRTCMCallback processRTCMCallback,
                                           void *processRTCMCallbackUserData) noexcept
        : SFE_UBLOX_GNSS(),
          processRTCMCallback_(processRTCMCallback),
          processRTCMCallbackUserData_(processRTCMCallbackUserData)
    {
    }

    /// @brief Overrides the RTCM processing method.
    /// @param byte the byte that should be processed.
    void SFE_UBLOX_GNSS_Ext::processRTCM_v(uint8_t byte) noexcept
    {
        this->processRTCMCallback_(this->processRTCMCallbackUserData_, byte);
    }

    /// @brief Constructs an default config instance.
    MyGPS::Config::Config(void) noexcept
        : observationTime(300U),
          requiredAccuracy(10.000f)
    {
    }

    /// @brief Constructs an empty enabling state data instance.
    MyGPS::EnablingStateData::EnablingStateData(void) noexcept
        : elapsedObservationTime(0),
          meanAccuracy(0.0f)
    {
    }

    MyGPS MyGPS::s_Instance;

    /// @brief Constructs a new GPS instance.
    MyGPS::MyGPS(void) noexcept
        : config_(),
          enablingStateData_(),
          enabledStateData_(),
          peripheral_(MyGPS::staticProcessRTCM, this),
          state_(State::Disabled),
          errorCause_(ErrorCause::Ok)
    {
    }

    // Disabled state.

    /// @brief Entry of the disabled state.
    void MyGPS::disabledEntry(void) noexcept
    {
    }

    /// @brief Do of the disabled state.
    void MyGPS::disabledDo(void) noexcept
    {
    }

    /// @brief Exit of the disabled state.
    void MyGPS::disabledExit(void) noexcept
    {
    }

    // Enabling state.

    /// @brief Entry of the enabling state.
    void MyGPS::enablingEntry(void) noexcept
    {
        // Begins the driver (stupid method name).
        if (!this->peripheral_.begin())
        {
            this->errorCause_ = ErrorCause::PeripheralBeginFailed;
            this->transition(State::Error);
            return;
        }

        // Sets the I2C output of the module.
        if (!this->peripheral_.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA | COM_TYPE_RTCM3))
        {
            this->errorCause_ = ErrorCause::PeripheralSetI2COutputFailed;
            this->transition(State::Error);
            return;
        }

        // Gets the survey status.
        if (!this->peripheral_.getSurveyStatus())
        {
            this->errorCause_ = ErrorCause::PeripheralGetSurveyStatusFailed;
            this->transition(State::Error);
            return;
        }

        // Check if survey mode has not been enabled yet.
        if (!this->peripheral_.getSurveyInActive())
        {
            // Gets the required configuration parameters.
            const uint16_t &observationTime = this->config_.observationTime;
            const float &requiredAccuracy = this->config_.requiredAccuracy;

            // Enable survey mode with the parameters.
            if (!this->peripheral_.enableSurveyMode(observationTime, requiredAccuracy))
            {
                this->errorCause_ = ErrorCause::PeripheralEnableSurveyModeFailed;
                this->transition(State::Error);
                return;
            }
        }

        // Puts the display in surveying mode.
        MyDisplay::getInstance().transition(MyDisplay::State::Survey);
    }

    /// @brief Do of the enabling state.
    void MyGPS::enablingDo(void) noexcept
    {
        // If the survey is valid then perform final configuration and transition to
        //  enabled state.
        if (this->peripheral_.getSurveyInValid())
        {
            // Enables the RTCM messages on the I2C port.
            if (!this->peripheral_.enableRTCMmessage(UBX_RTCM_1005, COM_PORT_I2C, 1) ||
                !this->peripheral_.enableRTCMmessage(UBX_RTCM_1077, COM_PORT_I2C, 1) ||
                !this->peripheral_.enableRTCMmessage(UBX_RTCM_1087, COM_PORT_I2C, 1) ||
                !this->peripheral_.enableRTCMmessage(UBX_RTCM_1230, COM_PORT_I2C, 10))
            {
                this->errorCause_ = ErrorCause::PeripheralEnableRTCMMessagesFailed;
                this->transition(State::Error);
                return;
            }

            // Transition to the enabled state.
            this->transition(State::Enabled);
            return;
        }

        // Requests the SVIN status.
        if (!this->peripheral_.getSurveyStatus())
        {
            this->errorCause_ = ErrorCause::PeripheralSvinStatusRequestFailed;
            this->transition(State::Error);
            return;
        }

        // Gets the elapsed observation time and the mean accuraccy and puts them
        //  in the enabled state data. Unfortunately the designers of this library
        //  are retards, and did not make it possible to check whether or not these
        //  values are correct.
        this->enablingStateData_.elapsedObservationTime = this->peripheral_.getSurveyInObservationTime();
        this->enablingStateData_.meanAccuracy = this->peripheral_.getSurveyInMeanAccuracy();
    }

    /// @brief Exit of the enabling state.
    void MyGPS::enablingExit(void) noexcept
    {
        MyDisplay::getInstance().transition(MyDisplay::State::Overview);
    }

    // Enabled state.

    /// @brief Entry of the enabled state.
    void MyGPS::enabledEntry(void) noexcept
    {
        // Resets the buffer index for the RTCM messages.
        this->enabledStateData_.rtcmBufferIdx = 0U;
    }

    /// @brief Do of the enabled state.
    void MyGPS::enabledDo(void) noexcept
    {
        const uint32_t currentMillis = millis();

        // Performs the updating of the GPS module (stupid name).
        this->peripheral_.checkUblox();

        // Flush the RTCM buffer if a given time has passed.
        if (this->enabledStateData_.rtcmBufferIdx > 0U && currentMillis - this->enabledStateData_._rtcmBufferWritingSince > LACAR_DROID_BASESTATION_FIRMWARE__GPS__RTCM_BUFFER_FLUSH_AFTER)
            this->enabledFlushRTCMBuffer();
    }

    /// @brief Exit of the enabled state.
    void MyGPS::enabledExit(void) noexcept
    {
    }

    /// @brief Flushes the RTCM buffer.
    void MyGPS::enabledFlushRTCMBuffer(void) noexcept
    {
        Serial.print("Flushing RTCM buffer of size ");
        Serial.println(this->enabledStateData_.rtcmBufferIdx);

        // Writes the stream chunk.
        MyCom::getInstance().writeRTCMStreamChunk(this->enabledStateData_.rtcmBuffer,
                                                  this->enabledStateData_.rtcmBufferIdx);

        // Resets the buffer index.
        this->enabledStateData_.rtcmBufferIdx = 0;
    }

    // Error state.

    /// @brief Entry of the error state.
    void MyGPS::errorEntry(void) noexcept
    {
    }

    /// @brief Do of the error state.
    void MyGPS::errorDo(void) noexcept
    {
    }

    /// @brief Exit of the error state.
    void MyGPS::errorExit(void) noexcept
    {
    }

    // Current state methods.

    /// @brief Performs the entry of the current state.
    void MyGPS::currentStateEntry(void) noexcept
    {
        switch (this->state_)
        {
        case State::Disabled:
            this->disabledEntry();
            break;
        case State::Enabling:
            this->enablingEntry();
            break;
        case State::Enabled:
            this->enabledEntry();
            break;
        case State::Error:
            this->errorEntry();
            break;
        default:
            break;
        }
    }

    /// @brief Performs the do of the current state.
    void MyGPS::currentStateDo(void) noexcept
    {
        switch (this->state_)
        {
        case State::Disabled:
            this->disabledDo();
            break;
        case State::Enabling:
            this->enablingDo();
            break;
        case State::Enabled:
            this->enabledDo();
            break;
        case State::Error:
            this->errorDo();
            break;
        default:
            break;
        }
    }

    /// @brief Performs the exit of the current state.
    void MyGPS::currentStateExit(void) noexcept
    {
        switch (this->state_)
        {
        case State::Disabled:
            this->disabledExit();
            break;
        case State::Enabling:
            this->enablingExit();
            break;
        case State::Enabled:
            this->enabledExit();
            break;
        case State::Error:
            this->errorExit();
            break;
        default:
            break;
        }
    }

    // Other private method.

    /// @brief The static method to process the given byte.
    /// @param u the user data (MyGPS class instance).
    /// @param byte the byte to process.
    void MyGPS::staticProcessRTCM(void *u, uint8_t byte) noexcept
    {
        // Casts the userdata back to a pointer of the class instance.
        MyGPS &instance = *reinterpret_cast<MyGPS *>(u);

        // Calls the processRTCM method.
        instance.processRTCM(byte);
    }

    /// @brief Processes the given RTCM byte.
    /// @param byte The byte to process.
    void MyGPS::processRTCM(uint8_t byte)
    {
        // Stores the start of the write time if this is the first write to the buffer.
        if (this->enabledStateData_.rtcmBufferIdx == 0)
            this->enabledStateData_._rtcmBufferWritingSince = millis();

        // Writes the byte to the RTCM buffer.
        this->enabledStateData_.rtcmBuffer[this->enabledStateData_.rtcmBufferIdx++] = byte;

        // Don't flush the buffer if it's not full yet.
        if (this->enabledStateData_.rtcmBufferIdx < LACAR_DROID_BASESTATION_FIRMWARE__GPS__RTCM_BUFFER_SIZE)
            return;

        // Flush the buffer.
        this->enabledFlushRTCMBuffer();
    }

    /// @brief Transitions to the given state.
    /// @param state The state to transition to.
    void MyGPS::transition(State state) noexcept
    {
        this->currentStateExit();
        this->state_ = state;
        this->currentStateEntry();
    }

    /// @brief Enables the GPS.
    void MyGPS::enable(void) noexcept
    {
        // Don't transition if we're not disabled.
        if (this->state_ != State::Disabled)
        {
            return;
        }

        // Transitions to the enabling state.
        this->transition(State::Enabling);
    }

    /// @brief performs all the setup for the GPS.
    void MyGPS::setup(void) noexcept
    {
        // Performs the entry of the initial state.
        this->currentStateEntry();
    }

    /// @brief performs all the processing for the GPS.
    void MyGPS::loop(void) noexcept
    {
        // Performs the do of the current state.
        this->currentStateDo();
    }
}
