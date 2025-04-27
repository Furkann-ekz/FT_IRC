#include "../include/Channel.hpp"

Channel::Channel(const std::string& name)
    : _name(name) {}

Channel::~Channel() {}

const std::string& Channel::getName() const {
    return _name;
}

void Channel::addClient(Client* client) {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i] == client)
            return; // Zaten kanalda varsa ekleme
    }
    _clients.push_back(client);
}

void Channel::removeClient(Client* client) {
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            _clients.erase(it);
            return;
        }
    }
}

bool Channel::hasClient(Client* client) const {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i] == client)
            return true;
    }
    return false;
}

size_t Channel::getClientCount() const {
    return _clients.size();
}

const std::vector<Client*>& Channel::getClients() const {
    return _clients;
}
