// Server.hpp
#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <cstdio>
#include <map>
#include <string>
#include <poll.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include "Client.hpp"
#include "Commands.hpp"
#include "Channel.hpp"

extern std::map<std::string, Channel*> g_channels;

class Server
{
    private:
        int _listenFd;
        std::string _password;
        std::vector<struct pollfd> _pollfds;
        std::map<int, Client*> _clients;

    public:
        Server(int port, const std::string& password);
        ~Server();

        void run();
        void acceptNewClient();
        void handleClientData(int fd);
        void removeClient(int fd);
        std::map<int, Client*>& getClients();
};

#endif
