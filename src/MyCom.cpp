#include "MyCom.hpp"
#include "config.hpp"

namespace lacar::droid_basestation::firmware
{
    MyCom MyCom::s_Instance;

    /// @brief Constructs a new com instance.
    MyCom::MyCom(void) noexcept : peripheral_(LACAR_DROID_BASESTATION_FIRMWARE__RF24__CE, LACAR_DROID_BASESTATION_FIRMWARE__RF24__CS),
                                  network_(peripheral_),
                                  errorCause_(ErrorCause::Ok)
    {
    }

    // Idle state methods.

    /// @brief Entry of the idle state.
    void MyCom::idleEntry(void) noexcept
    {
    }

    /// @brief Do of the idle state.
    void MyCom::idleDo(void) noexcept
    {
    }

    /// @brief Exit of the idle state.
    void MyCom::idleExit(void) noexcept
    {
    }

    // Running state methods.

    /// @brief Entry of the running state.
    void MyCom::runningEntry(void) noexcept
    {
    }

    /// @brief Do of the running state.
    void MyCom::runningDo(void) noexcept
    {
        RF24NetworkHeader header;
        uint8_t message[128];
        uint16_t messageSize;

        // Updates the network.
        network_.update();

        // Stays in loop as long as messages are available.
        while (network_.available())
        {
            // Reads the available message from the network.
            messageSize = network_.read(header, message, sizeof(message));

            // Checks the message type and calls the appropriate method.
            switch (static_cast<PacketType>(header.type))
            {
            default:
                break;
            }
        }
    }

    /// @brief Exit of the running state.
    void MyCom::runningExit(void) noexcept
    {
    }

    // Error state methods.

    /// @brief Entry of the error state.
    void MyCom::errorEntry(void) noexcept
    {
    }

    /// @brief Do of the error state.
    void MyCom::errorDo(void) noexcept
    {
    }

    /// @brief Exit of the error state.
    void MyCom::errorExit(void) noexcept
    {
    }

    // Other private methods.

    /// @brief Transitions to the given state.
    /// @param state the state to transition to.
    void MyCom::transition(State state) noexcept
    {
        currentStateExit();
        this->state_ = state;
        currentStateEntry();
    }

    /// @brief Entry of the current state.
    void MyCom::currentStateEntry(void) noexcept
    {
        switch (this->state_)
        {
        case State::Idle:
            idleEntry();
            break;
        case State::Running:
            runningEntry();
            break;
        case State::Error:
            errorEntry();
            break;
        default:
            break;
        }
    }

    /// @brief Do of the current state.
    void MyCom::currentStateDo(void) noexcept
    {
        switch (this->state_)
        {
        case State::Idle:
            idleDo();
            break;
        case State::Running:
            runningDo();
            break;
        case State::Error:
            errorDo();
            break;
        default:
            break;
        }
    }

    /// @brief Exit of the current state.
    void MyCom::currentStateExit(void) noexcept
    {
        switch (this->state_)
        {
        case State::Idle:
            idleExit();
            break;
        case State::Running:
            runningExit();
            break;
        case State::Error:
            errorExit();
            break;
        default:
            break;
        }
    }

    /// @brief Enables the COM.
    void MyCom::enable(void) noexcept
    {
        // Do not enable if not in idle mode.
        if (this->state_ != State::Idle)
            return;

        // Transitions to the running state.
        this->transition(State::Running);
    }

    /// @brief Performs the setup of the com.
    void MyCom::setup(void) noexcept
    {
        // Begins the peripheral and makes the initial state the error
        //  state if it fails.
        if (!peripheral_.begin())
        {
            this->errorCause_ = ErrorCause::PeripheralBeginFailed;
            this->state_ = State::Error;
            this->currentStateEntry();
            return;
        }

        // Enables relaying.
        this->network_.multicastRelay = true;

        // Begins the network.
        this->network_.begin(LACAR_DROID_BASESTATION_FIRMWARE__RF24__CHANNEL,
                       LACAR_DROID_BASESTATION_FIRMWARE__RF24__ADDR);
        
        // Sets the network level.
        this->network_.multicastLevel(0U);

        // Sets the current state to idle and enters it.
        this->state_ = State::Idle;
        this->currentStateEntry();
    }

    /// @brief Performs the loop of the com.
    void MyCom::loop(void) noexcept
    {
        // Calls the do of the current state.
        currentStateDo();
    }

    /// @brief Writes the given RTCM chunk to the stream.
    /// @param chunk the chunk.
    /// @param chunkSize the size of the chunk.
    void MyCom::writeRTCMStreamChunk(uint8_t *chunk, uint16_t chunkSize) noexcept
    {
        // Don't write if we're not in the enabled state.
        if (this->state_ != State::Running)
        {
            Serial.println(F("Not writing RTCM stream chunk, not running"));
            return;
        }

        // Builds the header.
        RF24NetworkHeader header(NETWORK_MULTICAST_ADDRESS, static_cast<uint8_t>(PacketType::RTCMStreamChunk));

        // Writes the message to the droids.
        if (!network_.multicast(header, chunk, chunkSize))
        {
            Serial.println(F("RF24 network RTCM chunk multicast failed"));
            this->errorCause_ = ErrorCause::PeripheralMulticastFailed;
            this->transition(State::Error);
        }
    }
}