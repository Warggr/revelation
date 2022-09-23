#include "logger.hpp"
#include "nlohmann/json.hpp"
#include "network/server.hpp"
#include <thread>

using json = nlohmann::json;

class FileLogger : public SubLogger {
    std::ostream& file;
    bool firstStep;
public:
    FileLogger(std::ostream& file, const std::string& start): file(file) {
        file << "{\"state\":" << start << ",\"steps\":[";
        firstStep = true;
    }
    ~FileLogger() override {
        file << "]}";
    }
    void addStep(const json& step) override {
        if(firstStep) firstStep = false;
        else file << ',';
        file << step << '\n';
    }
};

class LiveServerAndLogger : public SubLogger {
    ServerRoom& serverRoom;
public:
    LiveServerAndLogger(ServerRoom& serverRoom, const std::string& start): serverRoom(serverRoom){
        serverRoom.setGreeterMessage(start);
    };
    ~LiveServerAndLogger() override {
    }
    void addStep(const json& j) override {
        serverRoom.send(j.dump());
    }
};

Logger* Logger::liveServer(ServerRoom& server){
    subLoggers.push_back(std::make_unique<LiveServerAndLogger>(server, startState));
    return this;
}

Logger* Logger::logToFile(std::ostream& stream){
    subLoggers.push_back(std::make_unique<FileLogger>(stream, startState));
    return this;
}

void Logger::addStep(const uptr<Step>& step) {
    for(const auto& sub : subLoggers)
        sub->addStep(json(*step));
}
