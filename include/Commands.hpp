#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>

// Forward declaration
class Client;
class Server;

std::string getPrefix(Client* client);

class Commands
{
	public:
		static std::vector<std::string> split(const std::string& input);
    	static void setPassword(const std::string& password);
    	static void execute(Client* client, const std::string& input);
		static void tryRegister(Client* client);
		static void cleanupChannels();		
	
		private:
			static void handleQuit(Client* client, const std::vector<std::string>& tokens);
			static void handlePrivmsg(Client* sender, const std::vector<std::string>& tokens);
			static void handlePart(Client* client, const std::vector<std::string>& tokens);
			static void handlePass(Client* client, const std::vector<std::string>& tokens);
			static void handleNick(Client* client, const std::vector<std::string>& tokens);
			static void handleUser(Client* client, const std::vector<std::string>& tokens);
			static void handleJoin(Client* client, const std::vector<std::string>& tokens);
			static void handlePing(Client* client, const std::vector<std::string>& tokens);
			static void handleMode(Client* client, const std::vector<std::string>& tokens);
			static void handleKick(Client* client, const std::vector<std::string>& tokens);
			static void handleNames(Client* client, const std::vector<std::string>& tokens);
			static void handleTopic(Client* client, const std::vector<std::string>& tokens);
			static void handleInvite(Client* client, const std::vector<std::string>& tokens);
			static void handleList(Client* client, const std::vector<std::string>& tokens);
			static void handleNotice(Client* client, const std::vector<std::string>& tokens);
			static void handleWho(Client* client, const std::vector<std::string>& tokens);
};

#endif
