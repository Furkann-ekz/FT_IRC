#include "../include/Client.hpp"
#include <iostream>

Client::Client(int fd)
    : _fd(fd), _nickname(""), _username(""), _realname(""),
      _authenticated(false), _registered(false) {}

Client::~Client() {}

int Client::getFd() const { return _fd; }
const std::string& Client::getNickname() const { return _nickname; }
const std::string& Client::getUsername() const { return _username; }
const std::string& Client::getRealname() const { return _realname; }
bool Client::isAuthenticated() const { return _authenticated; }
bool Client::isRegistered() const { return _registered; }

void Client::setNickname(const std::string& nickname) { _nickname = nickname; }
void Client::setUsername(const std::string& username) { _username = username; }
void Client::setRealname(const std::string& realname) { _realname = realname; }
void Client::authenticate() { _authenticated = true; }
void Client::registerUser() { _registered = true; }

void Client::appendToBuffer(const std::string& data) {
    _recvBuffer += data;
}

std::string& Client::getBuffer() {
    return _recvBuffer;
}

void Client::setBuffer(const std::string& buffer) {
    _recvBuffer = buffer;
}