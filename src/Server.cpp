#include "../include/Server.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

Server::Server() : _server_fd(-1), _port(0), _password("") {}
Server::~Server() { if (_server_fd != -1) close(_server_fd); }

void Server::init(int port, const std::string& password) {
    _port = port;
    _password = password;
    _lastPingTime = time(NULL);

    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    int opt = 1;
    setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    if (bind(_server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { perror("bind"); exit(EXIT_FAILURE); }
    if (listen(_server_fd, SOMAXCONN) < 0) { perror("listen"); exit(EXIT_FAILURE); }

    struct pollfd server_poll;
    server_poll.fd = _server_fd;
    server_poll.events = POLLIN;
    _fds.push_back(server_poll);

    std::cout << "Server listening on port " << _port << std::endl;
}

void Server::run() {
    while (true) {
        int poll_count = poll(&_fds[0], _fds.size(), 1000); // 1 saniyede bir poll
        if (poll_count == -1) { perror("poll"); continue; }

        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _server_fd)
                    acceptNewClient();
                else
                    receiveFromClient(i);
            }
        }

        time_t now = time(NULL);
        if (now - _lastPingTime >= 60) {
            for (size_t i = 0; i < _clients.size(); ++i)
                sendPing(&_clients[i]);
            _lastPingTime = now;
        }

        checkClientTimeouts();
    }
}

void Server::acceptNewClient() {
    struct sockaddr_in client_address;
    socklen_t addr_len = sizeof(client_address);
    int client_fd = accept(_server_fd, (struct sockaddr*)&client_address, &addr_len);
    if (client_fd == -1) { perror("accept"); return; }

    struct pollfd client_poll;
    client_poll.fd = client_fd;
    client_poll.events = POLLIN;
    _fds.push_back(client_poll);
    _clients.push_back(Client(client_fd));

    std::cout << "New client connected (fd: " << client_fd << ")" << std::endl;
}

void Server::receiveFromClient(size_t index) {
    char buffer[512];
    std::memset(buffer, 0, sizeof(buffer));
    int bytes_read = recv(_fds[index].fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        if (bytes_read == 0) std::cout << "Client disconnected (fd: " << _fds[index].fd << ")" << std::endl;
        else perror("recv");
        removeClient(index);
    } else {
        Client* client = getClientByFd(_fds[index].fd);
        if (!client) return;
        std::string input(buffer);
        size_t pos;
        while ((pos = input.find("\r\n")) != std::string::npos) {
            std::string line = input.substr(0, pos);
            input.erase(0, pos + 2);
            if (!line.empty())
                handleCommand(client, line);
        }
    }
}

void Server::handleCommand(Client* client, const std::string& command) {
    std::cout << "Handling command: [" << command << "]" << std::endl;
    std::string cmd, params;
    size_t space = command.find(' ');
    if (space != std::string::npos) {
        cmd = command.substr(0, space);
        params = command.substr(space + 1);
    } else cmd = command;
    for (size_t i = 0; i < cmd.size(); ++i) cmd[i] = std::toupper(cmd[i]);

    if (cmd == "PASS" && params == _password)
        client->authenticate();
    else if (cmd == "PONG")
        client->updatePingTime();
    else if (cmd == "NICK")
        client->setNickname(params);
    else if (cmd == "USER") {
        client->setUsername(params);
        client->registerUser();
    }
    // JOIN, PRIVMSG, MODE, KICK vs buraya eklenir (isteğe göre).
}

void Server::sendPing(Client* client) {
    std::string ping_msg = "PING :ping\r\n";
    send(client->getFd(), ping_msg.c_str(), ping_msg.length(), 0);
}

void Server::checkClientTimeouts() {
    time_t now = time(NULL);
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (now - _clients[i].getLastPingTime() > 120) { // 2 dakika tolerans
            std::cout << "Client " << _clients[i].getNickname() << " timed out." << std::endl;
            removeClient(i);
            break; // Çünkü _clients değişti!
        }
    }
}

void Server::removeClient(size_t index) {
    int fd = _fds[index].fd;
    close(fd);
    _fds.erase(_fds.begin() + index);
    _clients.erase(_clients.begin() + index);
}

Client* Server::getClientByFd(int fd) {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i].getFd() == fd)
            return &_clients[i];
    }
    return NULL;
}
