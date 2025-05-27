#include "../include/Commands.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/Server.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <map>

extern Server* g_server;
static std::string g_password;
std::map<std::string, Channel*> g_channels;


void Commands::execute(Client* client, const std::string& input)
{
	std::vector<std::string> tokens = split(input);
	if (tokens.empty())
		return;

	const std::string& command = tokens[0];

	std::set<std::string> knownCommands;
	knownCommands.insert("PASS");
	knownCommands.insert("NICK");
	knownCommands.insert("USER");
	knownCommands.insert("JOIN");
	knownCommands.insert("PRIVMSG");
	knownCommands.insert("PART");
	knownCommands.insert("QUIT");
	knownCommands.insert("PING");
	knownCommands.insert("MODE");
	knownCommands.insert("NAMES");
	knownCommands.insert("KICK");
	knownCommands.insert("TOPIC");
	knownCommands.insert("WHO");
	knownCommands.insert("INVITE");
	knownCommands.insert("LIST");
	knownCommands.insert("NOTICE");

	if (knownCommands.find(command) == knownCommands.end())
	{
		std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
		std::string errMsg = ":irc.localhost 421 " + nick + " " + command + " :Unknown command\r\n";
		send(client->getFd(), errMsg.c_str(), errMsg.length(), 0);
		std::cout << "Unknown command: " << command << std::endl;
		return;
	}

	if (!client->isAuthenticated() && command != "PASS")
	{
		std::string errMsg = ":irc.localhost 451 * :You have not registered\r\n";
		send(client->getFd(), errMsg.c_str(), errMsg.length(), 0);
		std::cout << command << " rejected: Client not authenticated." << std::endl;
		return;
	}

	if (command == "PASS")
		handlePass(client, tokens);
	else if (command == "NICK")
		handleNick(client, tokens);
	else if (command == "USER")
		handleUser(client, tokens);
	else if (command == "JOIN")
		handleJoin(client, tokens);
	else if (command == "PRIVMSG")
		handlePrivmsg(client, tokens);
	else if (command == "PART")
		handlePart(client, tokens);
	else if (command == "QUIT")
		handleQuit(client, tokens);
	else if (command == "PING")
		handlePing(client, tokens);
	else if (command == "MODE")
		handleMode(client, tokens);
	else if (command == "NAMES")
		handleNames(client, tokens);
	else if (command == "KICK")
		handleKick(client, tokens);
	else if (command == "TOPIC")
		handleTopic(client, tokens);
	else if (command == "WHO")
		handleWho(client, tokens);
	else if (command == "INVITE")
		handleInvite(client, tokens);
	else if (command == "LIST")
		handleList(client, tokens);
	else if (command == "NOTICE")
		handleNotice(client, tokens);
	else
	{
		std::cout << "Unknown command: " << command << std::endl;
		std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
		std::string errMsg = ":irc.localhost 421 " + nick + " " + command + " :Unknown command\r\n";
		send(client->getFd(), errMsg.c_str(), errMsg.length(), 0);
	}
}

static bool isValidChannelName(const std::string& name)
{
	return !name.empty() && name[0] == '#' && name.length() <= 50;
}

void Commands::handleList(Client* client, const std::vector<std::string>& tokens)
{
	(void)tokens;

	std::string startMsg = ":irc.localhost 321 " + client->getNickname() + " Channel :Users Name\r\n";
	send(client->getFd(), startMsg.c_str(), startMsg.length(), 0);

	for (std::map<std::string, Channel*>::iterator it = g_channels.begin(); it != g_channels.end(); ++it)
	{
		Channel* ch = it->second;
		std::string name = ch->getName();
		std::stringstream ss;
		ss << ":irc.localhost 322 " << client->getNickname() << " " << name << " " << ch->getClients().size() << " :No topic\r\n";
		std::string listLine = ss.str();
		send(client->getFd(), listLine.c_str(), listLine.length(), 0);
	}

	std::string endMsg = ":irc.localhost 323 " + client->getNickname() + " :End of /LIST\r\n";
	send(client->getFd(), endMsg.c_str(), endMsg.length(), 0);
}

