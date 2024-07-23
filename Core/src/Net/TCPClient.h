#pragma once

#include <iostream>

#include "logging/Logger.h"
#include "TCPClientInterface.h"

namespace net
{
    struct ClientStatus
    {
        bool loggedIn = false;
    };

    class TCPClient : public TCPClientInterface<PacketType>
    {
        typedef std::function<void(Packet<PacketType>&)> fnPointer; // function pointer type
        typedef std::unordered_map<PacketType, fnPointer> functionMap;

    public:
        TCPClient()
        {
            m_packetHandlers[PacketType::Client_Return_Ping]      = [this](Packet<PacketType>& packet) { this->handleReturnPing(packet); };
            m_packetHandlers[PacketType::Client_Connected]        = [this](Packet<PacketType>& packet) { this->handleConnected(packet); };
            m_packetHandlers[PacketType::Client_Register_Success] = [this](Packet<PacketType>& packet) { this->handleRegisterSuccess(packet); };
            m_packetHandlers[PacketType::Client_Register_Fail]    = [this](Packet<PacketType>& packet) { this->handleRegisterFail(packet); };
            m_packetHandlers[PacketType::Client_Login_Success]    = [this](Packet<PacketType>& packet) { this->handleLoginSuccess(packet); };
            m_packetHandlers[PacketType::Client_Login_Fail]       = [this](Packet<PacketType>& packet) { this->handleLoginFail(packet); };
            m_packetHandlers[PacketType::Client_Logout_Success]   = [this](Packet<PacketType>& packet) { this->handleLogoutSuccess(packet); };
            m_packetHandlers[PacketType::Client_Logout_Fail]      = [this](Packet<PacketType>& packet) { this->handleLogoutFail(packet); };
        }

        ~TCPClient()
        {}

        void start()
        {}

        void stop()
        {}

        ClientStatus getStatus()
        {
            return m_clientStatus;
        }

        void onMessage(Packet<PacketType>& packet) override
        {
            if (m_packetHandlers.find(packet.header.id) != m_packetHandlers.end()) 
            {
                auto& handler = m_packetHandlers[packet.header.id];
                if (handler) 
                {
                    handler(packet);
                }
                else 
                {
                    CLIENT_ERROR("Handler for packet ID {} is invalid!", static_cast<int>(packet.header.id));
                }
            }
            else 
            {
                CLIENT_ERROR("No handler found for packet ID {}", static_cast<int>(packet.header.id));
            }
            //m_packetHandlers[packet.header.id](packet);
        }

        void pingServer()
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_Get_Ping;

            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
            auto now_se = now_ms.time_since_epoch();
            long long now_value = now_se.count();

            packet.writeLong(now_value);

            send(packet);
        }

        void tryLogin(const std::string& username, const std::string& password)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_Login;

            packet.writeInt((uint32_t)username.size());
            packet.writeString(username);
            packet.writeInt((uint32_t)password.size());
            packet.writeString(password);

            send(packet);
        }

        void tryRegister(const std::string& username, const std::string& password)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_Register;

            packet.writeInt((uint32_t)username.size());
            packet.writeString(username);
            packet.writeInt((uint32_t)password.size());
            packet.writeString(password);
            
            send(packet);
        }

        void tryCreateServer(const std::string& userId, const std::string& serverName)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_CreateServer;

            packet.writeInt((uint32_t)userId.size());
            packet.writeString(userId);
            packet.writeInt((uint32_t)serverName.size());
            packet.writeString(serverName);

            send(packet);
        }

        void tryDeleteServer(const std::string& serverId)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_DeleteServer;

            packet.writeInt((uint32_t)serverId.size());
            packet.writeString(serverId);

            send(packet);
        }

        void tryCreateChannel(const std::string& serverId, const std::string& channelName)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_CreateChannel;

            packet.writeInt((uint32_t)serverId.size());
            packet.writeString(serverId);
            packet.writeInt((uint32_t)channelName.size());
            packet.writeString(channelName);

            send(packet);
        }

        void tryDeleteChannel(const std::string& channelId)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_DeleteChannel;

            packet.writeInt((uint32_t)channelId.size());
            packet.writeString(channelId);

            send(packet);
        }

        void tryJoinServer(const std::string& userId, const std::string& serverId)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_JoinServer;

            packet.writeInt((uint32_t)userId.size());
            packet.writeString(userId);
            packet.writeInt((uint32_t)serverId.size());
            packet.writeString(serverId);

            send(packet);
        }

        void tryLeaveServer(const std::string& userId, const std::string& serverId)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_LeaveServer;

            packet.writeInt((uint32_t)userId.size());
            packet.writeString(userId);
            packet.writeInt((uint32_t)serverId.size());
            packet.writeString(serverId);

            send(packet);
        }

        void trySendMessage(const std::string& authorId, const std::string& channelId, const std::string& messageContent)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_SendMessage;

            packet.writeInt((uint32_t)authorId.size());
            packet.writeString(authorId);
            packet.writeInt((uint32_t)channelId.size());
            packet.writeString(channelId);
            packet.writeInt((uint32_t)messageContent.size());
            packet.writeString(messageContent);

            send(packet);
        }

        void tryDeleteMessage(const std::string& channelId, const std::string& messageId)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_DeleteMessage;

            packet.writeInt((uint32_t)channelId.size());
            packet.writeString(channelId);
            packet.writeInt((uint32_t)messageId.size());
            packet.writeString(messageId);

            send(packet);
        }

        void tryEditMessage(const std::string& messageId, const std::string& content)
        {
            Packet<PacketType> packet;
            packet.header.id = PacketType::Server_EditMessage;

            packet.writeInt((uint32_t)messageId.size());
            packet.writeString(messageId);
            packet.writeInt((uint32_t)content.size());
            packet.writeString(content);

            send(packet);
        }



        void handleReturnPing(Packet<PacketType>& packet)
        {
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            long long then = packet.readLong();
            std::chrono::milliseconds then_ms(then);
            std::chrono::time_point<std::chrono::system_clock> then_sc(then_ms);
            CLIENT_INFO("Ping: {}", std::chrono::duration<double>(now - then_sc).count());
        }

        void handleConnected(Packet<PacketType>& packet)
        {
            CLIENT_INFO("Connected to server");
        }

        void handleRegisterSuccess(Packet<PacketType>& packet)
        {
            CLIENT_INFO("Register Success!");
        }

        void handleRegisterFail(Packet<PacketType>& packet)
        {
            CLIENT_INFO("Register Fail!");
        }

        void handleLoginSuccess(Packet<PacketType>& packet)
        {
            m_clientStatus.loggedIn = true;
            CLIENT_INFO("Login Success!");
        }

        void handleLoginFail(Packet<PacketType>& packet)
        {
            CLIENT_INFO("Login Fail!");
        }

        void handleLogoutSuccess(Packet<PacketType>& packet)
        {
            m_clientStatus.loggedIn = false;
            CLIENT_INFO("Login Success!");
        }

        void handleLogoutFail(Packet<PacketType>& packet)
        {
            CLIENT_INFO("Login Fail!");
        }

    private:
        functionMap m_packetHandlers;
        ClientStatus m_clientStatus;
    };
}