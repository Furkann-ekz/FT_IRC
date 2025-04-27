#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <ctime>

class Client {
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    bool _authenticated;
    bool _registered;
	time_t _lastPingTime;

public:
    Client(int fd);
    ~Client();

    int getFd() const;
    const std::string& getNickname() const;
    const std::string& getUsername() const;
    const std::string& getRealname() const;
    bool isAuthenticated() const;
    bool isRegistered() const;

    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);
    void authenticate();
    void registerUser();
	void updatePingTime();
    time_t getLastPingTime() const;
	void handlePingResponse(); // PONG mesajı alındığında
    void sendPing();
};

#endif
