#pragma once

#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "config.hpp"

namespace lacar::droid_basestation::firmware
{
    /// @brief An extension of the SFE_UBLOX_GNSS class to support callbacks for RTCM processing.
    class SFE_UBLOX_GNSS_Ext : public SFE_UBLOX_GNSS
    {
    public:
        typedef void (*ProcessRTCMCallback)(void *, uint8_t);

    private:
        ProcessRTCMCallback processRTCMCallback_;
        void *processRTCMCallbackUserData_;

    public:
        /// @brief Constructs a new UBLOX GNSS driver extension instance.
        /// @param processRTCMCallback the callback to call for RTCM processing.
        /// @param processRTCMCallbackUserData the user data for the RTCM processing callback.
        SFE_UBLOX_GNSS_Ext(ProcessRTCMCallback processRTCMCallback,
                           void *processRTCMCallbackUserData) noexcept;

    public:
        /// @brief Overrides the RTCM processing method.
        /// @param byte the byte that should be processed.
        void processRTCM_v(uint8_t byte) noexcept override;
    };

    class MyGPS
    {
    public:
        /// @brief The configuration for the GPS.
        struct Config
        {
        public:
            const uint16_t observationTime;
            const float requiredAccuracy;

        public:
            /// @brief Constructs an default config instance.
            Config(void) noexcept;
        };

        /// @brief The data of the enabling state.
        struct EnablingStateData
        {
        public:
            uint16_t elapsedObservationTime;
            float meanAccuracy;

        public:
            /// @brief Constructs an empty enabling state data instance.
            EnablingStateData(void) noexcept;
        };

        /// @brief The data of the enabled state.
        struct EnabledStateData
        {
        public:
            uint32_t _rtcmBufferWritingSince;
            uint8_t rtcmBuffer[LACAR_DROID_BASESTATION_FIRMWARE__GPS__RTCM_BUFFER_SIZE];
            uint8_t rtcmBufferIdx;
        };

        /// @brief The cause of an error in the GPS.
        enum class ErrorCause : uint8_t
        {
            Ok = 0,
            PeripheralBeginFailed = 1,
            PeripheralSetI2COutputFailed = 2,
            PeripheralGetSurveyStatusFailed = 3,
            PeripheralEnableSurveyModeFailed = 4,
            PeripheralSvinStatusRequestFailed = 5,
            PeripheralEnableRTCMMessagesFailed = 6,
            PeripheralCheckUbloxFailed = 7,
        };

        /// @brief The state of the GPS.
        enum class State : uint8_t
        {
            Disabled = 0,
            Enabling = 1,
            Enabled = 2,
            Error = 3,
        };

    private:
        static MyGPS s_Instance;

    public:
        /// @brief Gets the current GPS instance.
        /// @return The GPS instance.
        static inline MyGPS &getInstance(void) noexcept
        {
            return s_Instance;
        }

    private:
        Config config_;
        EnablingStateData enablingStateData_;
        EnabledStateData enabledStateData_;
        SFE_UBLOX_GNSS_Ext peripheral_;
        State state_;
        ErrorCause errorCause_;

    public:
        /// @brief Constructs a new GPS instance.
        MyGPS(void) noexcept;

    public:
        /// @brief Gets the current state.
        /// @return the current state.
        inline const State &getState(void) const noexcept
        {
            return this->state_;
        }

        inline const ErrorCause &getErrorCause(void) const noexcept
        {
            return this->errorCause_;
        }

        /// @brief Gets the enabling state data.
        /// @return the enabling state data.
        inline const EnablingStateData &getEnablingStateData(void) const noexcept
        {
            return this->enablingStateData_;
        }

    private:
        // Disabled state.

        /// @brief Entry of the disabled state.
        void disabledEntry(void) noexcept;

        /// @brief Do of the disabled state.
        void disabledDo(void) noexcept;

        /// @brief Exit of the disabled state.
        void disabledExit(void) noexcept;

        // Enabling state.

        /// @brief Entry of the enabling state.
        void enablingEntry(void) noexcept;

        /// @brief Do of the enabling state.
        void enablingDo(void) noexcept;

        /// @brief Exit of the enabling state.
        void enablingExit(void) noexcept;

        // Enabled state.

        /// @brief Entry of the enabled state.
        void enabledEntry(void) noexcept;

        /// @brief Do of the enabled state.
        void enabledDo(void) noexcept;

        /// @brief Exit of the enabled state.
        void enabledExit(void) noexcept;

        /// @brief Flushes the RTCM buffer.
        void enabledFlushRTCMBuffer(void) noexcept;

        // Error state.

        /// @brief Entry of the error state.
        void errorEntry(void) noexcept;

        /// @brief Do of the error state.
        void errorDo(void) noexcept;

        /// @brief Exit of the error state.
        void errorExit(void) noexcept;

        // Current state methods.

        /// @brief Performs the entry of the current state.
        void currentStateEntry(void) noexcept;

        /// @brief Performs the do of the current state.
        void currentStateDo(void) noexcept;

        /// @brief Performs the exit of the current state.
        void currentStateExit(void) noexcept;

        // Other private method.

        /// @brief Transitions to the given state.
        /// @param state The state to transition to.
        void transition(State state) noexcept;

    public:
        /// @brief The static method to process the given byte.
        /// @param u the user data (MyGPS class instance).
        /// @param byte the byte to process.
        static void staticProcessRTCM(void *u, uint8_t byte) noexcept;

        /// @brief Processes the given RTCM byte.
        /// @param byte The byte to process.
        void processRTCM(uint8_t byte);

        /// @brief Enables the GPS.
        void enable(void) noexcept;

        /// @brief performs all the setup for the GPS.
        void setup(void) noexcept;

        /// @brief performs all the processing for the GPS.
        void loop(void) noexcept;
    };
}