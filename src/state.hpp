#ifndef REVELATION_STATE_HPP
#define REVELATION_STATE_HPP

#include "constants.hpp"
#include "deck.hpp"
#include "team.hpp"
#include <vector>
#include <tuple>
#include "step.hpp"
#include "agent.hpp"
#include "character.hpp"
#include "BoardTile.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class State {
    std::vector<std::vector<BoardTile>> board;
    std::vector<int> nbAliveUnits;
    Deck<Faction> resDeck;
    int turnID;

public:
    Timestep timestep;
    uint8_t iActive;
    std::array<character*, 6> units[2];
    std::vector<Player> players;

    State() = default;
    State(std::vector<std::vector<BoardTile>> board, std::vector<std::vector<character>> units, std::vector<Player> players, Deck<Faction> resDeck, Timestep timestep, int turnID);
    static State createStart(Team teams[2]);
    BoardTile* getBoardField(position coords) const;
    character* getBoardFieldDeref(position coords) const;
    void setBoardField(position coords, BoardTile* value);
    void setBoardFieldDeref(position coords, BoardTile* value);
    bool isFinished() const;
    std::tuple<State, Step> stepDraw(ActionOrResource decision) const;
    void checkConsistency() const;
    std::tuple<State, Step> stepMov(MoveDecision decision) const;
    std::tuple<State, Step> stepAbil(const AbilityDecision& decision) const;
    std::tuple<State, Step> stepAct(ActionDecision decision) const;
    std::tuple<State, Step> beginTurn() const;
    std::tuple<State, Step> advance(Agent agent) const;
    std::vector<MoveDecision> allMovementsForCharacter(character character) const;
    std::map<character*, std::vector<character*>> allAttacks() const;
    json to_json(nlohmann::basic_json<> &j, const State &state) const;

    static State invalid() { State retVal; retVal.iActive = 3; return retVal; }
    inline static bool isInvalid(const State& state) { return state.iActive == 3; }
};

constexpr character* DEAD_UNIT = nullptr;

inline bool isDead(const character* cha){ return cha == nullptr; }

#endif //REVELATION_STATE_HPP
