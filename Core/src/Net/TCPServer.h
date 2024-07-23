#pragma once

#include "TCPServerInterface.h"
#include "TCPConnection.h"
#include "MongoDbHandler.h"

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
            m_packetHandlers[PacketType::Server_Get_Ping] = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleGetPing(client, packet); };
            m_packetHandlers[PacketType::Server_Register] = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleRegister(client, packet); };
            m_packetHandlers[PacketType::Server_Login]    = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleLogin(client, packet); };
            m_packetHandlers[PacketType::Server_Logout]   = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleLogout(client, packet); };

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

        MongoDbHandler& getDbHandler()
        {
            return m_dbHandler;
        }

    protected:
        bool onClientConnect(clientConnection client) override
        {
            // Client passed validation, so send them a packet to inform them they can communicate
            Packet<PacketType> packet;
            packet.header.id = PacketType::Client_Connected;
            client->send(packet);
            return true;
        }

        void onClientDisconnect(clientConnection client) override
        {
        }

        void onMessage(clientConnection client, Packet<PacketType>& packet) override
        {
            // Make sure the client is logged in before handling any packets other than Login or Register
            if (packet.header.id != PacketType::Server_Register && packet.header.id != PacketType::Server_Login)
            {
                if(client->getClientState() == ClientState::AUTHED_LOGGEDIN)
                {
                    m_packetHandlers[packet.header.id](client, packet);
                }
            }
            else
            {
                m_packetHandlers[packet.header.id](client, packet);
            }
        }

        void handleGetPing(clientConnection& client, Packet<PacketType>& packet)
        {
            SERVER_INFO("[{}]: Server Ping", client->getID());
            Packet<PacketType> retPacket;
            retPacket.header.id = PacketType::Client_Return_Ping;
            client->send(retPacket);
        }

        void handleRegister(clientConnection& client, Packet<PacketType>& packet)
        {
            SERVER_INFO("[{}]: Register", client->getID());

            uint32_t usernameSize = packet.readInt();
            std::string username = packet.readString(usernameSize);
            uint32_t passwordSize = packet.readInt();
            std::string password = packet.readString(passwordSize);

            bool success = m_dbHandler.createUser(username, password);

            Packet<PacketType> retPacket;
            if (success)
                retPacket.header.id = PacketType::Client_Register_Success;
            else
                retPacket.header.id = PacketType::Client_Register_Fail;
            client->send(retPacket);
        }

        void handleLogin(clientConnection& client, Packet<PacketType>& packet)
        {
            SERVER_INFO("[{}]: Login", client->getID());

            uint32_t usernameSize = packet.readInt();
            std::string username = packet.readString(usernameSize);
            uint32_t passwordSize = packet.readInt();
            std::string password = packet.readString(passwordSize);

            bool success = m_dbHandler.login(username, password);

            Packet<PacketType> retPacket;
            if (success)
            {
                retPacket.header.id = PacketType::Client_Login_Success;
                client->updateClientState(ClientState::AUTHED_LOGGEDIN);
            }
            else
            {
                retPacket.header.id = PacketType::Client_Login_Fail;
            }
            client->send(retPacket);
        }

        void handleLogout(clientConnection& client, Packet<PacketType>& packet)
        {
            spdlog::info("[{}]: Logout", client->getID());

            uint32_t usernameSize = 0;
            std::string username;

            usernameSize = packet.readInt();
            username = packet.readString(usernameSize);

            bool success = m_dbHandler.logout(username);

            Packet<PacketType> retPacket;
            if (success)
            {
                retPacket.header.id = PacketType::Client_Logout_Success;
                client->updateClientState(ClientState::NOT_AUTHED);
            }
            else
            {
                retPacket.header.id = PacketType::Client_Logout_Fail;
            }
            client->send(retPacket);
        }

        void handleCreateServer(clientConnection& client, Packet<PacketType>& packet)
        {
            SERVER_INFO("[{}]: Create Server", client->getID());
        }

        void handleDeleteServer(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Delete Server", client->getID());
        }
        
        void handleCreateChannel(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Create Channel", client->getID());
        }
        
        void handleDeleteChannel(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Delete Channel", client->getID());
        }
        
        void handleJoinServer(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Join Server", client->getID());
        }
        
        void handleLeaveServer(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Leave Server", client->getID());
        }
        
        void handleSendMessage(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Send Message", client->getID());
        }
        
        void handleDeleteMessage(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Delete Message", client->getID());
        }
        
        void handleEditMessage(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Edit Message", client->getID());
        }


    private:
        functionMap m_packetHandlers;
        MongoDbHandler m_dbHandler;
    };
}