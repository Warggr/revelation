#ifndef REVELATION_AGENT_SETUP_HPP
#define REVELATION_AGENT_SETUP_HPP

#include "nlohmann/json.hpp"
#include "random.hpp"
#include <exception>
#include <string>
#include <array>
#include <optional>

constexpr unsigned NB_AGENTS = 2;

using nlohmann::json;
using nlohmann::json_pointer;

class Agent;
class ServerRoom;
struct Team;

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
    enum AgentType { BOT, LOCAL, NETWORK, RANDOM, TIMEOUT_PROXY } type;
    void* data = nullptr;
    AgentDescriptor() = default;
    AgentDescriptor(AgentType type): type(type){}
};

using AgentDescription = std::array<AgentDescriptor, NB_AGENTS>;
using Agents = std::array<std::unique_ptr<Agent>, NB_AGENTS>;

struct GameDescription {
    AgentDescription agents;
    std::optional<Generator::result_type> seed;
    std::array<std::optional<std::string>, NB_AGENTS> teams;
};

extern const std::string_view grammar_as_json_string;
GameDescription parseGame(const json& j);
Agents agentsFromDescription(AgentDescription&& descr, ServerRoom& room);

#endif //REVELATION_AGENT_SETUP_HPP
