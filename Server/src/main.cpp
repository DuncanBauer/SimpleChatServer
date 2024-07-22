#include <iostream>

#include <mongocxx/instance.hpp>

#include <logging/Logger.h>
#include <net/TCPServer.h>
#include "MongoDbHandler.h"

int main() 
{
    // Initialize logging
    Logger::init();

    // Initialize MongoDB instance
    mongocxx::instance instance;
    MongoDbHandler handler;

    //handler.changeDisplayName("username", "omg wut");
    //handler.changeDisplayName("username1", "newDisplayName1");
    //handler.deleteUser("duncan");
    //handler.registerUser("duncan", "password", "erotichousecat");
    handler.login("duncan", "password");

    //handler.deleteUser("stefan");
    //handler.registerUser("stefan", "password", "erotichousecat");
    handler.login("stefan", "password1");

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