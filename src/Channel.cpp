#include "../include/Channel.hpp"

Channel::Channel(const std::string& name)
    : _name(name), _inviteOnly(false), _topicRestricted(false), _key(""), _userLimit(-1) {}

Channel::~Channel() {}

const std::string& Channel::getName() const { return _name; }
size_t Channel::getClientCount() const { return _clients.size(); }
const std::vector<Client*>& Channel::getClients() const { return _clients; }

void Channel::addClient(Client* client) {
    if (!hasClient(client)) {
        _clients.push_back(client);
        if (_clients.size() == 1)
            addOperator(client);
    }
}

void Channel::removeClient(Client* client) {
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            _clients.erase(it);
            break;
        }
    }
    removeOperator(client);
}

bool Channel::hasClient(Client* client) const {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i] == client)
            return true;
    }
    return false;
}

void Channel::setInviteOnly(bool value) { _inviteOnly = value; }
bool Channel::isInviteOnly() const { return _inviteOnly; }

void Channel::setTopicRestricted(bool value) { _topicRestricted = value; }
bool Channel::isTopicRestricted() const { return _topicRestricted; }

void Channel::setKey(const std::string& key) { _key = key; }
const std::string& Channel::getKey() const { return _key; }

void Channel::setUserLimit(int limit) { _userLimit = limit; }
int Channel::getUserLimit() const { return _userLimit; }



void Channel::addOperator(Client* client) {
    if (!isOperator(client)) {
        _operators.push_back(client);
    }
}

void Channel::removeOperator(Client* client) {
    for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if (*it == client) {
            _operators.erase(it);
            break;
        }
    }
}


void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

const std::string& Channel::getTopic() const {
    return _topic;
}


bool Channel::isOperator(Client* client) const {
    for (size_t i = 0; i < _operators.size(); ++i) {
        if (_operators[i] == client)
            return true;
    }
    return false;
}

void Channel::inviteClient(Client* client) {
    if (!isInvited(client))
        _invited.push_back(client);
}

bool Channel::isInvited(Client* client) const {
    for (size_t i = 0; i < _invited.size(); ++i) {
        if (_invited[i] == client)
            return true;
    }
    return false;
}

