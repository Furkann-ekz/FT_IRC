#include "../include/Server.hpp"


Server::Server(int port, const std::string& password) : _password(password) {
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(_listenFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(_listenFd, SOMAXCONN) < 0) {
        perror("listen");
        exit(1);
    }

    // poll için ilk fd listen soketidir
    struct pollfd listenPollFd;
    listenPollFd.fd = _listenFd;
    listenPollFd.events = POLLIN;
    _pollfds.push_back(listenPollFd);

    std::cout << "IRC Server listening on port " << port << std::endl;
}

Server::~Server()
{
    if (_listenFd >= 0)
	{
        close(_listenFd);
        _listenFd = -1;
    }
    for (size_t i = 0; i < _pollfds.size(); ++i)
	{
        if (_pollfds[i].fd >= 0 && _pollfds[i].fd != _listenFd)
		{
			shutdown(_pollfds[i].fd, SHUT_RDWR);
            close(_pollfds[i].fd);
		}
	}
			_pollfds.clear();
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        delete it->second;
    _clients.clear();
}

void Server::run()
{
    while (true)
	{
        int ret = poll(&_pollfds[0], _pollfds.size(), -1);
        if (ret < 0)
		{
            perror("poll");
            break;
        }

        for (size_t i = 0; i < _pollfds.size(); ++i)
		{
            if (_pollfds[i].revents & POLLIN)
			{
                if (_pollfds[i].fd == _listenFd)
                    acceptNewClient();
				else
                    handleClientData(_pollfds[i].fd);
            }
        }
    }
}

void Server::acceptNewClient()
{
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientFd = accept(_listenFd, (sockaddr*)&clientAddr, &clientLen);
    if (clientFd < 0)
    {
        perror("accept");
        return;
    }

    // poll kontrolüyle blocking/non-blocking zaten yapılmış olacak, ayrıca fcntl ile ayar yapmıyoruz (ubuntu kuralı)
    int flags = fcntl(clientFd, F_GETFL, 0);
    if (flags < 0 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("fcntl");
        close(clientFd);
        return;
    }
    Client* newClient = new Client(clientFd);
    _clients[clientFd] = newClient;

    struct pollfd pfd;
	std::memset(&pfd, 0, sizeof(pfd));
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	_pollfds.push_back(pfd);

    std::cout << "New client connected: " << clientFd << std::endl;
}

void Server::handleClientData(int fd)
{
    char buffer[512];
    std::memset(buffer, 0, sizeof(buffer));

    ssize_t bytesReceived = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0)
    {
        std::cout << "Client " << fd << " disconnected." << std::endl;
        removeClient(fd);
        return;
    }

    buffer[bytesReceived] = '\0';

    Client* client = _clients[fd];
    client->getRecvBuffer().append(buffer);

    size_t pos;
    while ((pos = client->getRecvBuffer().find('\n')) != std::string::npos)
    {
        std::string line = client->getRecvBuffer().substr(0, pos);
        client->getRecvBuffer().erase(0, pos + 1);

        if (!line.empty() && line[line.length() - 1] == '\r')
            line.erase(line.length() - 1);

        if (line.empty())
            continue;

        std::cout << "Received from " << fd << ": [" << line << "]" << std::endl; // ✅ sadece işlenen komut

        Commands::execute(client, line);

        if (_clients.find(fd) == _clients.end())
            return;

        client = _clients[fd];
    }
}


void Server::removeClient(int fd)
{
    if (_clients.find(fd) == _clients.end())
        return;

    close(fd); // kapanmışsa tekrar kapama dert yaratmaz

    for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _pollfds.erase(it);
            break;
        }
    }

    delete _clients[fd];
    _clients.erase(fd);
}

std::map<int, Client*>& Server::getClients()
{
    return _clients;
}