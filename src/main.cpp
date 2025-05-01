#include "../include/Server.hpp"
#include <cstdlib>
#include <iostream>
#include <csignal>

Server* global_server = NULL;

void handleSigInt(int) {
	if (global_server) {
		std::cout << "Shutting down cleanly..." << std::endl;
		global_server->cleanup();
	}
	std::exit(0);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	int port = std::atoi(argv[1]);
	std::string password = argv[2];

	Server server;
	global_server = &server;
	std::signal(SIGINT, handleSigInt);

	server.init(port, password);
	server.run();
	return 0;
}

