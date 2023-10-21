#pragma once

#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "config.hpp"

namespace lacar::droid_basestation::firmware
{
    class MyGPS: public SFE_UBLOX_GNSS
    {
    public:
        enum class Status : uint8_t {
            Ok = 0,
            BeginFailed,
            EnableSurveyModeFailed,            
        };

    private:
        static MyGPS s_Instance;

    public:
        static inline MyGPS &getInstance(void) noexcept
        {
            return s_Instance;
        }

    private:
        Status m_Status;
        uint32_t M_RTCMBufferLastWriteMillis;
        uint8_t m_RTCMBuffer[LACAR_DROID_BASESTATION_FIRMWARE__GPS__RTCM_BUFFER_SIZE];
        uint8_t m_RTCMBufferIndex;

    public:
        MyGPS(void) noexcept;

    public:
        inline const Status &getStatus(void) const noexcept
        {
            return m_Status;
        }

    public:
        /// @brief performs all the setup for the GPS. 
        void setup(void) noexcept;

        /// @brief performs all the processing for the GPS.
        void loop(void) noexcept;

    private:
        void processRTCM_v(uint8_t byte) noexcept;
    };
}