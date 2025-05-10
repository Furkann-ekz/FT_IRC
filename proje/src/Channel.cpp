#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include <algorithm>
#include <sys/socket.h>
#include <vector>

void Channel::addOperator(Client* client)
{
	if (!isOperator(client))
		_operators.push_back(client);
}

void Channel::removeOperator(Client* client)
{
	_operators.erase(std::remove(_operators.begin(), _operators.end(), client), _operators.end());
}

bool Channel::isOperator(Client* client) const
{
	return std::find(_operators.begin(), _operators.end(), client) != _operators.end();
}


Channel::Channel(const std::string &name)
	: _name(name) {}

const std::string &Channel::getName() const
{
	return _name;
}

void Channel::addClient(Client *client)
{
	_clients.insert(client);
}

void Channel::removeClient(Client *client)
{
	_clients.erase(client);
}

bool Channel::hasClient(Client *client) const
{
	return _clients.find(client) != _clients.end();
}

void Channel::broadcast(const std::string &message, Client *sender)
{
	for (std::set<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		if (*it != sender)
			send((*it)->getFd(), message.c_str(), message.size(), 0);
}
