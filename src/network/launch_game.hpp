#ifndef REVELATION_LAUNCH_GAME_HPP
#define REVELATION_LAUNCH_GAME_HPP

#include "nlohmann/json.hpp"
#include <exception>
#include <string>
#include <array>

constexpr unsigned NB_AGENTS = 2;

using nlohmann::json;
using nlohmann::json_pointer;

class Agent;
class ServerRoom;

class AgentCreationException : public std::exception {
    std::string _what;
public:
    AgentCreationException(const json_pointer<json>& where, const std::string& what){
        _what = std::string("~") + where.to_string() + ": " + what;
    }
    const char* what() const noexcept override {
        return _what.c_str();
    }
};

struct AgentDescriptor{
    enum AgentType { BOT, LOCAL, NETWORK, RANDOM } type;
    void* data = nullptr;
    AgentDescriptor() = default;
    AgentDescriptor(AgentType type): type(type){}
};

using AgentDescription = std::array<AgentDescriptor, NB_AGENTS>;
using Agents = std::array<std::unique_ptr<Agent>, NB_AGENTS>;

AgentDescription parseAgents(const json& j);
Agents agentsFromDescription(AgentDescription&& descr, ServerRoom& room);

#endif //REVELATION_LAUNCH_GAME_HPP