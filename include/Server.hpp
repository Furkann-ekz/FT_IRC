#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"
#include "cstdio"
#include "MessageParser.hpp"

class Server {
private:
    int _server_fd;
    int _port;
    std::string _password;
    std::vector<struct pollfd> _fds;
    std::vector<Client*> _clients;
    std::vector<Channel> _channels;
    

public:
    Server();
    ~Server();

    void init(int port, const std::string& password);
    void run();
    void cleanup();
    void clearResources();

private:
    void acceptNewClient();
    void receiveFromClient(size_t index);
    void handleCommand(Client* client, const std::string& command);
    void removeClient(size_t index);
    Client* getClientByFd(int fd);

    Channel* findChannelByName(const std::string& name);
    Client* findClientByNick(const std::string& nick);
    void sendToClient(Client* client, const std::string& msg);
    void removeClientByFd(int fd);

};

#endif
