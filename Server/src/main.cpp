#include <iostream>

#include <mongocxx/instance.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/exception/error_code.hpp>

#include <logging/Logger.h>
#include <net/TCPServer.h>

int main() 
{
    // Initialize logging
    Logger::init();

    // Initialize MongoDB instance
    mongocxx::instance instance{};
    std::cout << '\n';

    try
    {
        net::TCPServer server(60000);
        server.start();

        server.getDbHandler().registerUser("duncan", "password");
        server.getDbHandler().createServer("duncan", "duncan's server");
        //server.getDbHandler().deleteServer("duncan's server");
        //server.getDbHandler().deleteUser("duncan");

        while (1)
        {
            server.update(-1, true);
        }

    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
    }

    std::cin.get();
}