#include <logging/Logger.h>
#include <net/TCPServer.h>
#include <iostream>

int main() 
{
    // Initialize logging
    //Logger::init();

    try
    {
        //SERVER_INFO("CONNECTING");
        net::TCPServer server(60000);
        server.start();

        while (1)
        {
            server.update(-1, true);
        }
    }
    catch (std::exception& e)
    {
        //SERVER_ERROR("{0}", e.what());
         std::cerr << e.what() << std::endl;
    }
}