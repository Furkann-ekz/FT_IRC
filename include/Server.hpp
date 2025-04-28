#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"

class Server {
private:
    int _server_fd;
    int _port;
    std::string _password;
    std::vector<struct pollfd> _fds;
    std::vector<Client> _clients;
    std::vector<Channel> _channels;
    time_t _lastPingTime;

public:
    Server();
    ~Server();

    void init(int port, const std::string& password);
    void run();

private:
    void acceptNewClient();
    void receiveFromClient(size_t index);
    void handleCommand(Client* client, const std::string& command);
    void removeClient(size_t index);
    Client* getClientByFd(int fd);

    void sendPing(Client* client);
    void checkClientTimeouts();
};

#endif
