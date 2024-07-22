#pragma once

#include "TCPServerInterface.h"

namespace net
{
    using asio::ip::tcp;

    template<typename T>
    class TCPConnection : public std::enable_shared_from_this<TCPConnection<T>>
    {
    public:
        enum class Owner
        {
            Server,
            Client
        };

        TCPConnection(Owner parent, asio::io_context& ioContext, tcp::socket socket, ThreadSafeQueue<OwnedPacket<T>>& incomingPackets)
            : m_owner(parent), m_ioContext(ioContext), m_socket(std::move(socket)), m_incomingPackets(incomingPackets) {

            // Construct validation check
            if (m_owner == Owner::Server)
            {
                // Connection is Server->Client
                // Construct random data to send to client for validation
                m_handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

                // Pre-calculate handshake validation value
                m_handshakeCheck = scramble(m_handshakeOut);
            }
            else
            {
                // Connection is Client->Server, so nothing to define
                m_handshakeIn = 0;
                m_handshakeOut = 0;
            }
        }

        ~TCPConnection()
        {
            disconnect();
        }

        uint32_t getID() const
        {
            return m_id;
        }

        // Connect to client
        void connectToClient(TCPServerInterface<T>* server, uint32_t uid = 0)
        {
            if (m_owner == Owner::Server)
            {
                if (m_socket.is_open())
                {
                    m_id = uid;
                    readHeader();

                    // A client has attempted to connect to the server
                    // But we want to validate the client first
                    // So first send out handshake data
                    //writeValidation();

                    // Next, wait asynchronously for validation data to be returned from the client
                    //readValidation(server);
                }
            }
        }

        // Connect to server
        void connectToServer(const tcp::resolver::results_type& endpoints)
        {
            if (m_owner == Owner::Client)
            {
                // Request asio attempts to connect to an endpoint
                asio::async_connect(m_socket, endpoints,
                    [this](std::error_code ec, tcp::endpoint endpoints)
                    {
                        if (!ec)
                        // On connection, server will send packet to validate client
                        //readValidation();
                            readHeader();
                    });
            }
        }

        // Disconnect from the client/server
        void disconnect()
        {
            // If the socket is connected, close it
            if (isConnected())
                asio::post(m_ioContext, [this]() { m_socket.close(); });
        }

        // Check if the connection is still up
        bool isConnected()
        {
            return m_socket.is_open();
        }

        // Listen for incoming packets
        void listen()
        {}

        // Send a written packet to the client/server
        void send(const Packet<T>& packet)
        {
            asio::post(m_ioContext,
                [this, packet]()
                {
                    // If the queue has a message in it, then assume that it is in the process of asynchronously being written.
                    bool writingMessage = !m_outgoingPackets.empty();

                    // Either way add the message to the queue to be output.
                    m_outgoingPackets.push_back(packet);

                    // If no messages were available to be written, then start the process of writing the
                    // message at the front of the queue.
                    if (!writingMessage)
                        writeHeader();
                });
        }

