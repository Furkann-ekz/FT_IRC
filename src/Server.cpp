#include "../include/Server.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>

Server::Server() : _server_fd(-1), _port(0), _password("") {}

void Server::clearResources() {
    for (size_t i = 0; i < _clients.size(); ++i)
        delete _clients[i];
    _clients.clear();
    std::vector<Client*>().swap(_clients);
    _channels.clear();
    std::vector<Channel>().swap(_channels);
    _fds.clear();
    std::vector<struct pollfd>().swap(_fds);
}

void Server::cleanup() {
    clearResources();
}

Server::~Server() {
    if (_server_fd != -1)
        close(_server_fd);
    clearResources();
}


void Server::init(int port, const std::string& password) {
    _port = port;
    _password = password;

    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    int opt = 1;
    setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    if (bind(_server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }
    if (listen(_server_fd, SOMAXCONN) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    struct pollfd server_poll;
    server_poll.fd = _server_fd;
    server_poll.events = POLLIN;
    server_poll.revents = 0; // Açıkça sıfırla
    _fds.push_back(server_poll);

    std::cout << "Server listening on port " << _port << std::endl;
}

void Server::acceptNewClient() {
    struct sockaddr_in client_address;
    std::memset(&client_address, 0, sizeof(client_address)); // Güvenli başlatma
    socklen_t addr_len = sizeof(client_address);
    int client_fd = accept(_server_fd, (struct sockaddr*)&client_address, &addr_len);
    if (client_fd == -1) { perror("accept"); return; }

    struct pollfd client_poll;
    client_poll.fd = client_fd;
    client_poll.events = POLLIN;
    client_poll.revents = 0; // Açıkça sıfırla
    _fds.push_back(client_poll);

    Client* new_client = new Client(client_fd);
    _clients.push_back(new_client);

    std::cout << "New client connected (fd: " << client_fd << ")" << std::endl;
}

void Server::run() {
    while (true) {
        if (_fds.empty()) continue;

        int poll_count = poll(&_fds[0], _fds.size(), 1000);
        if (poll_count == -1) { perror("poll"); continue; }

        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _server_fd)
                    acceptNewClient();
                else
                    receiveFromClient(i);
            }
        }
    }
}



