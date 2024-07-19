#include <logging/Logger.h>
#include <net/TCPServer.h>

int main() 
{
    // Initialize logging
    Logger::init();

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