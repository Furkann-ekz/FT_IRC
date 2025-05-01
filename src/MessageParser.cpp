#include "../include/MessageParser.hpp"

std::vector<std::string> MessageParser::extractCommands(std::string& buffer) {
    std::vector<std::string> commands;
    size_t pos;

    // Hem \r\n hem de \n destekleniyor
    while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string line = buffer.substr(0, pos);

        // Eğer önceki karakter \r ise onu da temizle
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }

        if (!line.empty()) {
            commands.push_back(line);
        }

        buffer.erase(0, pos + 1);
    }

    return commands;
}
