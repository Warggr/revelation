#ifndef REVELATION_AGENT_HPP
#define REVELATION_AGENT_HPP

#include "position.hpp"
#include "player.hpp"
#include "character.hpp"
#include "deck.hpp"
#include "decision.hpp"

class State;

class Agent {
protected:
    uint myId : 1;

    const Player& getMyPlayer(const State& state) const;
public:
    Agent(unsigned int myId) : myId(myId) {};
    virtual ~Agent() = default;
    virtual ActionOrResource getDrawAction(const State& state) = 0;
    virtual void onBegin(const State&) {};
    virtual MoveDecision getMovement(const State& state, unsigned nb) = 0;
    virtual AbilityDecision getAbility(const State&) { return {}; };
    virtual ActionDecision getAction(const State& state) = 0;
    virtual unsigned int getSpecialAction(const State& state, Effect& effect) = 0;
};

class HumanAgent: public Agent {
    const Character& chooseCharacter(const State& state) const;
public:
    HumanAgent(uint myId);
    ActionOrResource getDrawAction(const State&) override;
    MoveDecision getMovement(const State& state, unsigned int) override;
    ActionDecision getAction(const State& state) override;
    unsigned int getSpecialAction(const State& state, Effect& effect) override;
};

#endif //REVELATION_AGENT_HPP
