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
        //handler.deleteUser("duncan");
        //handler.deleteUser("duncan");
        //handler.registerUser("duncan", "password");
        //handler.registerUser("duncan", "password");
        //handler.registerUser("duncan", "password");
        //handler.registerUser("duncan", "password");
        //handler.login("duncan", "password");
        //handler.logout("duncan");
        //handler.createServer("duncan", "duncan's server");
        //handler.deleteServer("duncan's server");
        //handler.deleteUser("duncan");

        //handler.registerUser("stefan", "password");
        //handler.login("stefan", "password1");
        //handler.deleteUser("stefan");

        net::TCPServer server(60000);
        server.start();

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