        // Read an incoming packet header
        void readHeader()
        {
            // If this function is called, we are expecting to receive a packet header
            // We know the headers are a fixed size so allocate to that size
            asio::async_read(m_socket, asio::buffer(&m_tempIncomingPacket.header, sizeof(PacketHeader<T>)),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        // If there's no error
                        // We've received a complete packet header, check if it has a body after it
                        if (m_tempIncomingPacket.header.size > 0)
                        {
                            // It has a body so allocate space in the packets body vector and read the body
                            m_tempIncomingPacket.body.resize(m_tempIncomingPacket.header.size);
                            readBody();
                        }
                        else
                        {
                            // There's no body with the packet, so add it to the incomingPacket queue
                            addToIncomingPacketQueue();
                        }
                    }
                    else
                    {
                        // Reading from the client went wrong. Close the socket and let the system tidy it up later.
                        spdlog::warn("[{}] Read Header Fail.", m_id);
                        spdlog::warn(ec.message());
                        m_socket.close();
                    }
                });
        }

        // Read an incoming packet body
        void readBody()
        {
            // If this function is called, we are expecting to receive a packet body so wait for the data to arrive
            asio::async_read(m_socket, asio::buffer(m_tempIncomingPacket.body.data(), m_tempIncomingPacket.body.size()),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        // No error, the packet is now complete, so add the whole packet to incoming queue
                        addToIncomingPacketQueue();
                    }
                    else
                    {
                        // Reading from the client went wrong. Close the socket and let the system tidy it up later.
                        spdlog::warn("[{}] Read Body Fail.", m_id);
                        spdlog::warn(ec.message());
                        m_socket.close();
                    }
                });
        }

        // Adds incoming packets to the packet queue for processing
        void addToIncomingPacketQueue()
        {
            // Convert to an OwnedPacket and add it to the queue
            if (m_owner == Owner::Server)
                m_incomingPackets.push_back({ this->shared_from_this(), m_tempIncomingPacket });
            else
                m_incomingPackets.push_back({ nullptr, m_tempIncomingPacket });

            // Ready asio to read next packet header
            readHeader();
        }

        // Write a packet header
        void writeHeader()
        {
            // If this function is called, then there is at least one packet in the outgoing packet queue
            // Allocate a buffer to hold the packet and construct the header
            asio::async_write(m_socket, asio::buffer(&m_outgoingPackets.front().header, sizeof(PacketHeader<T>)),
                [this](std::error_code ec, std::size_t length)
                {
                    // asio has send the packet
                    // Check for error
                    if (!ec)
                    {
                        // No error, check if sent packet header has a body
                        if (m_outgoingPackets.front().body.size() > 0)
                        {
                            // If it does, write the body data
                            writeBody();
                        }
                        else
                        {
                            // If there's no body, remove it from the packet queue
                            m_outgoingPackets.pop_front();

                            // If the queue is not empty, send the next header
                            if (!m_outgoingPackets.empty())
                            {
                                writeHeader();
                            }
                        }
                    }
                    else
                    {
                        // There's an error, so output to console and close the socket
                        spdlog::warn("[{}] Write Header Fail.", m_id);
                        spdlog::warn(ec.message());
                        m_socket.close();
                    }
                });
        }

        // Write a packet body
        void writeBody()
        {
            // If this function is called, a header was just sent and had a body, so send its associated body
            asio::async_write(m_socket, asio::buffer(m_outgoingPackets.front().body.data(), m_outgoingPackets.front().body.size()),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        // There's no error sending the body
                        // Remove it from the queue
                        m_outgoingPackets.pop_front();

                        // If there's more packets in the queue, send them
                        if (!m_outgoingPackets.empty())
                            writeHeader();
                    }
                    else
                    {
                        // There's an error, so output to console and close the socket
                        spdlog::warn("[{}] Write Body Fail.", m_id);
                        spdlog::warn(ec.message());
                        m_socket.close();
                    }
                });
        }

        uint64_t scramble(uint64_t input)
        {
            uint64_t out = input ^ 0xDEADBEEFC0DECAFE;
            out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
            return out ^ 0xC0DEFACE12345678;
        }

        // Validates incoming connection
        void readValidation(TCPServerInterface<T>* server = nullptr)
        {
            asio::async_read(m_socket, asio::buffer(&m_handshakeIn, sizeof(uint64_t)),
                [this, server](std::error_code ec, std::size_t length) {
                    if (!ec)
                    {
                        if (m_owner == Owner::Server)
                        {
                            // Connection is a server, check response from client
                            if (m_handshakeIn == m_handshakeCheck) {
                                // Client has successfully validated, allow it to connect
                                spdlog::info("Client validated");
                                server->onClientValidated(this->shared_from_this());

                                // Wait for data
                                readHeader();
                            }
                            else
                            {
                                // Client gave invalid data, close connection
                                spdlog::warn("Client failed validation");
                                m_socket.close();
                            }
                        }
                        else
                        {
                            // Connection is a client, solve the puzzle
                            m_handshakeOut = scramble(m_handshakeIn);

                            // Write results
                            writeValidation();
                        }
                    }
                    else
                    {
                        // Some error occurred
                        spdlog::warn("Client disconnected (readValidation)");
                        spdlog::warn(ec.message());
                        m_socket.close();
                    }
                });
        }

        // Validates outgoing connection
        void writeValidation()
        {
            asio::async_write(m_socket, asio::buffer(&m_handshakeOut, sizeof(uint64_t)),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec) {
                        // Theres no error sending validation data, wait for client response
                        if (m_owner == Owner::Client)
                            readHeader();
                        else
                            // If there's an error sending validation data, close the socket
                            m_socket.close();
                    }
                });
        }

    private:
        uint32_t m_id = 0;
        Owner m_owner = Owner::Server;

        // Unique socket to remote connection
        tcp::socket m_socket;

        // This context is shared with the asio instance
        asio::io_context& m_ioContext;

        // Incoming messages are async so we store the partially assembled message here
        Packet<T> m_tempIncomingPacket;

        // Holds messages coming from the remote connection(s)
        ThreadSafeQueue<OwnedPacket<T>>& m_incomingPackets;

        // Holds messages to be sent to the remote connection
        ThreadSafeQueue<Packet<T>> m_outgoingPackets;

        // Handshake Validation            
        uint64_t m_handshakeOut = 0;
        uint64_t m_handshakeIn = 0;
        uint64_t m_handshakeCheck = 0;

        bool m_validHandshake = false;
        bool m_connectionEstablished = false;
    };
}