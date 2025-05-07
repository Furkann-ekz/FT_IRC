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
    else
        std::cout << "Unknown command: " << command << std::endl;
}

void Commands::handlePrivmsg(Client* sender, const std::vector<std::string>& tokens)
{
    if (!sender->isRegistered()) {
        std::cout << "PRIVMSG rejected: Client not registered." << std::endl;
        return;
    }

    if (tokens.size() < 3) {
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

    if (target[0] == '#') {
        std::map<std::string, Channel*>::iterator it = g_channels.find(target);
        if (it != g_channels.end()) {
            Channel* channel = it->second;
            if (!channel->hasClient(sender)) {
                std::cout << "PRIVMSG: Sender is not in channel " << target << std::endl;
                return;
            }
            channel->broadcast(fullMessage, sender);
        } else {
            std::cout << "PRIVMSG: No such channel " << target << std::endl;
        }
    } else {
        std::map<int, Client*>& clients = g_server->getClients();
        for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second->getNickname() == target) {
                send(it->second->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
                return;
            }
        }
        std::cout << "PRIVMSG: No such nick " << target << std::endl;
    }
}



void Commands::handlePass(Client* client, const std::vector<std::string>& tokens)
{
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
        // Şifre yanlışsa bağlantıyı kesmek istersen:
        return;
    }

    client->setAuthenticated(true);
    std::cout << "PASS received: " << pass << std::endl;
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
        std::cout << "Client " << client->getNickname() << " registered successfully." << std::endl;
    }
}

void Commands::cleanupChannels()
{
    for (std::map<std::string, Channel*>::iterator it = g_channels.begin(); it != g_channels.end(); ++it)
        delete it->second; // Her channel nesnesini sil
    g_channels.clear(); // Haritayı temizle
}
