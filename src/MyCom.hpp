#pragma once

#include <RF24.h>
#include <RF24Network.h>

namespace lacar::droid_basestation::firmware
{
    class MyCom
    {
    public:
        enum class ErrorCause : uint8_t
        {
            Ok = 0,
            PeripheralBeginFailed = 1,
            PeripheralMulticastFailed = 2,
        };

        enum class State : uint8_t
        {
            Idle = 0,
            Running = 1,
            Error = 2,
        };

        enum class PacketType : uint8_t
        {
            RTCMStreamChunk = 0,
        };

    private:
        static MyCom s_Instance;

    public:
        static inline MyCom &getInstance(void) noexcept
        {
            return s_Instance;
        }

    private:
        RF24 peripheral_;
        RF24Network network_;
        ErrorCause errorCause_;
        State state_;

    public:
        /// @brief Constructs a new com instance.
        MyCom(void) noexcept;

    public:
        /// @brief Gets the current state of the com.
        /// @return The current state of the com.
        inline const State &getState(void) noexcept
        {
            return this->state_;
        }

        inline const ErrorCause &getErrorCause(void) noexcept
        {
            return this->errorCause_;
        }

    private:
        // Idle state methods.

        /// @brief Entry of the idle state.
        void idleEntry(void) noexcept;

        /// @brief Do of the idle state.
        void idleDo(void) noexcept;

        /// @brief Exit of the idle state.
        void idleExit(void) noexcept;

        // Running state methods.

        /// @brief Entry of the running state.
        void runningEntry(void) noexcept;

        /// @brief Do of the running state.
        void runningDo(void) noexcept;

        /// @brief Exit of the running state.
        void runningExit(void) noexcept;

        // Error state methods.

        /// @brief Entry of the error state.
        void errorEntry(void) noexcept;

        /// @brief Do of the error state.
        void errorDo(void) noexcept;

        /// @brief Exit of the error state.
        void errorExit(void) noexcept;

        // Other private methods.

        /// @brief Transitions to the given state.
        /// @param state the state to transition to.
        void transition(State state) noexcept;

        /// @brief Entry of the current state.
        void currentStateEntry(void) noexcept;

        /// @brief Do of the current state.
        void currentStateDo(void) noexcept;

        /// @brief Exit of the current state.
        void currentStateExit(void) noexcept;

    public:
        /// @brief Performs the setup of the com.
        void setup(void) noexcept;

        /// @brief Performs the loop of the com.
        void loop(void) noexcept;

        /// @brief Enables the COM.
        void enable(void) noexcept;

        /// @brief Writes the given RTCM chunk to the stream.
        /// @param chunk the chunk.
        /// @param chunkSize the size of the chunk.
        void writeRTCMStreamChunk(uint8_t *chunk, uint16_t chunkSize) noexcept;
    };
}