#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include <algorithm>
#include <sys/socket.h>
#include <vector>

void Channel::addOperator(Client* client)
{
	_operatorNicks.insert(client->getNickname());
}

void Channel::removeOperator(Client* client)
{
	_operatorNicks.erase(client->getNickname());
}

bool Channel::isOperator(Client* client) const
{
	return _operatorNicks.find(client->getNickname()) != _operatorNicks.end();
}

const std::set<Client*>& Channel::getClients() const
{
	return _clients;
}

const std::set<std::string>& Channel::getOperatorNicks() const
{
	return _operatorNicks;
}

Channel::Channel(const std::string &name) : _name(name), _key(""), _userLimit(-1), _topic(""), _topicRestricted(false), _inviteOnly(false) {}

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

void Channel::broadcast(const std::string &message)
{
	for (std::set<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		send((*it)->getFd(), message.c_str(), message.size(), 0);
}

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

const std::string& Channel::getTopic() const
{
	return _topic;
}

bool Channel::hasTopic() const
{
	return !_topic.empty();
}

void Channel::setKey(const std::string& key)
{
	_key = key;
}

void Channel::removeKey()
{
	_key.clear();
}

bool Channel::hasKey() const
{
	return !_key.empty();
}

const std::string& Channel::getKey() const
{
	return _key;
}

void Channel::setLimit(int limit)
{
	_userLimit = limit;
}

void Channel::removeLimit()
{
	_userLimit = -1;
}

bool Channel::hasLimit() const
{
	return _userLimit > 0;
}

int Channel::getLimit() const
{
	return _userLimit;
}

int Channel::getClientCount() const
{
	return _clients.size();
}

void Channel::setTopicRestricted(bool value) 
{
	_topicRestricted = value;
}

bool Channel::isTopicRestricted() const
{
	return _topicRestricted;
}

void Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

bool Channel::isInviteOnly() const
{
	return _inviteOnly;
}

void Channel::invite(const std::string& nick)
{
	_invitedNicks.insert(nick);
}

bool Channel::isInvited(const std::string& nick) const
{
	return _invitedNicks.find(nick) != _invitedNicks.end();
}

void Channel::removeInvite(const std::string& nick)
{
	_invitedNicks.erase(nick);
}
