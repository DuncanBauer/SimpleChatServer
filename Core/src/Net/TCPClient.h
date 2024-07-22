#pragma once

#include <iostream>

#include "TCPNet.h"
#include "TCPClientInterface.h"

namespace net
{
    class TCPClient : public TCPClientInterface<PacketType>
    {
    public:
        //TCPClient();
        //~TCPClient();

        //void start();
        //void stop();
        //void update();

        //void pingServer();
        TCPClient()
        {}

        ~TCPClient()
        {}

        void start()
        {}

        void stop()
        {}

        void update()
        {}

        void pingServer()
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_GetPing;

            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
            auto now_se = now_ms.time_since_epoch();
            long long now_value = now_se.count();

            packet.writeLong(now_value);
            send(packet);
        }

        void tryLogin(std::string username, std::string password)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_Login;

            packet.writeInt((uint32_t)username.size());
            packet.writeString(username);
            packet.writeInt((uint32_t)password.size());
            packet.writeString(password);

            send(packet);
        }

        void tryRegister(std::string username, std::string password)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_Register;

            packet.writeInt((uint32_t)username.size());
            packet.writeString(username);
            packet.writeInt((uint32_t)password.size());
            packet.writeString(password);
            
            send(packet);
        }

    private:
    };
}