#ifndef MESSAGEPARSER_HPP
#define MESSAGEPARSER_HPP

#include <string>
#include <vector>

class MessageParser {
public:
    static std::vector<std::string> extractCommands(std::string& buffer);
};

#endif
