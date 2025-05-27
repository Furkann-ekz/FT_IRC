#include "../include/Client.hpp"

Client::Client(int fd)
	: _fd(fd),
	  _nickname(""),
	  _username(""),
	  _realname(""),
	  _recvBuffer(""),
	  _sendBuffer(""),
	  _authenticated(false),
	  _registered(false)
{}


Client::~Client() {}

int Client::getFd() const
{
	return _fd;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

const std::string& Client::getUsername() const
{
	return _username;
}

const std::string& Client::getRealname() const
{
	return _realname;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

void Client::setRealname(const std::string& realname)
{
	_realname = realname;
}

bool Client::isAuthenticated() const
{
	return _authenticated;
}

void Client::setAuthenticated(bool value)
{
	_authenticated = value;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setRegistered(bool value)
{
	_registered = value;
}

std::string& Client::getRecvBuffer()
{
	return _recvBuffer;
}

std::string& Client::getSendBuffer()
{
	return _sendBuffer;
}

void Client::addUserMode(char mode)
{
	_userModes.insert(mode);
}

void Client::removeUserMode(char mode)
{
	_userModes.erase(mode);
}

bool Client::hasUserMode(char mode) const
{
	return _userModes.find(mode) != _userModes.end();
}
