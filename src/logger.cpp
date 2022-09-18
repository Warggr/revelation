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
    Server server;
    std::thread networkThread;
public:
    LiveServerAndLogger(const char* ipAddress, unsigned short port, const std::string& start): server(ipAddress, port, std::string(start)){
        networkThread = std::thread(&Server::start, &server);
    };
    ~LiveServerAndLogger() override {
        server.stop(); networkThread.join();
    }
    void addStep(const json& j) override {
        server.send(j.dump());
    }
};

Logger* Logger::liveServer(const char* ipAddress, unsigned short port){
    subLoggers.push_back(std::make_unique<LiveServerAndLogger>(ipAddress, port, startState));
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
