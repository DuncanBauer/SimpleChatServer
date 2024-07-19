#pragma once

namespace net
{
    enum class PacketType {
        Server_GetStatus,
        Server_GetPing,

        Client_Accepted,
        Client_AssignID,
        Client_ConnectToServer,
        Client_DisconnectFromServer,
    };

    template <typename T>
    struct PacketHeader
    {
        T id{};
        size_t size = 0;
    };

    template <typename T>
    struct Packet
    {
        // Header & Body vector
        PacketHeader<T> header{};
        std::vector<uint8_t> body;

        // returns size of entire message packet in bytes
        size_t size() const
        {
            return body.size();
        }

        // Override for std::cout compatibility - produces friendly description of message
        template<typename T>
        friend std::ostream& operator<<(std::ostream& os, const Packet<T>& packet)
        {
            os << "ID:" << int(packet.header.id) << " Size:" << packet.header.size;
            return os;
        }

        // Pushes any POD-like data into the message buffer
        template<typename DataType>
        friend Packet<T>& operator<<(Packet<T>& packet, const DataType& data)
        {
            // Check that the type of the data being pushed is trivially copyable
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

            // Cache current size of vector, as this will be the point we insert the data
            size_t i = packet.body.size();

            // Resize the vector by the size of the data being pushed
            packet.body.resize(packet.body.size() + sizeof(DataType));

            // Physically copy the data into the newly allocated vector space
            std::memcpy(packet.body.data() + i, &data, sizeof(DataType));

            // Recalculate the message size
            packet.header.size = packet.size();

            // Return the target message so it can be "chained"
            return packet;
        }

        // Pulls any POD-like data form the message buffer
        template<typename DataType>
        friend Packet<T>& operator>>(Packet<T>& packet, DataType& data)
        {
            // Check that the type of the data being pushed is trivially copyable
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

            // Cache the location towards the end of the vector where the pulled data starts
            size_t i = packet.body.size() - sizeof(DataType);

            // Physically copy the data from the vector into the user variable
            std::memcpy(&data, packet.body.data() + i, sizeof(DataType));

            // Shrink the vector to remove read bytes, and reset end position
            packet.body.resize(i);

            // Recalculate the message size
            packet.header.size = packet.size();

            // Return the target message so it can be "chained"
            return packet;
        }
    };

    // Forward declare the connection
    template <typename T>
    class TCPConnection;

    template <typename T>
    struct OwnedPacket
    {
        std::shared_ptr<TCPConnection<T>> remote = nullptr;
        Packet<T> packet;

        // Friendly string maker
        friend std::ostream& operator<<(std::ostream& os, const OwnedPacket<T>& packet)
        {
            os << packet.packet;
            return os;
        }
    };
}