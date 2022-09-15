#ifndef REVELATION_AGENT_HPP
#define REVELATION_AGENT_HPP

#include "position.hpp"
#include "player.hpp"
#include "character.hpp"
#include "deck.hpp"
#include "decision.hpp"
#include <iostream>
#include <random>

class State;

class Agent {
protected:
    uint myId : 1;

    const Player& getMyPlayer(const State& state) const;
public:
    Agent(unsigned int myId) : myId(myId) {};
    virtual ~Agent() = default;
    virtual void onBegin(const State&) {};
    virtual DiscardDecision getDiscard(const State& state) = 0;
    virtual MoveDecision getMovement(const State& state, unsigned nb) = 0;
    virtual AbilityDecision getAbility(const State&) { return {}; };
    virtual ActionDecision getAction(const State& state) = 0;
    virtual unsigned int getSpecialAction(const State& state, Effect& effect) = 0;
};

class StepByStepAgent: public Agent {
    const Character& chooseCharacter(const State& state);
protected:
    virtual uint input(uint min, uint max) = 0;
    virtual std::ostream& ostream() = 0;
public:
    StepByStepAgent(uint myId) : Agent(myId) {};
    DiscardDecision getDiscard(const State&) override;
    MoveDecision getMovement(const State& state, unsigned int) override;
    ActionDecision getAction(const State& state) override;
    unsigned int getSpecialAction(const State& state, Effect& effect) override;
};

class HumanAgent: public StepByStepAgent {
protected:
    uint input(uint min, uint max) override;
    std::ostream& ostream() override { return std::cout; }
public:
    HumanAgent(uint myId);
};

class RandomAgent: public StepByStepAgent {
    std::minstd_rand generator;
    class NoopOstream: public std::ostream {
        template<typename T> std::ostream& operator<<(const T&){ /*do nothing*/ return *this; }
    } myostream;
protected:
    uint input(uint min, uint max) override;
    std::ostream& ostream() override { return myostream; }
public:
    RandomAgent(uint myId);
};

#endif //REVELATION_AGENT_HPP
