#include "MyGPS.hpp"
#include "MyCom.hpp"
#include "config.hpp"

namespace lacar::droid_basestation::firmware
{
    MyGPS MyGPS::s_Instance;

    MyGPS::MyGPS(void) noexcept : m_Status(Status::Ok),
                                  M_RTCMBufferLastWriteMillis(0U),
                                  m_RTCMBufferIndex(0U)
    {
    }

    /// @brief performs all the setup for the GPS.
    void MyGPS::setup(void) noexcept
    {
        enableDebugging(Serial, false);
        if (!begin(Wire))
        {
            Serial.println(F("Failed to begin GPS"));
            m_Status = Status::BeginFailed;
            return;
        }
        Serial.println(F("Began GPS"));

        if (!enableSurveyMode(10, 20.000))
        {
            Serial.println(F("Failed to enable survey mode for GPS"));
            m_Status = Status::EnableSurveyModeFailed;
            return;
        }
        Serial.println(F("Enabled survey mode for GPS"));

        if (!enableRTCMmessage(UBX_RTCM_1005, COM_PORT_I2C, 1)) {
            return;
        }
        if (!enableRTCMmessage(UBX_RTCM_1077, COM_PORT_I2C, 1)) {
            return;
        }
        if (!enableRTCMmessage(UBX_RTCM_1087, COM_PORT_I2C, 1)) {
            return;
        }
        if (!enableRTCMmessage(UBX_RTCM_1230, COM_PORT_I2C, 10)) {
            return;
        }
        Serial.println("Enabled RTCM messages");

        m_Status = Status::Ok;
    }

    /// @brief performs all the processing for the GPS.
    void MyGPS::loop(void) noexcept
    {
        checkUblox();
        
        // Don't do anything if something went wrong.
        if (m_Status != Status::Ok)
            return;

        // Gets the current time.
        const uint32_t currentTime = millis();

        // Flushes the buffer if there is data we've exceeded the max wait time.
        if (m_RTCMBufferIndex > 0 && currentTime - M_RTCMBufferLastWriteMillis > LACAR_DROID_BASESTATION_FIRMWARE__GPS__RTCM_BUFFER_FLUSH_AFTER)
        {
            // Writes the chunk to the com and resets the buffer index.
            Serial.println(F("Flushing RTCM buffer due to timeout"));
            MyCom::getInstance().writeRTCMStreamChunk(m_RTCMBuffer, m_RTCMBufferIndex);
            m_RTCMBufferIndex = 0;
        }
    }

    void MyGPS::processRTCM_v(uint8_t byte) noexcept
    {
        Serial.println("RTCM");

        // Don't do anything if something went wrong.
        if (m_Status != Status::Ok)
            return;

        // Puts the byte in the buffer and updates the last write millis.
        m_RTCMBuffer[m_RTCMBufferIndex++] = byte;
        M_RTCMBufferLastWriteMillis = millis();

        // If the buffer is not full yet, don't send it.
        if (m_RTCMBufferIndex < LACAR_DROID_BASESTATION_FIRMWARE__GPS__RTCM_BUFFER_SIZE)
            return;

        // Writes the chunk to the com and resets the buffer index.
        Serial.println(F("Flushing RTCM buffer since it's full"));
        MyCom::getInstance().writeRTCMStreamChunk(m_RTCMBuffer, m_RTCMBufferIndex);
        m_RTCMBufferIndex = 0;
    }
}
