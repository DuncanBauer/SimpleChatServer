#pragma once

#include <iostream>
#include <vector>

namespace net
{
    enum class PacketType {
        // Packets starting with Server_ are packets being sent TO the server
        Server_GetPing,

        Server_Register,
        Server_Login,
        Server_Logout,
        Server_ChangeDisplayName,
        
        Server_CreateServer,
        Server_DeleteServer,

        Server_AddChannelToServer,
        Server_RemoveChannelFromServer,
        Server_JoinServer,
        Server_LeaveServer,
        Server_JoinChannel,
        Server_LeaveChannel,


        // Packets starting with Client_ are packets being sent TO the client
        Client_ClientConnected,
        Client_ClientAccepted,

        Client_Register_Success,
        Client_Register_Fail,
        Client_Login_Success,
        Client_Login_Fail,
        Client_Logout_Success,
        Client_Logout_Fail,
        Client_ChangeDisplayName_Success,
        Client_ChangeDisplayName_Fail,

        Client_CreateServer_Success,
        Client_CreateServer_Fail,
        Client_DeleteServer_Success,
        Client_DeleteServer_Fail,

        Client_AddChannelToServer_Success,
        Client_AddChannelToServer_Fail,
        Client_RemoveChannelFromServer_Success,
        Client_RemoveChannelFromServer_Fail,
        Client_JoinServer_Success,
        Client_JoinServer_Fail,
        Client_LeaveServer_Success,
        Client_LeaveServer_Fail,
        Client_JoinChannel_Success,
        Client_JoinChannel_Fail,
        Client_LeaveChannel_Success,
        Client_LeaveChannel_Fail,
    };

    template <typename T>
    struct PacketHeader
    {
        T id;
        size_t size = 0;
    };

    template <typename T>
    struct Packet
    {
        // Header & Body vector
        PacketHeader<T> header;
        std::vector<unsigned char> body;

        // returns size of entire message packet in bytes
        size_t size() const
        {
            return body.size();
        }

        unsigned char readByte()
        {
            unsigned char result = body.front();
            body.erase(body.begin());
            header.size--;
            return result;
        }

        uint16_t readShort()
        {
            uint16_t r1, r2;
            uint16_t result = 0;

            r1 = readByte();
            r2 = readByte();

            result |= r2 << 8;
            result |= r1;

            return result;
        }

        int16_t readSignedShort()
        {
            int16_t r1, r2;
            int16_t result = 0;

            r1 = readByte();
            r2 = readByte();

            result |= r2 << 8;
            result |= r1;

            return result;
        }

        uint32_t readInt()
        {
            uint32_t r1, r2, r3, r4;
            uint32_t result = 0;

            r1 = readByte();
            r2 = readByte();
            r3 = readByte();
            r4 = readByte();

            result |= r4 << 24;
            result |= r3 << 16;
            result |= r2 << 8;
            result |= r1;

            return result;
        }

        int32_t readSignedInt()
        {
            int32_t r1, r2, r3, r4;
            int32_t result = 0;

            r1 = readByte();
            r2 = readByte();
            r3 = readByte();
            r4 = readByte();

            result |= r4 << 24;
            result |= r3 << 16;
            result |= r2 << 8;
            result |= r1;

            return result;
        }

        uint64_t readLong()
        {
            uint64_t r1, r2, r3, r4,
                     r5, r6, r7, r8;
            uint64_t result = 0;

            r1 = readByte();
            r2 = readByte();
            r3 = readByte();
            r4 = readByte();
            r5 = readByte();
            r6 = readByte();
            r7 = readByte();
            r8 = readByte();

            result |= r8 << 56;
            result |= r7 << 48;
            result |= r6 << 40;
            result |= r5 << 32;
            result |= r4 << 24;
            result |= r3 << 16;
            result |= r2 << 8;
            result |= r1;

            return result;
        }

        int64_t readSignedLong()
        {
            int64_t r1, r2, r3, r4,
                    r5, r6, r7, r8;
            int64_t result = 0;

            r1 = readByte();
            r2 = readByte();
            r3 = readByte();
            r4 = readByte();
            r5 = readByte();
            r6 = readByte();
            r7 = readByte();
            r8 = readByte();

            result |= r8 << 56;
            result |= r7 << 48;
            result |= r6 << 40;
            result |= r5 << 32;
            result |= r4 << 24;
            result |= r3 << 16;
            result |= r2 << 8;
            result |= r1;

            return result;
        }

        std::string readString(int length)
        {
            std::string data;
            for (int i = 0; i < length; i++)
                data += readByte();
            return data;
        }

        void writeByte(unsigned char data)
        {
            body.push_back(data);
            header.size++;
        }

        void writeShort(uint16_t data)
        {
            writeByte(data & 0xFF);
            writeByte((data >> 8) & 0xFF);
        }

        void writeSignedShort(int16_t data)
        {
            writeByte(data & 0xFF);
            writeByte((data >> 8) & 0xFF);
        }

        void writeInt(uint32_t data)
        {
            writeByte(data & 0xFF);
            writeByte((data >> 8) & 0xFF);
            writeByte((data >> 16) & 0xFF);
            writeByte((data >> 24) & 0xFF);
        }

        void writeSignedInt(int32_t data)
        {
            writeByte(data & 0xFF);
            writeByte((data >> 8) & 0xFF);
            writeByte((data >> 16) & 0xFF);
            writeByte((data >> 24) & 0xFF);
        }

        void writeLong(uint64_t data)
        {
            writeByte(data & 0xFF);
            writeByte((data >> 8) & 0xFF);
            writeByte((data >> 16) & 0xFF);
            writeByte((data >> 24) & 0xFF);
            writeByte((data >> 32) & 0xFF);
            writeByte((data >> 40) & 0xFF);
            writeByte((data >> 48) & 0xFF);
            writeByte((data >> 56) & 0xFF);
        }

        void writeSignedLong(int64_t data)
        {
            writeByte(data & 0xFF);
            writeByte((data >> 8) & 0xFF);
            writeByte((data >> 16) & 0xFF);
            writeByte((data >> 24) & 0xFF);
            writeByte((data >> 32) & 0xFF);
            writeByte((data >> 40) & 0xFF);
            writeByte((data >> 48) & 0xFF);
            writeByte((data >> 56) & 0xFF);
        }

        void writeString(std::string data)
        {
            for (unsigned char c : data)
                writeByte(c);
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