#ifndef REVELATION_TIMEOUT_HPP
#define REVELATION_TIMEOUT_HPP

#include "agent.hpp"
#include <memory>

class AgentTimeoutProxy: public Agent {
    std::unique_ptr<Agent> agent;
    template<typename RetType, typename... Args>
    RetType timeout(RetType (Agent::*function)(Args...), Args... args);
public:
    AgentTimeoutProxy(unsigned int myId, std::unique_ptr<Agent> agent): Agent(myId), agent(std::move(agent)) {};
    void sync_init() override { agent->sync_init(); }
    const Team& getTeam(const UnitsRepository& repo) override;
    void onBegin(const State& state) override;

    DiscardDecision getDiscard(const State& state) override;
    MoveDecision getMovement(const State& state, unsigned nb) override;
    ActionDecision getAction(const State& state) override;
    unsigned int getSpecialAction(const State& state, const Effect& effect) override;

    Agent& getFallback() override { return agent->getFallback(); }
    void interrupt() override { agent->interrupt(); Agent::interrupt(); }
};

#endif //REVELATION_TIMEOUT_HPP
