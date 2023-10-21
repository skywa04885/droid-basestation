#include "MyCom.hpp"
#include "config.hpp"

namespace lacar::droid_basestation::firmware
{
    MyCom MyCom::s_Instance;

    MyCom::MyCom(void) noexcept : m_RF24(LACAR_DROID_BASESTATION_FIRMWARE__RF24__CE, LACAR_DROID_BASESTATION_FIRMWARE__RF24__CS),
                                  m_RF24Network(m_RF24),
                                  m_Status(Status::Ok)
    {
    }

    void MyCom::setup(void) noexcept
    {
        if (!m_RF24.begin())
        {
            Serial.println(F("Failed to begin RF24"));
            m_Status = Status::BeginFailure;
            return;
        }
        Serial.println(F("Began RF24"));

        if (!m_RF24.setDataRate(LACAR_DROID_BASESTATION_FIRMWARE__RF24__DATA_RATE))
        {
            Serial.println(F("Failed to set RF24 data rate"));
            m_Status = Status::SetDataRateFailure;
            return;
        }
        Serial.println(F("Set RF24 data rate"));

        m_RF24.setPALevel(LACAR_DROID_BASESTATION_FIRMWARE__RF24__PA_LAVEL);
        m_RF24.setChannel(LACAR_DROID_BASESTATION_FIRMWARE__RF24__CHANNEL);

        m_RF24Network.begin(LACAR_DROID_BASESTATION_FIRMWARE__RF24__ADDR);

        m_Status = Status::Ok;
    }

    void MyCom::loop(void) noexcept
    {
        if (m_Status != Status::Ok) return;

        m_RF24Network.update();

        while (m_RF24Network.available())
        {
            // Reads the message from the network.
            RF24NetworkHeader header;
            uint8_t message[256];
            const uint16_t messageSize = m_RF24Network.read(header, message,
                                                            sizeof(message));
        }
    }

    void MyCom::writeRTCMStreamChunk(uint8_t *chunk, uint16_t chunkSize) noexcept
    {
        if (m_Status != Status::Ok) return;

        RF24NetworkHeader header(LACAR_DROID_BASESTATION_FIRMWARE__DROID_NODE_ADDRESS, 
            static_cast<uint8_t>(PacketType::RTCMStreamChunk));
        
        if (!m_RF24Network.write(header, chunk, chunkSize))
        {
            m_Status = Status::WriteFailure;
        }
    }
}