void Commands::handleNotice(Client* client, const std::vector<std::string>& tokens)
{
	if (!client->isRegistered())
		return;

	if (tokens.size() < 3)
		return;
		

	std::string target = tokens[1];
	std::string message = tokens[2];

	for (size_t i = 3; i < tokens.size(); ++i)
		message += " " + tokens[i];

	if (message[0] == ':')
		message = message.substr(1);

	if (message.empty())
		return;

	std::string fullMessage = getPrefix(client) + " NOTICE " + target + " :" + message + "\r\n";

	if (target[0] == '#')
	{
		std::map<std::string, Channel*>::iterator it = g_channels.find(target);
		if (it != g_channels.end())
		{
			Channel* channel = it->second;
			if (!channel->hasClient(client))
				return;

			channel->broadcast(fullMessage);
		}
	}
	else
	{
		std::map<int, Client*>& clients = g_server->getClients();
		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			if (it->second->getNickname() == target)
			{
				send(it->second->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
				break;
			}
		}
	}
}

void Commands::handleInvite(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3)
	{
		std::string err = "461 " + client->getNickname() + " INVITE :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	std::string targetNick = tokens[1];
	std::string channelName = tokens[2];

	std::map<std::string, Channel*>::iterator chIt = g_channels.find(channelName);
	if (chIt == g_channels.end())
	{
		std::string err = "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Channel* channel = chIt->second;

	if (!channel->hasClient(client))
	{
		std::string err = "442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	if (!channel->isOperator(client))
	{
		std::string err = "482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Client* targetClient = NULL;
	std::map<int, Client*>& clients = g_server->getClients();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second->getNickname() == targetNick)
		{
			targetClient = it->second;
			break;
		}
	}

	if (!targetClient)
	{
		std::string err = "401 " + client->getNickname() + " " + targetNick + " :No such nick\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	if (channel->hasClient(targetClient))
	{
		std::string err = "443 " + client->getNickname() + " " + targetNick + " " + channelName + " :is already on channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	std::string msg = ":irc.localhost 341 " + client->getNickname() + " " + targetNick + " " + channelName + "\r\n";
	send(client->getFd(), msg.c_str(), msg.length(), 0);

	std::string inviteMsg = ":" + client->getNickname() + " INVITE " + targetNick + " :" + channelName + "\r\n";
	send(targetClient->getFd(), inviteMsg.c_str(), inviteMsg.length(), 0);

	channel->invite(targetNick);

	std::cout << "User " << client->getNickname() << " invited " << targetNick << " to " << channelName << std::endl;
}

void Commands::handleWho(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 2)
	{
		std::string err = "461 " + client->getNickname() + " WHO :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	const std::string& channelName = tokens[1];
	std::map<std::string, Channel*>::iterator it = g_channels.find(channelName);
	if (it == g_channels.end())
	{
		std::string err = "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Channel* channel = it->second;
	const std::set<Client*>& members = channel->getClients();

	for (std::set<Client*>::const_iterator cit = members.begin(); cit != members.end(); ++cit)
	{
		Client* c = *cit;

		std::string flag = "H";
		if (channel->isOperator(c))
			flag += "@";

		std::string whoMsg = ":irc.localhost 352 " + client->getNickname() + " " + channelName + " "
			+ c->getUsername() + " localhost irc.localhost " + c->getNickname()
			+ " " + flag + " :0 " + c->getRealname() + "\r\n";

		send(client->getFd(), whoMsg.c_str(), whoMsg.length(), 0);
	}

	std::string endMsg = ":irc.localhost 315 " + client->getNickname() + " " + channelName + " :End of WHO list\r\n";
	send(client->getFd(), endMsg.c_str(), endMsg.length(), 0);
}


void Commands::handleNames(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 2)
	{
		std::string err = "461 " + client->getNickname() + " NAMES :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	std::string channelName = tokens[1];
	std::map<std::string, Channel*>::iterator it = g_channels.find(channelName);
	if (it == g_channels.end())
	{
		std::string err = "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Channel* channel = it->second;
	std::string namesReply = ":irc.localhost 353 " + client->getNickname() + " = " + channelName + " :";

	const std::set<Client*>& clients = channel->getClients();
	for (std::set<Client*>::const_iterator cit = clients.begin(); cit != clients.end(); ++cit)
	{
		if (channel->isOperator(*cit))
			namesReply += "@" + (*cit)->getNickname() + " ";
		else
			namesReply += (*cit)->getNickname() + " ";
	}

	namesReply += "\r\n";

	std::string endMsg = ":irc.localhost 366 " + client->getNickname() + " " + channelName + " :End of /NAMES list.\r\n";

	std::string fullMsg = namesReply + endMsg;
	send(client->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
}


void Commands::handleJoin(Client* client, const std::vector<std::string>& tokens)
{
	if (!client->isAuthenticated() || !client->isRegistered())
	{
		std::cout << "JOIN rejected: Not fully registered." << std::endl;
		std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
		std::string err = ":irc.localhost 451 " + nick + " JOIN :You have not registered\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	if (tokens.size() < 2)
	{
		std::cout << "JOIN: Missing channel name." << std::endl;
		return;
	}

	std::string channelName = tokens[1];
	std::string keyGiven;
	if (tokens.size() >= 3)
		keyGiven = tokens[2];

	if (!isValidChannelName(channelName))
	{
		std::cout << "JOIN: Invalid channel name." << std::endl;
		return;
	}

	Channel* channel;
	if (g_channels.find(channelName) == g_channels.end()) 
	{
		channel = new Channel(channelName);
		g_channels[channelName] = channel;
		std::cout << "Created channel: " << channelName << std::endl;
	}
	else
	{
		channel = g_channels[channelName];

		if (channel->hasKey() && channel->getKey() != keyGiven)
		{
			std::string err = ":irc.localhost 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}

		if (channel->hasLimit() && channel->getClientCount() >= channel->getLimit())
		{
			std::string err = ":irc.localhost 471 " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}

		if (channel->isInviteOnly() && !channel->isInvited(client->getNickname()))
		{
			std::string err = ":irc.localhost 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}
	}

	if (!channel->hasClient(client))
	{
		channel->addClient(client);
		std::cout << "User " << client->getNickname() << " joined channel " << channelName << std::endl;

		if (channel->getClients().size() == 1)
			channel->addOperator(client);

		channel->removeInvite(client->getNickname());

		std::string joinMsg = getPrefix(client) + " JOIN :" + channelName + "\r\n";
		channel->broadcast(joinMsg);

		if (channel->hasTopic())
		{
			std::string topicMsg = ":irc.localhost 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
			send(client->getFd(), topicMsg.c_str(), topicMsg.length(), 0);
		}
		else
		{
			std::string noTopic = ":irc.localhost 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
			send(client->getFd(), noTopic.c_str(), noTopic.length(), 0);
		}

		std::vector<std::string> fakeTokens;
		fakeTokens.push_back("NAMES");
		fakeTokens.push_back(channelName);
		handleNames(client, fakeTokens);
	}
}

std::vector<std::string> Commands::split(const std::string& input)
{
	std::vector<std::string> tokens;
	std::istringstream iss(input);
	std::string token;
	while (iss >> token)
		tokens.push_back(token);
	return tokens;
}

void Commands::handlePing(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 2)
	{
		std::cout << "PING: Missing parameter." << std::endl;
		return;
	}

	std::string token = tokens[1];
	std::string pongReply = "PONG :" + token + "\r\n";
	send(client->getFd(), pongReply.c_str(), pongReply.length(), 0);

	std::cout << "PING received, PONG sent: " << pongReply;
}

void Commands::handlePrivmsg(Client* sender, const std::vector<std::string>& tokens)
{
	if (!sender->isRegistered())
	{
		std::cout << "PRIVMSG rejected: Client not registered." << std::endl;
		return;
	}

	if (tokens.size() < 3)
	{
		std::cout << "PRIVMSG: Not enough parameters." << std::endl;
		return;
	}

	std::string target = tokens[1];
	std::string message = tokens[2];

	for (size_t i = 3; i < tokens.size(); ++i)
		message += " " + tokens[i];

	if (message[0] == ':')
		message = message.substr(1);

	std::string fullMessage = getPrefix(sender) + " PRIVMSG " + target + " :" + message + "\r\n";

	if (target[0] == '#')
	{
		std::map<std::string, Channel*>::iterator it = g_channels.find(target);
		if (it != g_channels.end())
		{
			Channel* channel = it->second;
			if (!channel->hasClient(sender))
			{
				std::cout << "PRIVMSG: Sender is not in channel " << target << std::endl;
				return;
			}
			channel->broadcast(fullMessage);
		}
		else
			std::cout << "PRIVMSG: No such channel " << target << std::endl;
	}
	else
	{
		std::map<int, Client*>& clients = g_server->getClients();
		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			if (it->second->getNickname() == target)
			{
				send(it->second->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
				return;
			}
		}
		std::cout << "PRIVMSG: No such nick " << target << std::endl;
	}
}

void Commands::handleQuit(Client* client, const std::vector<std::string>& tokens)
{
	std::string reason = "Client Quit";
	if (tokens.size() > 1)
		reason = tokens[1];
	if (!reason.empty() && reason[0] == ':')
		reason = reason.substr(1);

	std::string quitMsg = getPrefix(client) + " QUIT :" + reason + "\r\n";

	for (std::map<std::string, Channel*>::iterator it = g_channels.begin(); it != g_channels.end();)
	{
		Channel* channel = it->second;
		if (channel->hasClient(client))
		{
			channel->broadcast(quitMsg);
			channel->removeOperator(client);
			channel->removeClient(client);

			if (channel->getClients().empty())
			{
				delete channel;
				g_channels.erase(it++);
				continue;
			}
		}
		++it;
	}

	int fd = client->getFd();
	g_server->removeClient(fd);
}


void Commands::handlePass(Client* client, const std::vector<std::string>& tokens)
{
	if (client->isAuthenticated())
	{
		std::cout << "PASS rejected: Already authenticated." << std::endl;
		return;
	}
	if (tokens.size() < 2)
	{
		std::string err = ":irc.localhost 461 PASS :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}
	std::string pass = tokens[1];
	if (pass != g_password)
	{
		std::string err = ":irc.localhost 464 * :Password incorrect\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		client->setAuthenticated(false);
		return;
	}
	client->setAuthenticated(true);
	std::cout << "PASS received: " << pass << std::endl;
}

void Commands::handleKick(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3)
	{
		std::string err = "461 " + client->getNickname() + " KICK :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	const std::string& channelName = tokens[1];
	const std::string& targetNick = tokens[2];

	if (g_channels.find(channelName) == g_channels.end())
	{
		std::string err = "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Channel* channel = g_channels[channelName];

	if (!channel->hasClient(client))
	{
		std::string err = "442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	if (!channel->isOperator(client))
	{
		std::string err = "482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Client* target = NULL;
	std::map<int, Client*>& clients = g_server->getClients();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second->getNickname() == targetNick)
		{
			target = it->second;
			break;
		}
	}

	if (!target || !channel->hasClient(target))
	{
		std::string err = "441 " + client->getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	std::string reason = "Kicked";
	if (tokens.size() >= 4)
	{
		reason = tokens[3];
		for (size_t i = 4; i < tokens.size(); ++i)
			reason += " " + tokens[i];
		if (reason[0] == ':')
			reason = reason.substr(1);
	}

	std::string kickMsg = getPrefix(client) + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
	channel->broadcast(kickMsg);
	channel->removeOperator(target);
	channel->removeClient(target);

	if (channel->getClients().empty())
	{
		delete channel;
		g_channels.erase(channelName);
	}
}

void Commands::handleNick(Client* client, const std::vector<std::string>& tokens)
{
	if (!client->isAuthenticated())
	{
		std::cout << "NICK rejected: Not authenticated." << std::endl;
		return;
	}

	if (tokens.size() < 2)
	{
		std::string err = "431 :No nickname given\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	std::string newNick = tokens[1];

	std::map<int, Client*>& clients = g_server->getClients();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second != client && it->second->getNickname() == newNick)
		{
			std::string err = "433 " + newNick + " :Nickname is already in use\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}
	}

	std::string oldPrefix = getPrefix(client);

	client->setNickname(newNick);

	std::string nickMsg = oldPrefix + " NICK :" + newNick + "\r\n";
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
		if (it->second != client)
			send(it->second->getFd(), nickMsg.c_str(), nickMsg.length(), 0);

	std::cout << "Nickname set to: " << newNick << std::endl;

	tryRegister(client);
}

void Commands::handleUser(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 5)
	{
		std::string err = "461 " + client->getNickname() + " USER :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}
	client->setUsername(tokens[1]);
	client->setRealname(tokens[4]);
	std::cout << "Username: " << tokens[1] << ", Realname: " << tokens[4] << std::endl;
	tryRegister(client);

}

void Commands::setPassword(const std::string& password)
{
	g_password = password;
}

void Commands::tryRegister(Client* client)
{
	if (client->isAuthenticated() &&
		!client->getNickname().empty() &&
		!client->getUsername().empty() &&
		!client->getRealname().empty() &&
		!client->isRegistered())
	{
		client->setRegistered(true);
		std::string nick = client->getNickname();
		std::string msg001 = ":irc.localhost 001 " + nick + " :Welcome to the IRC server, " + nick + "!\r\n";
		std::string msg002 = ":irc.localhost 002 " + nick + " :Your host is irc.localhost, running version 0.1\r\n";
		std::string msg003 = ":irc.localhost 003 " + nick + " :This server was created just now\r\n";
		std::string msg004 = ":irc.localhost 004 " + nick + " irc.localhost 0.1 iowghraAsORTVSxNCWqBzvdHtGp\r\n";

		send(client->getFd(), msg001.c_str(), msg001.length(), 0);
		send(client->getFd(), msg002.c_str(), msg002.length(), 0);
		send(client->getFd(), msg003.c_str(), msg003.length(), 0);
		send(client->getFd(), msg004.c_str(), msg004.length(), 0);

		std::cout << "Client " << client->getNickname() << " registered successfully." << std::endl;
	}
}

void Commands::cleanupChannels()
{
	for (std::map<std::string, Channel*>::iterator it = g_channels.begin(); it != g_channels.end(); ++it)
		delete it->second;
	g_channels.clear();
}

void Commands::handlePart(Client* client, const std::vector<std::string>& tokens)
{
	if (!client->isRegistered())
	{
		std::cout << "PART rejected: Client not registered." << std::endl;
		return;
	}

	if (tokens.size() < 2)
	{
		std::cout << "PART: Missing channel name." << std::endl;
		return;
	}

	std::string channelName = tokens[1];
	if (g_channels.find(channelName) == g_channels.end())
	{
		std::cout << "PART: Channel does not exist." << std::endl;
		return;
	}

	Channel* channel = g_channels[channelName];
	if (!channel->hasClient(client))
	{
		std::cout << "PART: You are not in channel " << channelName << std::endl;
		return;
	}

	std::string message = getPrefix(client) + " PART " + channelName + "\r\n";
	channel->broadcast(message);

	channel->removeClient(client);
	std::cout << "User " << client->getNickname() << " left channel " << channelName << std::endl;

	if (!channel->hasClient(client))
	{
		delete channel;
		g_channels.erase(channelName);
		std::cout << "Deleted empty channel: " << channelName << std::endl;
	}
}

std::string getPrefix(Client* client)
{
	return ":" + client->getNickname() + "!" + client->getUsername() + "@localhost";
}

void Commands::handleMode(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 3)
	{
		std::string err = "461 " + client->getNickname() + " MODE :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	const std::string& target = tokens[1];
	const std::string& mode = tokens[2];

	
	if (target[0] != '#')
	{
		if (client->getNickname() != target)
		{
			std::string err = "502 " + client->getNickname() + " :Cannot change mode for other users\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}

		if (mode[0] == '+')
			for (size_t i = 1; i < mode.length(); ++i)
				client->addUserMode(mode[i]);
		else if (mode[0] == '-')
			for (size_t i = 1; i < mode.length(); ++i)
				client->removeUserMode(mode[i]);

		std::string modeStr = "";
		if (client->hasUserMode('i'))
			modeStr += "i";

		std::string reply = ":irc.localhost 221 " + client->getNickname() + " +" + modeStr + "\r\n";
		send(client->getFd(), reply.c_str(), reply.length(), 0);

		std::string broadcast = getPrefix(client) + " MODE " + client->getNickname() + " +" + modeStr + "\r\n";
		send(client->getFd(), broadcast.c_str(), broadcast.length(), 0);
		return;
	}

	if (g_channels.find(target) == g_channels.end())
	{
		std::string err = "403 " + client->getNickname() + " " + target + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Channel* channel = g_channels[target];

	if (!channel->isOperator(client))
	{
		std::string err = "482 " + client->getNickname() + " " + target + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}

	if ((mode == "+o" || mode == "-o") && tokens.size() >= 4)
	{
		const std::string& nick = tokens[3];
		Client* targetClient = NULL;
		std::map<int, Client*>& clients = g_server->getClients();

		for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			if (it->second->getNickname() == nick)
			{
				targetClient = it->second;
				break;
			}
		}

		if (!targetClient)
		{
			std::string err = "401 " + client->getNickname() + " " + nick + " :No such nick\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}

		if (!channel->hasClient(targetClient))
		{
			std::string err = "441 " + client->getNickname() + " " + nick + " " + target + " :They aren't on that channel\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}

		if (mode == "+o")
			channel->addOperator(targetClient);
		else
			channel->removeOperator(targetClient);

		std::string modeMsg = getPrefix(client) + " MODE " + target + " " + mode + " " + nick + "\r\n";
		channel->broadcast(modeMsg);
	}
	else if (mode == "+k" && tokens.size() >= 4)
	{
		std::string key = tokens[3];
		channel->setKey(key);
		std::string msg = getPrefix(client) + " MODE " + target + " +k\r\n";
		channel->broadcast(msg);
	}
	else if (mode == "-k")
	{
		channel->removeKey();
		std::string msg = getPrefix(client) + " MODE " + target + " -k\r\n";
		channel->broadcast(msg);
	}
	else if (mode == "+l" && tokens.size() >= 4)
	{
		std::istringstream iss(tokens[3]);
		int limit;
		if (!(iss >> limit) || limit <= 0)
		{
			std::string err = "461 " + client->getNickname() + " MODE :Invalid limit value\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}

		channel->setLimit(limit);
		std::string msg = getPrefix(client) + " MODE " + target + " +l " + tokens[3] + "\r\n";
		channel->broadcast(msg);
	}
	else if (mode == "-l")
	{
		channel->removeLimit();
		std::string msg = getPrefix(client) + " MODE " + target + " -l\r\n";
		channel->broadcast(msg);
	}
	else if (mode == "+t")
	{
		channel->setTopicRestricted(true);
		std::string msg = getPrefix(client) + " MODE " + target + " +t\r\n";
		channel->broadcast(msg);
	}
	else if (mode == "-t")
	{
		channel->setTopicRestricted(false);
		std::string msg = getPrefix(client) + " MODE " + target + " -t\r\n";
		channel->broadcast(msg);
	}
	else if (mode == "+i")
	{
		channel->setInviteOnly(true);
		std::string msg = getPrefix(client) + " MODE " + target + " +i\r\n";
		channel->broadcast(msg);
	}
	else if (mode == "-i")
	{
		channel->setInviteOnly(false);
		std::string msg = getPrefix(client) + " MODE " + target + " -i\r\n";
		channel->broadcast(msg);
	}
	else
	{
		std::string err = "472 " + client->getNickname() + " " + mode + " :Unknown MODE flag\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
	}

}

void Commands::handleTopic(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 2)
	{
		std::string err = "461 " + client->getNickname() + " TOPIC :Not enough parameters\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	const std::string& channelName = tokens[1];
	std::map<std::string, Channel*>::iterator it = g_channels.find(channelName);
	if (it == g_channels.end())
	{
		std::string err = "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Channel* channel = it->second;

	if (tokens.size() == 2)
	{
		if (channel->hasTopic())
		{
			std::string msg = ":irc.localhost 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n";
			send(client->getFd(), msg.c_str(), msg.length(), 0);
		}
		else
		{
			std::string msg = ":irc.localhost 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
			send(client->getFd(), msg.c_str(), msg.length(), 0);
		}
		return;
	}

	if (channel->isTopicRestricted() && !channel->isOperator(client))
	{
		std::string err = "482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	std::string topic = tokens[2];
	for (size_t i = 3; i < tokens.size(); ++i)
		topic += " " + tokens[i];

	if (topic[0] == ':')
		topic = topic.substr(1);

	channel->setTopic(topic);

	std::string topicMsg = getPrefix(client) + " TOPIC " + channelName + " :" + topic + "\r\n";
	channel->broadcast(topicMsg);
}
