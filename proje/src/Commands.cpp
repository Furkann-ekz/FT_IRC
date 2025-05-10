#include "../include/Commands.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"
#include "../include/Server.hpp"  // Gereken tanım burada

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <map>

extern Server* g_server;
static std::string g_password;
static std::map<std::string, Channel*> g_channels;

void Commands::execute(Client* client, const std::string& input)
{
	std::vector<std::string> tokens = split(input);
	if (tokens.empty())
		return;

	const std::string& command = tokens[0];

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
	else if (command == "KICK")
    	handleKick(client, tokens);
	else
		std::cout << "Unknown command: " << command << std::endl;
}

static bool isValidChannelName(const std::string& name)
{
	return !name.empty() && name[0] == '#' && name.length() <= 50;
}



void Commands::handleJoin(Client* client, const std::vector<std::string>& tokens)
{
	if (!client->isAuthenticated() || !client->isRegistered())
	{
		std::cout << "JOIN rejected: Not fully registered." << std::endl;
		return;
	}

	if (tokens.size() < 2)
	{
		std::cout << "JOIN: Missing channel name." << std::endl;
		return;
	}

	std::string channelName = tokens[1];
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
		channel = g_channels[channelName];

	if (!channel->hasClient(client))
	{
		channel->addClient(client);
		std::cout << "User " << client->getNickname() << " joined channel " << channelName << std::endl;
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

	std::string fullMessage = ":" + sender->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";

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
			channel->broadcast(fullMessage, sender);
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

	std::string quitMsg = ":" + client->getNickname() + " QUIT :" + reason + "\r\n";

	std::map<int, Client*>& clients = g_server->getClients();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second != client)
			send(it->second->getFd(), quitMsg.c_str(), quitMsg.length(), 0);
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
        std::cout << "PASS: Missing password." << std::endl;
        return;
    }
    std::string pass = tokens[1];
    if (pass != g_password)
    {
        std::cout << "PASS: Incorrect password." << std::endl;
        client->setAuthenticated(false);

        std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
        std::string errMsg = "ERROR :Closing Link: " + nick + " (Incorrect password)\r\n";
        send(client->getFd(), errMsg.c_str(), errMsg.length(), 0);

        g_server->removeClient(client->getFd());
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
    std::string reason = "Kicked";

    if (tokens.size() >= 4)
    {
        reason = tokens[3];
        for (size_t i = 4; i < tokens.size(); ++i)
            reason += " " + tokens[i];

        if (reason[0] == ':')
            reason = reason.substr(1);
    }

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

    // Hedef kullanıcıyı bul
    std::map<int, Client*>& clients = g_server->getClients();
    Client* target = NULL;
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

    std::string kickMsg = ":" + client->getNickname() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg, NULL); // herkese gönder
    channel->removeClient(target);

    std::cout << "User " << targetNick << " kicked from " << channelName << " by " << client->getNickname() << std::endl;

    // Kanal boşsa sil
    if (!channel->hasClient(target))
    {
        delete channel;
        g_channels.erase(channelName);
        std::cout << "Deleted empty channel: " << channelName << std::endl;
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
		std::cout << "NICK: Missing nickname." << std::endl;
		return;
	}
	client->setNickname(tokens[1]);
	std::cout << "Nickname set to: " << tokens[1] << std::endl;
	tryRegister(client);
}

void Commands::handleUser(Client* client, const std::vector<std::string>& tokens)
{
	if (tokens.size() < 5) {
		std::cout << "USER: Not enough parameters." << std::endl;
		return;
	}
	client->setUsername(tokens[1]);
	client->setRealname(tokens[4]);
	std::cout << "Username: " << tokens[1] << ", Realname: " << tokens[4] << std::endl;
	tryRegister(client);  // <---

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
		delete it->second; // Her channel nesnesini sil
	g_channels.clear(); // Haritayı temizle
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

    std::string message = ":" + client->getNickname() + " PART " + channelName + "\r\n";
    channel->broadcast(message, NULL);

    channel->removeClient(client);
    std::cout << "User " << client->getNickname() << " left channel " << channelName << std::endl;

    // Kanal boş kaldıysa bellekten sil
    if (!channel->hasClient(client))
	{
        delete channel;
        g_channels.erase(channelName);
        std::cout << "Deleted empty channel: " << channelName << std::endl;
    }
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

	// Kullanıcı modu (örnek: MODE fekiz +i)
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

		std::string modeStr = "+";
		if (client->hasUserMode('i'))
			modeStr += "i"; // sadece +i destekliyoruz

		std::string reply = ":irc.localhost 221 " + client->getNickname() + " " + modeStr + "\r\n";
		send(client->getFd(), reply.c_str(), reply.length(), 0);
		return;
	}

	// Kanal modu (örnek: MODE #kanal +o nick)
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

	// +o / -o işlemi
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
		else if (mode == "-o")
			channel->removeOperator(targetClient);

		std::string modeMsg = ":" + client->getNickname() + " MODE " + target + " " + mode + " " + nick + "\r\n";
		channel->broadcast(modeMsg, NULL);
	}
	else
	{
		std::string err = "472 " + client->getNickname() + " " + mode + " :Unknown MODE flag\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
	}
}

