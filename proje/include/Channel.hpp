#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <vector>

class Client;

class Channel
{
	public:
		Channel(const std::string &name);

		const std::string &getName() const;
		void addClient(Client *client);
		void removeClient(Client *client);
		bool hasClient(Client *client) const;
		void broadcast(const std::string &message, Client *sender);
		void addOperator(Client* client);
		void removeOperator(Client* client);
		bool isOperator(Client* client) const;


	private:
		std::string _name;
		std::set<Client *> _clients;
		std::vector<Client*> _operators;
};

#endif
