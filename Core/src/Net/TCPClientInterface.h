#pragma once

#include <asio.hpp>
#include "spdlog/spdlog.h"

#include "ThreadSafeQueue.h"
#include "TCPNet.h"
#include "TCPConnection.h"

namespace net
{
    using asio::ip::tcp;

    template<typename T>
    class TCPClientInterface
    {
    public:
        TCPClientInterface() = default;

        ~TCPClientInterface()
        {
            disconnect();
        }

        bool connect(std::string& host, uint16_t port)
        {
            try
            {
                // Resolve hostname/ip-address into tangiable physical address
                tcp::resolver resolver(m_ioContext);
                tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

                // Create connection
                m_connection = std::make_unique<TCPConnection<T>>(TCPConnection<T>::Owner::Client,
                    m_ioContext,
                    tcp::socket(m_ioContext),
                    m_incomingPackets);

                // Tell the connection object to connect to server
                m_connection->connectToServer(endpoints);

                // Start Context Thread
                m_threadContext = std::thread([this]() { m_ioContext.run(); });
            }
            catch (std::exception& e)
            {
                spdlog::error("Client Exception: {}", e.what());
                return false;
            }
            return true;
        }

        void disconnect()
        {
            // If connection exists, and it's connected then...
            if (isConnected())
                // ...disconnect from server gracefully
                m_connection->disconnect();

            // Either way, we're also done with the asio context...                
            m_ioContext.stop();

            // ...and its thread
            if (m_threadContext.joinable())
                m_threadContext.join();

            // Destroy the connection object
            m_connection.release();
        }

        // Is connect to server
        bool isConnected()
        {
            if (m_connection)
                return m_connection->isConnected();
            else
                return false;
        }

        // Send packet to server
        void send(const Packet<T>& packet)
        {
            if (isConnected())
                m_connection->send(packet);
        }

        // Get incoming packets from server
        ThreadSafeQueue<OwnedPacket<T>>& getIncomingPackets()
        {
            return m_incomingPackets;
        }

    protected:
        // ASIO context for async IO
        asio::io_context m_ioContext;

        // Thread to execute work
        std::thread m_threadContext;

        // Connection to server
        std::unique_ptr<TCPConnection<T>> m_connection;

    private:
        // Queue for incoming packets from server
        ThreadSafeQueue<OwnedPacket<T>> m_incomingPackets;
    };
}