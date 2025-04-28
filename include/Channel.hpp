#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel {
private:
    std::string _name;
    std::vector<Client*> _clients;
    std::vector<Client*> _operators;

public:
    Channel(const std::string& name);
    ~Channel();

    const std::string& getName() const;
    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;
    size_t getClientCount() const;
    const std::vector<Client*>& getClients() const;

    void addOperator(Client* client);
    void removeOperator(Client* client);
    bool isOperator(Client* client) const;
};

#endif
