#include "../include/Server.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

Server::Server() : _server_fd(-1), _port(0), _password("") {}

Server::~Server() {
    if (_server_fd != -1)
        close(_server_fd);
}

void Server::init(int port, const std::string& password) {
    _port = port;
    _password = password;

    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) {
        std::cerr << "Error: socket() failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    if (bind(_server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Error: bind() failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(_server_fd, SOMAXCONN) < 0) {
        std::cerr << "Error: listen() failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct pollfd server_poll;
    server_poll.fd = _server_fd;
    server_poll.events = POLLIN;
    _fds.push_back(server_poll);

    std::cout << "Server listening on port " << _port << std::endl;
}

void Server::run() {
    while (true) {
        int poll_count = poll(&_fds[0], _fds.size(), -1);
        if (poll_count == -1) {
            std::cerr << "Error: poll() failed" << std::endl;
            continue;
        }

        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _server_fd) {
                    acceptNewClient();
                } else {
                    receiveFromClient(i);
                }
            }
        }

        // Her belirli aralıklarla PING gönder
        for (size_t i = 0; i < _clients.size(); ++i) {
            sendPing(&_clients[i]);
        }

        // Zaman aşımına uğramış istemcileri kontrol et
        checkClientTimeouts();

        sleep(60); // 60 saniye bekleyin ve tekrar PING gönderin
    }
}


void Server::sendPing(Client* client) {
    // PING mesajı gönder
    std::string ping_msg = "PING :ping\r\n";
    send(client->getFd(), ping_msg.c_str(), ping_msg.length(), 0);
    std::cout << "Sent PING to client " << client->getNickname() << std::endl();
}

void Server::handlePing(Client* client) {
    // PONG cevabını aldığında son ping zamanını güncelle
    std::string pong_msg = "PONG :pong\r\n";
    send(client->getFd(), pong_msg.c_str(), pong_msg.length(), 0);
    std::cout << "Sent PONG to client " << client->getNickname() << std::endl;
    client->updatePingTime(); // PONG cevabı alındığında ping zamanını güncelle
}

void Server::checkClientTimeouts() {
    time_t current_time = time(NULL);
    for (size_t i = 0; i < _clients.size(); ++i) {
        Client* client = &_clients[i];
        // Eğer son ping zamanından 60 saniye geçmişse, istemciyi zaman aşımına uğrat
        if (current_time - client->getLastPingTime() > 60) { // 60 saniye
            std::cout << "Client " << client->getNickname() << " timed out!" << std::endl;
            removeClient(i); // Zaman aşımına uğrayan istemciyi kaldır
        }
    }
}




void Server::acceptNewClient() {
    struct sockaddr_in client_address;
    socklen_t addr_len = sizeof(client_address);
    int client_fd = accept(_server_fd, (struct sockaddr*)&client_address, &addr_len);
    if (client_fd == -1) {
        std::cerr << "Error: accept() failed" << std::endl;
        return;
    }

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
        if (bytes_read == 0)
            std::cout << "Client disconnected (fd: " << _fds[index].fd << ")" << std::endl;
        else
            std::cerr << "Error: recv() failed" << std::endl;

        removeClient(index);
    } else {
        buffer[bytes_read] = '\0';
        Client* client = getClientByFd(_fds[index].fd);
        if (client) {
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
}

void Server::handleCommand(Client* client, const std::string& command) {
    std::cout << "Handling command: [" << command << "]" << std::endl;
    std::string cmd;
    std::string params;
    size_t pos = command.find(' ');

    if (pos != std::string::npos) {
        cmd = command.substr(0, pos);
        params = command.substr(pos + 1);
    } else {
        cmd = command;
    }

    for (size_t i = 0; i < cmd.length(); ++i)
        cmd[i] = std::toupper(cmd[i]);

    if (cmd == "PASS") {
        if (params == _password) client->authenticate();
    }
    else if (cmd == "PONG") {
        client->updatePingTime();
        std::cout << "PONG received from " << client->getNickname() << std::endl;
    }
    else if (cmd == "NICK") {
        client->setNickname(params);
    } else if (cmd == "USER") {
        client->setUsername(params);
        client->registerUser();
    } else if (cmd == "JOIN") {
        if (params[0] != '#') return;
        Channel* channel = NULL;
        for (size_t i = 0; i < _channels.size(); ++i) {
            if (_channels[i].getName() == params) {
                channel = &_channels[i];
                break;
            }
        }
        if (!channel) {
            _channels.push_back(Channel(params));
            channel = &_channels.back();
        }
        channel->addClient(client);
    } else if (cmd == "PRIVMSG") {
        size_t space = params.find(' ');
        if (space == std::string::npos) return;

        std::string target = params.substr(0, space);
        std::string message = params.substr(space + 1);
        if (!message.empty() && message[0] == ':')
            message = message.substr(1);

        if (target[0] == '#') {
            for (size_t i = 0; i < _channels.size(); ++i) {
                if (_channels[i].getName() == target) {
                    const std::vector<Client*>& clients = _channels[i].getClients();
                    for (size_t j = 0; j < clients.size(); ++j) {
                        if (clients[j]->getFd() != client->getFd()) {
                            std::string full_message = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
                            send(clients[j]->getFd(), full_message.c_str(), full_message.length(), 0);
                        }
                    }
                    return;
                }
            }
        }
    }
}

void Server::removeClient(size_t index) {
    int fd = _fds[index].fd;
    close(fd);
    _fds.erase(_fds.begin() + index);
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->getFd() == fd) {
            _clients.erase(it);
            break;
        }
    }
}

Client* Server::getClientByFd(int fd) {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i].getFd() == fd)
            return &_clients[i];
    }
    return NULL;
}
