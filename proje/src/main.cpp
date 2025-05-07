#include "../include/Server.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>

Server* g_server = NULL;

void signalHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    if (g_server)
    {
        delete g_server;
        g_server = NULL;
    }
    Commands::cleanupChannels(); // BURADA ÇAĞRILIYOR MU KONTROL ET
    exit(0);
}


bool isNumber(const std::string& str)
{
    for (size_t i = 0; i < str.length(); ++i)
        if (!isdigit(str[i]))
            return false;
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    std::string portStr = argv[1];
    std::string password = argv[2];

    if (!isNumber(portStr)) {
        std::cerr << "Error: Port must be a number." << std::endl;
        return 1;
    }

    int port = std::atoi(portStr.c_str());
    if (port < 1024 || port > 65535) {
        std::cerr << "Error: Port must be between 1024 and 65535." << std::endl;
        return 1;
    }

    signal(SIGINT, signalHandler);

    try {
        Commands::setPassword(password);  // ŞİFRE BURADA AYARLANMALI

        Server* server = new Server(port, password);
        g_server = server;
        server->run();
        delete server;
        g_server = NULL;
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

