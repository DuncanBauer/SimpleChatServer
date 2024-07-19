#pragma once

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
            packet << now;
            send(packet);
        }

    private:
    };
}