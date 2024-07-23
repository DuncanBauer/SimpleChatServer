#pragma once

#include <asio.hpp>
#include "spdlog/spdlog.h"

#include "ThreadSafeQueue.h"
#include "TCPNet.h"

namespace net
{
    using asio::ip::tcp;

    template<typename T>
    class TCPServerInterface
    {
    public:
        TCPServerInterface(uint16_t port) : m_acceptor(m_ioContext, tcp::endpoint(tcp::v4(), port))
        {}

        virtual ~TCPServerInterface()
        {
            stop();
        }

        // Starts the server
        bool start()
        {
            try
            {
                // Issue a task to the asio context - This is important
                // as it will prime the context with "work", and stop it
                // from exiting immediately. Since this is a server, we 
                // want it primed ready to handle clients trying to
                // connect.
                listenForClientConnection();

                // Launch the asio context in its own thread
                m_threadContext = std::thread([this]() { m_ioContext.run(); });
            }
            catch (std::exception& e)
            {
                // Something prohibited the server from listening
                spdlog::error("[SERVER] Exception: {}", e.what());
                return false;
            }

            spdlog::info("[SERVER] Started!");
            return true;
        }

        // Stops the server
        void stop()
        {
            // Stop asio context
            m_ioContext.stop();

            // End asio thread
            if (m_threadContext.joinable())
                m_threadContext.join();

            // Output to console
            spdlog::info("[SERVER] Stopped!");
        }

        // Instructs asio to wait for a client so it doesn't simply stop
        void listenForClientConnection()
        {
            // Have asio context wait for client connections
            m_acceptor.async_accept(
                [this](std::error_code ec, tcp::socket socket)
                {
                    if (!ec)
                    {
                        // No errors receiving incoming connection
                        spdlog::info("[SERVER] New Connection: {}:{}", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());

                        // Create new connection to handle client
                        std::shared_ptr <TCPConnection<T>> conn = std::make_shared<TCPConnection<T>>(
                            TCPConnection<T>::Owner::Server,
                            m_ioContext,
                            std::move(socket),
                            m_incomingPackets);

                        // Give the user a chance to deny connection
                        if (onClientConnect(conn))
                        {
                            // Add connection to connections container
                            m_connections.push_back(std::move(conn));

                            // Inform connection to wait for incoming packets
                            m_connections.back()->connectToClient(this, m_idCounter++);

                            spdlog::info("[{}] Connection Approved", m_connections.back()->getID());
                        }
                        else
                        {
                            // Connection will go out of scope without tasks and will get destroyed by the smart pointer    
                            spdlog::info("[-----] Connection Denied");
                        }
                    }
                    else
                    {
                        // Error has occurred while accepting client
                        spdlog::info("[SERVER] New Connection Error: {}", ec.message().data());
                    }

            // Wait for more connections
            listenForClientConnection();
                });
        }

        // Send a message to a unique client
        void messageClient(std::shared_ptr<TCPConnection<T>> client, const Packet<T>& packet)
        {
            // Check if client is legit
            if (client && client->isConnected)
            {
                // Send the packet
                client->send(packet);
            }
            else
            {
                // Remove the client since we can't communicate with it
                // Handle any disconnect requirements of the server
                onClientDisconnect(client);

                // Reset the shared pointer to nullptr
                client.reset();

                // Remove the connection from the connections container
                m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end());
            }
        }

        // Send a message to all clients
        void messageAllClients(const Packet<T>& packet, std::shared_ptr<TCPConnection<T>> pIgnoreClient = nullptr)
        {
            // Flag for an invalid client
            bool bInvalidClientExists = false;

            // Iterate through all clients in container
            for (auto& client : m_connections)
            {
                // Check client is connected...
                if (client && client->isConnected())
                {
                    // ..it is!
                    if (client != pIgnoreClient)
                        client->send(packet);
                }
                else
                {
                    // The client couldnt be contacted, so assume it has disconnected.
                    onClientDisconnect(client);

                    // Reset the shared pointer to nullptr
                    client.reset();

                    // Set this flag to then remove dead clients from container
                    bInvalidClientExists = true;
                }
            }

            // Remove dead clients, all in one go - this way, we dont invalidate the
            // container as we iterated through it.
            if (bInvalidClientExists)
                m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), nullptr), m_connections.end());
        }

        // Forces to the server to process incoming messages
        void update(size_t maxPackets = -1, bool wait = false)
        {
            if (wait)
            {
                m_incomingPackets.wait();

                // Process messages up to maxPackets
                size_t packetCount = 0;
                while (packetCount < maxPackets && !m_incomingPackets.empty())
                {
                    // Grab first packet
                    auto packet = m_incomingPackets.pop_front();

                    // Handle packet
                    onMessage(packet.remote, packet.packet);

                    packetCount++;
                }
            }
        }

    protected:
        // These should be overridden in a derived class
        // Called when a client connects
        virtual bool onClientConnect(std::shared_ptr<TCPConnection<PacketType>> client)
        {
            return false;
        }

        // Called when a client disconnects
        virtual void onClientDisconnect(std::shared_ptr<TCPConnection<PacketType>> client)
        {}

        // Called when a message is received
        virtual void onMessage(std::shared_ptr<TCPConnection<PacketType>> client, Packet<PacketType>& packet)
        {}

    public:
        // Called when a client is validated
        virtual void onClientValidated(std::shared_ptr<TCPConnection<PacketType>> client)
        {}

    private:
        // Order of declaration matters regardless of whether i want it to be
        asio::io_context m_ioContext;

        std::thread m_threadContext;

        // Handles incoming connection attemps
        tcp::acceptor m_acceptor;

        // ThreadSafeQueue for incoming packets
        ThreadSafeQueue<OwnedPacket<T>> m_incomingPackets;

        // Queue for connections
        std::deque<std::shared_ptr<TCPConnection<T>>> m_connections;

        // Clients will be identified by id
        uint32_t m_idCounter = 10000;
    };
}