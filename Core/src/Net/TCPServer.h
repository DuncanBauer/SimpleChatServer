#pragma once

#include "Global.h"

#include "TCPServerInterface.h"
#include "TCPConnection.h"
#include "TCPNet.h"

namespace net
{
    class TCPServer : public TCPServerInterface<PacketType>
    {
        typedef std::shared_ptr<TCPConnection<PacketType>> clientConnection;
        typedef std::function<void(clientConnection&, Packet<PacketType>&)> fnPointer; // function pointer type
        typedef std::unordered_map<PacketType, fnPointer> functionMap;

    public:
        TCPServer(uint16_t port) : TCPServerInterface<PacketType>(port)
        {
            //Register Packet Handlers
            m_packetHandlers[PacketType::Server_GetPing] = [this](clientConnection& client, Packet<PacketType>& packet) {
                this->handleGetPing(client, packet);
            };
            m_packetHandlers[PacketType::Server_Register] = [this](clientConnection& client, Packet<PacketType>& packet) {
                this->handleRegister(client, packet);
            };
            m_packetHandlers[PacketType::Server_Login] = [this](clientConnection& client, Packet<PacketType>& packet) {
                this->handleLogin(client, packet);
            };
            //m_packetHandlers[PacketType::Server_Logout] = ;
            //m_packetHandlers[PacketType::Server_ChangeDisplayName] = ;

            //m_packetHandlers[PacketType::Server_CreateServer] = ;
            //m_packetHandlers[PacketType::Server_DeleteServer] = ;

            //m_packetHandlers[PacketType::Server_AddChannelToServer] = ;
            //m_packetHandlers[PacketType::Server_RemoveChannelFromServer] = ;
            //m_packetHandlers[PacketType::Server_JoinServer] = ;
            //m_packetHandlers[PacketType::Server_LeaveServer] = ;
            //m_packetHandlers[PacketType::Server_JoinChannel] = ;
            //m_packetHandlers[PacketType::Server_LeaveChannel] = ;
        }

        ~TCPServer()
        {}

    protected:
        bool onClientConnect(clientConnection client) override
        {
            // Client passed validation, so send them a packet to inform them they can communicate
            Packet<PacketType> packet;
            packet.header.id = PacketType::Client_ClientConnected;
            client->send(packet);
            return true;
        }

        void onClientDisconnect(clientConnection client) override
        {
            if (client)
            {
            }
        }

        void onClientValidated(clientConnection client) override
        {
            // Client passed validation, so send them a packet to inform them they can communicate
            Packet<PacketType> packet;
            packet.header.id = PacketType::Client_ClientAccepted;
            client->send(packet);
        }

        void onMessage(clientConnection client, Packet<PacketType>& packet) override
        {
            m_packetHandlers[packet.header.id](client, packet);
        }

        void handleGetPing(clientConnection& client, Packet<PacketType>& packet)
        {
            spdlog::info("[{}]: Server Ping", client->getID());
            client->send(packet);
        }

        void handleRegister(clientConnection& client, Packet<PacketType>& packet)
        {
            spdlog::info("[{}]: Register", client->getID());

            uint32_t usernameSize = 0;
            uint32_t passwordSize = 0;
            std::string username;
            std::string password;

            usernameSize = packet.readInt();
            username = packet.readString(usernameSize);
            passwordSize = packet.readInt();
            password = packet.readString(passwordSize);

            handler.registerUser(username, password);
        }

        void handleLogin(clientConnection& client, Packet<PacketType>& packet)
        {
            spdlog::info("[{}]: Login", client->getID());

            uint32_t usernameSize = 0;
            uint32_t passwordSize = 0;
            std::string username;
            std::string password;

            usernameSize = packet.readInt();
            username = packet.readString(usernameSize);
            passwordSize = packet.readInt();
            password = packet.readString(passwordSize);

            handler.login(username, password);
        }

    private:
        functionMap m_packetHandlers;
    };
}