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
    std::vector<Client*> _invited;
    std::string _topic;
    bool _inviteOnly;
    bool _topicRestricted;
    std::string _key;
    int _userLimit;


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
    void inviteClient(Client* client);
    bool isInvited(Client* client) const;
    void setTopic(const std::string& topic);
    const std::string& getTopic() const;
    // Getter/setter'lar
    void setInviteOnly(bool value);
    bool isInviteOnly() const;

    void setTopicRestricted(bool value);
    bool isTopicRestricted() const;

    void setKey(const std::string& key);
    const std::string& getKey() const;

    void setUserLimit(int limit);
    int getUserLimit() const;


};

#endif
