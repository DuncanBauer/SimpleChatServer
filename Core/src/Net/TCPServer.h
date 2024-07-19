#pragma once

#include "TCPServerInterface.h"
#include "TCPConnection.h"

namespace net
{
    class TCPServer : public TCPServerInterface<PacketType>
    {
    public:
        TCPServer(uint16_t port) : TCPServerInterface<PacketType>(port)
        {}

        ~TCPServer()
        {}

    protected:
        bool onClientConnect(std::shared_ptr<TCPConnection<PacketType>> client) override
        {
            return true;
        }

        void onClientDisconnect(std::shared_ptr<TCPConnection<PacketType>> client) override
        {
            if (client)
            {
            }
        }

        void onClientValidated(std::shared_ptr<TCPConnection<PacketType>> client) override
        {
            // Client passed validation, so send them a packet to inform them they can communicate
            Packet<PacketType> packet;
            packet.header.id = PacketType::Client_Accepted;
            client->send(packet);
        }

        void onMessage(std::shared_ptr<TCPConnection<PacketType>> client, Packet<PacketType>& packet) override
        {
            //if (!m_badIDs.empty()) {
            //    for (auto playerID : m_badIDs) {
            //        Packet<PacketType> packet;
            //        packet.header.id = PacketType::Client_DisconnectFromServer;
            //        packet << playerID;
            //        std::cout << "Removing " << playerID << '\n';
            //        messageAllClients(packet);
            //    }
            //    m_badIDs.clear();
            //}

            switch (packet.header.id)
            {
            case PacketType::Server_GetPing:
                spdlog::info("[{}]: Server Ping", client->getID());

                // Bounce ping back to client
                client->send(packet);
                break;
            default: break;
            }
        }

    private:
        //std::vector<uint32_t> m_badIDs;
    };
}