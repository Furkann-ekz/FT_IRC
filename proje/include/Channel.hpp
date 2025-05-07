#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Client;

class Channel {
public:
	Channel(const std::string &name);

	const std::string &getName() const;
	void addClient(Client *client);
	void removeClient(Client *client);
	bool hasClient(Client *client) const;
	void broadcast(const std::string &message, Client *sender);

private:
	std::string _name;
	std::set<Client *> _clients;
};

#endif
