#ifndef REVELATION_PY_AGENT_HPP
#define REVELATION_PY_AGENT_HPP

#include "step_agent.hpp"
#include "agent.hpp"
#include <boost/python.hpp>
#include <string_view>

namespace bpy = boost::python;

class PyStrategy {
    bpy::object agent;
public:
    PyStrategy(std::string_view module_name);
    template<typename ReturnType, typename... Args>
    ReturnType call_python_method(const char* funcName, Args&&... args);
};

// I'm not sure why the wrapper<Agent> is necessary
class PyAgent final: public Agent, public bpy::wrapper<Agent> {
    PyStrategy strategy;
public:
    PyAgent(unsigned int myId, std::string_view filename): Agent(myId), strategy(filename) {};
    const Team& getTeam(const UnitsRepository& repo) override;
    DiscardDecision getDiscard(const State &state) override;
    MoveDecision getMovement(const State &state, unsigned int nb) override;
    ActionDecision getAction(const State &state) override;
    unsigned int getSpecialAction(const State &state, const Effect &effect) override;
};

class SimplePyAgent final: public StepByStepAgent, public bpy::wrapper<StepByStepAgent> {
    PyStrategy strategy;
public:
    SimplePyAgent(unsigned int myId, std::string_view filename): StepByStepAgent(myId), strategy(filename) {};
    uint choose(const OptionList& list, const std::string_view& message) override;
};

#endif //REVELATION_PY_AGENT_HPP
