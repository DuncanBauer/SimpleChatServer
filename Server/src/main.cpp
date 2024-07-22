#include <iostream>

#include <mongocxx/instance.hpp>

#include <logging/Logger.h>
#include <net/TCPServer.h>
#include "Global.h"

int main() 
{
    // Initialize logging
    Logger::init();

    // Initialize MongoDB instance
    mongocxx::instance instance;

    //handler.registerUser("duncan", "password", "erotichousecat");
    //handler.login("duncan", "password");
    //handler.changeDisplayName("duncan", "moistoyster");
    //handler.logout("duncan");
    //handler.createServer("duncan", "duncan's server");
    //handler.deleteServer("duncan's server");
    //handler.deleteUser("duncan");

    //handler.registerUser("stefan", "password", "erotichousecat");
    //handler.login("stefan", "password1");
    //handler.deleteUser("stefan");

    try
    {
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
}