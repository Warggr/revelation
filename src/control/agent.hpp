#ifndef REVELATION_AGENT_HPP
#define REVELATION_AGENT_HPP

#define CONFIG_AGENT_IS_INTERRUPTIBLE

#include "gameplay/decision.hpp"
#include <exception>

class State;
class Player;
struct Character;
struct Team;
class Effect;
class UnitsRepository;

struct AgentSurrenderedException: public std::exception{
    uint id;
    AgentSurrenderedException(uint id): id(id){};
};

struct ProcessContext {
#ifdef CONFIG_AGENT_IS_INTERRUPTIBLE
    bool signal_interrupted = false;
    inline bool isInterrupted() const { return signal_interrupted; }
#else
    inline bool isInterrupted() const { return false; }
#endif
};

class Agent {
    ProcessContext processContext;
protected:
    const ProcessContext& getProcessContext() const { return processContext; }
    uint myId : 1;

    const Player& getMyPlayer(const State& state) const;

    void surrender(){ throw AgentSurrenderedException(myId); }
public:
    Agent(unsigned int myId) : myId(myId) {};
    virtual ~Agent() = default;
    virtual void sync_init(){};

    virtual const Team& getTeam(const UnitsRepository& repo) = 0;

    virtual void onBegin(const State&) {};
    virtual DiscardDecision getDiscard(const State& state) = 0;
    virtual MoveDecision getMovement(const State& state, unsigned nb) = 0;
    virtual AbilityDecision getAbility(const State&) { return {}; };
    virtual ActionDecision getAction(const State& state) = 0;
    virtual unsigned int getSpecialAction(const State& state, const Effect& effect) = 0;

#ifdef CONFIG_AGENT_IS_INTERRUPTIBLE
    virtual void interrupt(){ processContext.signal_interrupted = true; }
    virtual Agent& getFallback();
#endif
};

class SimpleAgent: public Agent {
    static SimpleAgent instance;
    SimpleAgent(): Agent(42) {};
public:
    static SimpleAgent& getInstance(){ return instance; }

    const Team& getTeam(const UnitsRepository& repo) override;
    DiscardDecision getDiscard(const State& state) override;
    MoveDecision getMovement(const State& state, unsigned int nb) override;
    ActionDecision getAction(const State& state) override;
    unsigned int getSpecialAction(const State& state, const Effect& effect) override;
};

#endif //REVELATION_AGENT_HPP