void Server::receiveFromClient(size_t index) {
	int client_fd = _fds[index].fd;

	Client* client = getClientByFd(client_fd);
	if (!client) {
		std::cerr << "Warning: No client found for fd " << client_fd << ". Removing poll entry." << std::endl;
		removeClientByFd(client_fd);  // Yeni fonksiyonu burada da çağır
		return;
	}

	char buffer[512];
	std::memset(buffer, 0, sizeof(buffer));
	int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_read <= 0) {
		// 🔴 HATALI İSTEMCİ — burada bağlantıyı sonlandırıyoruz
		removeClientByFd(client_fd);
		return;
	}

	std::string input(buffer);
	client->appendToBuffer(input);

	std::vector<std::string> commands = MessageParser::extractCommands(client->getBuffer());
	for (size_t i = 0; i < commands.size(); ++i) {
		if (!commands[i].empty())
			handleCommand(client, commands[i]);
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
	else if (cmd == "NICK") {
		client->setNickname(params);
	}
	else if (cmd == "USER") {
		client->setUsername(params);
	}
	else if (cmd == "PART") {
		std::string channelName = params;
		if (channelName.empty()) {
			sendToClient(client, "ERROR :No channel name given");
			return;
		}

		Channel* channel = findChannelByName(channelName);
		if (!channel) {
			sendToClient(client, "ERROR :No such channel");
			return;
		}

		if (!channel->hasClient(client)) {
			sendToClient(client, "ERROR :You're not in that channel");
			return;
		}

		std::string msg = ":" + client->getNickname() + " PART " + channelName;
		const std::vector<Client*>& members = channel->getClients();
		for (size_t i = 0; i < members.size(); ++i) {
			sendToClient(members[i], msg);
		}

		channel->removeClient(client);
	}

	else if (cmd == "QUIT") {
		std::string quitMsg = params.empty() ? "Client Quit" : params;
		std::string msg = ":" + client->getNickname() + " QUIT :" + quitMsg;

		// Tüm kanallardaki üyelere bildir
		for (size_t i = 0; i < _channels.size(); ++i) {
			if (_channels[i].hasClient(client)) {
				const std::vector<Client*>& members = _channels[i].getClients();
				for (size_t j = 0; j < members.size(); ++j) {
					if (members[j] != client) {
						sendToClient(members[j], msg);
					}
				}
				_channels[i].removeClient(client);
			}
		}

		// Bağlantıyı temizle
		removeClientByFd(client->getFd());
	}

	else if (cmd == "MODE") {
		size_t space = params.find(' ');
		if (space == std::string::npos) {
			sendToClient(client, "ERROR :MODE format invalid");
			return;
		}

		std::string channelName = params.substr(0, space);
		std::string modeParams = params.substr(space + 1);

		Channel* channel = findChannelByName(channelName);
		if (!channel) {
			sendToClient(client, "ERROR :No such channel");
			return;
		}

		if (!channel->isOperator(client)) {
			sendToClient(client, "ERROR :You're not channel operator");
			return;
		}

		std::stringstream ss(modeParams);
		std::string token;
		char action = 0;

		while (ss >> token) {
			if (token[0] == '+' || token[0] == '-') {
				action = token[0];
				for (size_t i = 1; i < token.size(); ++i) {
					char flag = token[i];

					switch (flag) {
						case 'i':
							channel->setInviteOnly(action == '+');
							break;
						case 't':
							channel->setTopicRestricted(action == '+');
							break;
						case 'k': {
							std::string key;
							if (action == '+' && ss >> key)
								channel->setKey(key);
							else if (action == '-')
								channel->setKey("");
							break;
						}
						case 'l': {
							std::string limit;
							if (action == '+' && ss >> limit)
								channel->setUserLimit(std::atoi(limit.c_str()));
							else if (action == '-')
								channel->setUserLimit(-1);
							break;
						}
						case 'o': {
							std::string nick;
							if (ss >> nick) {
								Client* target = findClientByNick(nick);
								if (!target || !channel->hasClient(target)) {
									sendToClient(client, "ERROR :Target not in channel");
									return;
								}

								if (action == '+') {
									channel->addOperator(target);
									sendToClient(target, "You are now a channel operator in " + channelName);
								} else {
									channel->removeOperator(target);
									sendToClient(target, "Your operator status has been removed in " + channelName);
								}
							} else {
								sendToClient(client, "ERROR :Missing nickname for +o/-o");
								return;
							}
							break;
						}
						default:
							sendToClient(client, "ERROR :Unknown mode flag");
							return;
					}
				}
			}
		}

		sendToClient(client, "MODE set successfully");
	}


	else if (cmd == "TOPIC") {
		size_t space = params.find(' ');
		std::string channelName = params;
		std::string topicText = "";

		if (space != std::string::npos) {
			channelName = params.substr(0, space);
			if (params.size() > space + 1 && params[space + 1] == ':')
				topicText = params.substr(space + 2); // skip " :"
		}

		Channel* channel = findChannelByName(channelName);
		if (!channel) {
			sendToClient(client, "ERROR :No such channel");
			return;
		}

		if (!channel->hasClient(client)) {
			sendToClient(client, "ERROR :You're not in that channel");
			return;
		}

		if (topicText.empty()) {
			const std::string& currentTopic = channel->getTopic();
			if (currentTopic.empty())
				sendToClient(client, "No topic is set");
			else
				sendToClient(client, "Current topic: " + currentTopic);
		} else {
			// Not: Şimdilik herkes değiştirebiliyor; MODE +t eklenince bu kontrol değişir
			channel->setTopic(topicText);

			std::string msg = ":" + client->getNickname() + " TOPIC " + channelName + " :" + topicText;
			const std::vector<Client*>& members = channel->getClients();
			for (size_t i = 0; i < members.size(); ++i) {
				sendToClient(members[i], msg);
			}
		}
	}

	else if (cmd == "INVITE") {
		size_t space = params.find(' ');
		if (space == std::string::npos) {
			sendToClient(client, "ERROR :Invalid INVITE format");
			return;
		}

		std::string targetNick = params.substr(0, space);
		std::string channelName = params.substr(space + 1);

		Channel* channel = findChannelByName(channelName);
		if (!channel) {
			sendToClient(client, "ERROR :No such channel");
			return;
		}

		if (!channel->hasClient(client)) {
			sendToClient(client, "ERROR :You're not in that channel");
			return;
		}

		Client* target = findClientByNick(targetNick);
		if (!target) {
			sendToClient(client, "ERROR :No such nick");
			return;
		}

		channel->inviteClient(target);
		std::string msg = ":" + client->getNickname() + " INVITE " + targetNick + " " + channelName;
		sendToClient(target, msg);
	}

	else if (cmd == "KICK") {
		size_t space = params.find(' ');
		if (space == std::string::npos) {
			sendToClient(client, "ERROR :Invalid KICK format");
			return;
		}

		std::string channelName = params.substr(0, space);
		std::string targetNick = params.substr(space + 1);

		Channel* channel = findChannelByName(channelName);
		if (!channel) {
			sendToClient(client, "ERROR :No such channel");
			return;
		}

		if (!channel->isOperator(client)) {
			sendToClient(client, "ERROR :You're not channel operator");
			return;
		}

		Client* target = findClientByNick(targetNick);
		if (!target || !channel->hasClient(target)) {
			sendToClient(client, "ERROR :User not in channel");
			return;
		}

		channel->removeClient(target);

		std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName + " " + targetNick;
		const std::vector<Client*>& members = channel->getClients();
		for (size_t i = 0; i < members.size(); ++i) {
			sendToClient(members[i], kickMsg);
		}

		// Kicked kullanıcıya da bildir
		sendToClient(target, kickMsg);
	}

	else if (cmd == "JOIN") {
		std::stringstream ss(params);
		std::string channelName;
		std::string keyGiven;
		ss >> channelName >> keyGiven;

		if (channelName.empty() || channelName[0] != '#') {
			sendToClient(client, "ERROR :Invalid channel name");
			return;
		}

		Channel* channel = findChannelByName(channelName);
		if (!channel) {
			_channels.push_back(Channel(channelName));
			channel = &_channels.back();
		}
		if (channel->getUserLimit() != -1 &&
			channel->getClientCount() >= (size_t)channel->getUserLimit()) {
			sendToClient(client, "ERROR :Channel is full");
			return;
		}

		// 🔐 Şifre kontrolü (yalnızca kanal şifreliyse)
		if (!channel->getKey().empty()) {
			if (keyGiven.empty() || keyGiven != channel->getKey()) {
				sendToClient(client, "ERROR :Cannot join channel (key required)");
				return;
			}
		}

		// 🔒 Invite-only kontrolü
		if (channel->isInviteOnly() && !channel->isInvited(client)) {
			sendToClient(client, "ERROR :You're not invited");
			return;
		}

		// 🔢 Kullanıcı limiti kontrolü
		if (channel->getUserLimit() != -1 &&
			channel->getClientCount() >= (size_t)channel->getUserLimit()) {
			sendToClient(client, "ERROR :Channel is full");
			return;
		}

		if (!channel->hasClient(client)) {
			channel->addClient(client);
			std::cout << client->getNickname() << " joined channel " << channelName << std::endl;
		}

		std::string join_msg = ":" + client->getNickname() + " JOIN " + channelName;
		const std::vector<Client*>& members = channel->getClients();
		for (size_t i = 0; i < members.size(); ++i)
			sendToClient(members[i], join_msg);
	}


	else if (cmd == "PRIVMSG") {
		size_t colon = params.find(" :");
		if (colon == std::string::npos) {
			sendToClient(client, "ERROR :No text to send");
			return;
		}

		std::string target = params.substr(0, colon);
		std::string message = params.substr(colon + 2);

		if (target.empty() || message.empty()) {
			sendToClient(client, "ERROR :Invalid PRIVMSG format");
			return;
		}

		if (target[0] == '#') {
			Channel* channel = findChannelByName(target);
			if (!channel || !channel->hasClient(client)) {
				sendToClient(client, "ERROR :Cannot send to channel");
				return;
			}

			const std::vector<Client*>& members = channel->getClients();
			for (size_t i = 0; i < members.size(); ++i) {
				if (members[i] != client) {
					std::string msg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message;
					sendToClient(members[i], msg);
				}
			}
		} else {
			Client* recipient = findClientByNick(target);
			if (!recipient) {
				sendToClient(client, "ERROR :No such nick");
				return;
			}

			std::string msg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message;
			sendToClient(recipient, msg);
		}
	}
	if (client->isAuthenticated() &&
		client->getNickname() != "" &&
		client->getUsername() != "" &&
		!client->isRegistered()) {

		client->registerUser();

		sendToClient(client, ":irc.localhost 001 " + client->getNickname() +
			" :Welcome to the IRC network, " + client->getNickname());
	}

}

