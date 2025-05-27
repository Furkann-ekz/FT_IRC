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
		~Channel();
		void setTopic(const std::string& topic);
		const std::string& getTopic() const;
		bool hasTopic() const;
		void setKey(const std::string& key);
		void removeKey();
		bool hasKey() const;
		const std::string& getKey() const;
		const std::string &getName() const;
		void addClient(Client *client);
		void removeClient(Client *client);
		bool hasClient(Client *client) const;
		void broadcast(const std::string &message);
		void addOperator(Client* client);
		void removeOperator(Client* client);
		bool isOperator(Client* client) const;
		const std::set<Client*>& getClients() const;
		const std::set<std::string>& getOperatorNicks() const;
		void setLimit(int limit);
		void removeLimit();
		bool hasLimit() const;
		int getLimit() const;
		int getClientCount() const;
		void setTopicRestricted(bool value);
		bool isTopicRestricted() const;
		void setInviteOnly(bool value);
		bool isInviteOnly() const;
		void invite(const std::string& nick);
		bool isInvited(const std::string& nick) const;
		void removeInvite(const std::string& nick);

	private:
		std::set<Client *> _clients;
		std::set<std::string> _invitedNicks;
		std::set<std::string> _operatorNicks;
		std::string _name;
		std::string _topic;
		std::string _key;
		int _userLimit;
		bool _topicRestricted;
		bool _inviteOnly;
};

#endif
