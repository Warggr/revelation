#ifndef REVELATION_AGENT_HPP
#define REVELATION_AGENT_HPP

/**
 * When this flag is set, agents can be interrupted.
 * Agents are then supposed to run in another thread. When the main thread wants to interrupt the agent,
 * it can set the field signal_interrupted of struct ProcessContext to true.
 * Agents are supposed to regularly check this field and stop any computations if it is set.
 */
#define CONFIG_AGENT_IS_INTERRUPTIBLE

struct ProcessContext {
#ifdef CONFIG_AGENT_IS_INTERRUPTIBLE
    bool signal_interrupted = false;
    inline bool isInterrupted() const { return signal_interrupted; }
#else
    inline bool isInterrupted() const { return false; }
#endif
};

#include "gameplay/decision.hpp"
#include <exception>
#include <cassert>

class State;
class Player;
struct Character;
struct Team;
class Effect;
class UnitsRepository;

/* Thrown when an agent concedes the game. */
struct AgentSurrenderedException: public std::exception{
    uint id; //The ID of the player who surrendered.
    AgentSurrenderedException(uint id): id(id){};
};

class Agent {
    ProcessContext processContext;
protected:
    const ProcessContext& getProcessContext() const { return processContext; }
    uint myId : 1;

    const Player& getMyPlayer(const State& state) const;

    void surrender(){ throw AgentSurrenderedException(myId); }
public:
    Agent(unsigned int myId) : myId(myId) { assert(myId == 0 or myId == 1); }
    virtual ~Agent() = default;
    uint getId() const { return myId; }
    // callback method: some agents (e.g. NetworkAgent) need some initialization before they can begin playing.
    // This method waits for this initialization. Since the initialization can be asynchronous, this method can take a long time.
    virtual void sync_init(){};

    virtual const Team& getTeam(const UnitsRepository& repo) = 0;

    // callback method: some agents (e.g. SearchAgent) need to do some computations at the beginning of each turn
    virtual void onBegin(const State&) {};
    virtual DiscardDecision getDiscard(const State& state) = 0;
    virtual MoveDecision getMovement(const State& state, unsigned nb) = 0;
    virtual AbilityDecision getAbility(const State&) { return {}; };
    virtual ActionDecision getAction(const State& state) = 0;
    virtual unsigned int getSpecialAction(const State& state, const Effect& effect) = 0;

#ifdef CONFIG_AGENT_IS_INTERRUPTIBLE
    virtual void interrupt(){ processContext.signal_interrupted = true; }
    virtual Agent& getFallback();
    // When the agent took too long and is interrupted, the game asks for a fallback agent.
    // The fallback agent is supposed to return results faster than the normal agent,
    // so it might have to return less relevant results (e.g. just a random answer)
#endif
};

// This is the default fallback agent. It takes only very simple decisions.
class SimpleAgent: public Agent {
    static SimpleAgent instance;
    SimpleAgent(): Agent(0) {}; // myId shouldn't matter
public:
    static SimpleAgent& getInstance(){ return instance; }

    const Team& getTeam(const UnitsRepository& repo) override;
    DiscardDecision getDiscard(const State& state) override;
    MoveDecision getMovement(const State& state, unsigned int nb) override;
    ActionDecision getAction(const State& state) override;
    unsigned int getSpecialAction(const State& state, const Effect& effect) override;
};

#endif //REVELATION_AGENT_HPP