Channel* Server::findChannelByName(const std::string& name) {
	for (size_t i = 0; i < _channels.size(); ++i) {
		if (_channels[i].getName() == name)
			return &_channels[i];
	}
	return NULL;
}

Client* Server::findClientByNick(const std::string& nick) {
	for (size_t i = 0; i < _clients.size(); ++i) {
		if (_clients[i]->getNickname() == nick)
			return _clients[i];
	}
	return NULL;
}

void Server::sendToClient(Client* client, const std::string& msg) {
	if (client)
		send(client->getFd(), (msg + "\r\n").c_str(), msg.length() + 2, 0);
}

void Server::removeClientByFd(int fd) {
	// pollfd listesinden sil
	for (size_t i = 0; i < _fds.size(); ++i) {
		if (_fds[i].fd == fd) {
			close(fd);
			_fds.erase(_fds.begin() + i);
			break;
		}
	}

	// Client* listesinden sil
	for (size_t i = 0; i < _clients.size(); ++i) {
		if (_clients[i]->getFd() == fd) {
			Client* client = _clients[i];

			// Tüm kanallardan çıkar
			for (size_t j = 0; j < _channels.size(); ++j) {
				if (_channels[j].hasClient(client))
					_channels[j].removeClient(client);
			}

			delete client;
			_clients.erase(_clients.begin() + i);
			break;
		}
	}

	std::cout << "Client disconnected (fd: " << fd << ")" << std::endl;
}


Client* Server::getClientByFd(int fd) {
	for (size_t i = 0; i < _clients.size(); ++i) {
		if (_clients[i] && _clients[i]->getFd() == fd)
			return _clients[i];
	}
	return NULL;
}

