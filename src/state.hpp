#ifndef REVELATION_STATE_HPP
#define REVELATION_STATE_HPP

#include "constants.hpp"
#include "deck.hpp"
#include "team.hpp"
#include "step.hpp"
#include "agent.hpp"
#include "character.hpp"
#include "BoardTile.hpp"
#include "nlohmann/json_fwd.hpp"
#include <vector>
#include <tuple>

using json = nlohmann::json;

using Board = std::array<std::array<BoardTile, FULL_BOARD_WIDTH>, 2>;

class State {
    Board board;
    std::vector<int> nbAliveUnits;
    Deck<Faction> resDeck;
    int turnID;

public:
    Timestep timestep;
    uint8_t iActive;
    std::array<std::array<character*, ARMY_SIZE>, 2> units;
    std::array<Player, 2> players;

    State() = default;
    State(Board board, std::array<std::array<character*, ARMY_SIZE>, 2> units, std::array<Player, 2> players,
          Deck<Faction> resDeck, Timestep timestep, int turnID);
    static State createStart(std::array<Team, 2> teams);
    const BoardTile& getBoardField(position coords) const;
    character* getBoardFieldDeref(position coords) const;
    void setBoardField(position coords, BoardTile value);
    void setBoardFieldDeref(position coords, BoardTile value);
    bool isFinished() const;
    std::tuple<State, uptr<DrawStep>> stepDraw(ActionOrResource decision) const;
    void checkConsistency() const;
    std::tuple<State, uptr<MoveStep>> stepMov(MoveDecision decision) const;
    std::tuple<State, uptr<AbilityStep>> stepAbil(const AbilityDecision& decision) const;
    std::tuple<State, uptr<ActionStep>> stepAct(ActionDecision decision) const;
    std::tuple<State, uptr<BeginStep>> beginTurn() const;
    std::tuple<State, uptr<Step>> advance(Agent& agent) const;
    std::vector<MoveDecision> allMovementsForCharacter(character character) const;
    std::map<const character*, std::vector<character*>> allAttacks() const;

    static State invalid() { State retVal; retVal.iActive = 3; return retVal; }
    inline static bool isInvalid(const State& state) { return state.iActive == 3; }

    friend void to_json(json& j, const State& state);
};

constexpr character* DEAD_UNIT = nullptr;

inline bool isDead(const character* cha){ return cha == nullptr; }

#endif //REVELATION_STATE_HPP
