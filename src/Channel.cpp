#include "../include/Channel.hpp"

Channel::Channel(const std::string& name) : _name(name) {}
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

void Channel::addOperator(Client* client) {
    if (!isOperator(client))
        _operators.push_back(client);
}

void Channel::removeOperator(Client* client) {
    for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if (*it == client) {
            _operators.erase(it);
            break;
        }
    }
}

bool Channel::isOperator(Client* client) const {
    for (size_t i = 0; i < _operators.size(); ++i) {
        if (_operators[i] == client)
            return true;
    }
    return false;
}
