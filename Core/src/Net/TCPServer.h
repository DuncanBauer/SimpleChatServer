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
            m_packetHandlers[PacketType::Server_Get_Ping]       = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleGetPing(client, packet); };
            m_packetHandlers[PacketType::Server_Register]       = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleRegister(client, packet); };
            m_packetHandlers[PacketType::Server_Login]          = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleLogin(client, packet); };
            m_packetHandlers[PacketType::Server_Logout]         = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleLogout(client, packet); };
            m_packetHandlers[PacketType::Server_CreateServer]   = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleCreateServer(client, packet); };
            m_packetHandlers[PacketType::Server_DeleteServer]   = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleDeleteServer(client, packet); };
            m_packetHandlers[PacketType::Server_CreateChannel]  = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleCreateChannel(client, packet); };
            m_packetHandlers[PacketType::Server_DeleteChannel]  = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleDeleteChannel(client, packet); };
            m_packetHandlers[PacketType::Server_JoinServer]     = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleJoinServer(client, packet); };
            m_packetHandlers[PacketType::Server_LeaveServer]    = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleLeaveServer(client, packet); };
            m_packetHandlers[PacketType::Server_SendMessage]    = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleSendMessage(client, packet); };
            m_packetHandlers[PacketType::Server_DeleteMessage]  = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleDeleteMessage(client, packet); };
            m_packetHandlers[PacketType::Server_EditMessage]    = [this](clientConnection& client, Packet<PacketType>& packet) { this->handleEditMessage(client, packet); };
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

            Packet<PacketType> retPacket;
            if (m_dbHandler.createUser(username, password))
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

            Packet<PacketType> retPacket;
            if (m_dbHandler.login(username, password))
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

            uint32_t userIdSize = 0;
            std::string userId;

            userIdSize = packet.readInt();
            userId = packet.readString(userIdSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.logout(userId))
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

            uint32_t userIdSize = packet.readInt();
            std::string userId = packet.readString(userIdSize);
            uint32_t serverNameSize = packet.readInt();
            std::string serverName = packet.readString(serverNameSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.createServer(serverName, userId))
                retPacket.header.id = PacketType::Client_CreateServer_Success;
            else
                retPacket.header.id = PacketType::Client_CreateServer_Fail;

            client->send(retPacket);
        }

        void handleDeleteServer(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Delete Server", client->getID());

            uint32_t serverIdSize = packet.readInt();
            std::string serverId = packet.readString(serverIdSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.deleteServer(serverId))
                retPacket.header.id = PacketType::Client_DeleteServer_Success;
            else
                retPacket.header.id = PacketType::Client_DeleteServer_Fail;

            client->send(retPacket);
        }
        
        void handleCreateChannel(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Create Channel", client->getID());

            uint32_t serverIdSize = packet.readInt();
            std::string serverId = packet.readString(serverIdSize);
            uint32_t channelNameSize = packet.readInt();
            std::string channelName = packet.readString(channelNameSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.createChannel(serverId, channelName))
                retPacket.header.id = PacketType::Client_CreateChannel_Success;
            else
                retPacket.header.id = PacketType::Client_CreateChannel_Fail;

            client->send(retPacket);
        }
        
        void handleDeleteChannel(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Delete Channel", client->getID());

            uint32_t serverIdSize = packet.readInt();
            std::string serverId = packet.readString(serverIdSize);
            uint32_t channelIdSize = packet.readInt();
            std::string channelId = packet.readString(channelIdSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.deleteChannel(serverId, channelId))
                retPacket.header.id = PacketType::Client_DeleteChannel_Success;
            else
                retPacket.header.id = PacketType::Client_DeleteChannel_Fail;

            client->send(retPacket);
        }
        
        void handleJoinServer(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Join Server", client->getID());

            uint32_t userIdSize = packet.readInt();
            std::string userId = packet.readString(userIdSize);
            uint32_t serverIdSize = packet.readInt();
            std::string serverId = packet.readString(serverIdSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.joinServer(serverId, userId))
                retPacket.header.id = PacketType::Client_JoinServer_Success;
            else
                retPacket.header.id = PacketType::Client_JoinServer_Fail;

            client->send(retPacket);
        }
        
        void handleLeaveServer(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Leave Server", client->getID());

            uint32_t userIdSize = packet.readInt();
            std::string userId = packet.readString(userIdSize);
            uint32_t serverIdSize = packet.readInt();
            std::string serverId = packet.readString(serverIdSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.leaveServer(serverId, userId))
                retPacket.header.id = PacketType::Client_LeaveServer_Success;
            else
                retPacket.header.id = PacketType::Client_LeaveServer_Fail;

            client->send(retPacket);
        }
        
        void handleSendMessage(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Send Message", client->getID());

            uint32_t authorIdSize = packet.readInt();
            std::string authorId = packet.readString(authorIdSize);
            uint32_t channelIdSize = packet.readInt();
            std::string channelId = packet.readString(channelIdSize);
            uint32_t contentSize = packet.readInt();
            std::string content = packet.readString(contentSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.sendMessage(authorId, channelId, content))
                retPacket.header.id = PacketType::Client_SendMessage_Success;
            else
                retPacket.header.id = PacketType::Client_SendMessage_Fail;

            client->send(retPacket);
        }
        
        void handleDeleteMessage(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Delete Message", client->getID());

            uint32_t channelIdSize = packet.readInt();
            std::string channelId = packet.readString(channelIdSize);
            uint32_t messageIdSize = packet.readInt();
            std::string messageId = packet.readString(messageIdSize);
            
            Packet<PacketType> retPacket;
            if (m_dbHandler.deleteMessage(channelId, messageId))
                retPacket.header.id = PacketType::Client_DeleteMessage_Success;
            else
                retPacket.header.id = PacketType::Client_DeleteMessage_Fail;

            client->send(retPacket);
        }
        
        void handleEditMessage(clientConnection & client, Packet<PacketType>&packet)
        {
            SERVER_INFO("[{}]: Edit Message", client->getID());

            uint32_t messageIdSize = packet.readInt();
            std::string messageId = packet.readString(messageIdSize);
            uint32_t contentSize = packet.readInt();
            std::string content = packet.readString(contentSize);

            Packet<PacketType> retPacket;
            if (m_dbHandler.editMessage(messageId, content))
                retPacket.header.id = PacketType::Client_DeleteMessage_Success;
            else
                retPacket.header.id = PacketType::Client_DeleteMessage_Fail;

            client->send(retPacket);
        }

    private:
        functionMap m_packetHandlers;
        MongoDbHandler m_dbHandler;
    };
}