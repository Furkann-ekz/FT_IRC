#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>

// Forward declaration
class Client;
class Server;

class Commands
{
	public:
		static std::vector<std::string> split(const std::string& input);
    	static void setPassword(const std::string& password);
    	static void execute(Client* client, const std::string& input);
		static void tryRegister(Client* client);
		static void handlePrivmsg(Client* sender, const std::vector<std::string>& tokens);
		static void cleanupChannels();
	
		private:
			static void handlePass(Client* client, const std::vector<std::string>& tokens);
			static void handleNick(Client* client, const std::vector<std::string>& tokens);
			static void handleUser(Client* client, const std::vector<std::string>& tokens);
			static void handleJoin(Client* client, const std::vector<std::string>& tokens);
		
};

#endif
