#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _recvBuffer;
    std::string _sendBuffer;
    bool _authenticated;
    bool _registered;

public:
    Client(int fd);
    ~Client();

    int getFd() const;

    const std::string& getNickname() const;
    const std::string& getUsername() const;
    const std::string& getRealname() const;

    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);

    bool isAuthenticated() const;
    void setAuthenticated(bool value);

    bool isRegistered() const;
    void setRegistered(bool value);

    std::string& getRecvBuffer();
    std::string& getSendBuffer();
};

#endif
