#include "../include/Client.hpp"
#include <iostream>
#include <ctime>
#include <sys/socket.h>

Client::Client(int fd)
    : _fd(fd), _nickname(""), _username(""), _realname(""),
      _authenticated(false), _registered(false), _lastPingTime(time(NULL)) {}


Client::~Client() {}

int Client::getFd() const {
    return _fd;
}

const std::string& Client::getNickname() const {
    return _nickname;
}

const std::string& Client::getUsername() const {
    return _username;
}

const std::string& Client::getRealname() const {
    return _realname;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}

bool Client::isRegistered() const {
    return _registered;
}

void Client::setNickname(const std::string& nickname) {
    _nickname = nickname;
}

void Client::setUsername(const std::string& username) {
    _username = username;
}

void Client::setRealname(const std::string& realname) {
    _realname = realname;
}

void Client::authenticate() {
    _authenticated = true;
    std::cout << "DEBUG: Client authenticated" << std::endl;
}

void Client::registerUser() {
    _registered = true;
    std::cout << "DEBUG: Client registered" << std::endl;
}

void Client::updatePingTime() {
    _lastPingTime = time(NULL);
}

time_t Client::getLastPingTime() const {
    return _lastPingTime;
}

void Client::handlePingResponse() {
    // PONG mesajı alındığında son ping zamanını güncelle
    _lastPingTime = time(NULL);
    std::cout << "Received PONG from server, last ping time updated." << std::endl;
}

void Client::sendPing() {
    // Server'a PING mesajı gönder
    std::string ping_msg = "PING :ping\r\n";
    send(_fd, ping_msg.c_str(), ping_msg.length(), 0);
    std::cout << "Sent PING to server." << std::endl;
}