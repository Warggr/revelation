#ifndef REVELATION_AGENT_HPP
#define REVELATION_AGENT_HPP

#include "position.hpp"
#include "player.hpp"
#include "character.hpp"
#include "deck.hpp"

class State;

struct MoveDecision {
    position from;
    position to;
    std::vector<Direction> moves;
    MoveDecision(position from, position to, std::vector<Direction> moves): from(from), to(to), moves(std::move(moves)) {};

    static MoveDecision pass() { return { {}, {}, {} }; };
};

bool isPass(const MoveDecision& decision){
    return decision.moves.empty();
}

//static_assert( isPass( MoveDecision::pass() ) );

class Agent {
protected:
    uint8_t myId;

    const Player& getMyPlayer(const State& state) const;
public:
    virtual ActionOrResource getDrawAction(const State& state) = 0;
    virtual void onBegin(const State& state) {};
    virtual MoveDecision getMovement(const State& state) = 0;
    virtual AbilityDecision getAbility(const State& state) { return {}; };
    virtual ActionDecision getAction(const State& state) = 0;
};

#endif //REVELATION_AGENT_HPP
