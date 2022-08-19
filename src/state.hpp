#ifndef REVELATION_STATE_HPP
#define REVELATION_STATE_HPP

#include "constants.hpp"
#include "deck.hpp"
#include "team.hpp"
#include "step_impl.hpp"
#include "agent.hpp"
#include "character.hpp"
#include "BoardTile.hpp"
#include "effect.hpp"
#include "memory.hpp"
#include "nlohmann/json_fwd.hpp"
#include <vector>
#include <tuple>
#include <forward_list>

using json = nlohmann::json;

using Board = std::array<std::array<BoardTile, FULL_BOARD_WIDTH>, 2>;
using UnitList = std::array<NullableShared<Character>, ARMY_SIZE>;

class State {
    Board board;
    std::array<unsigned short int, 2> nbAliveUnits;
    Deck<Faction> resDeck;
    int turnID;
    std::forward_list<Effect*> unresolvedSpecialAbility;
public:
    Timestep timestep;
    uint8_t iActive;
    std::array<UnitList, 2> units;
    std::array<Player, 2> players;

    State() = default;
    State(Board board, std::array<UnitList, 2> units, std::array<Player, 2> players,
          Deck<Faction> resDeck, Timestep timestep, int turnID);
    static State createStart(std::array<Team, 2> teams);

    const BoardTile& getBoardField(position coords) const;
    Character* getBoardFieldDeref(position coords);
    const Character* getBoardFieldDeref(position coords) const;
    void setBoardField(position coords, BoardTile value);
    void setBoardFieldDeref(position coords, BoardTile value);

    bool isFinished() const;
    void checkConsistency() const;

    std::tuple<State, uptr<DrawStep>> stepDraw(ActionOrResource decision) const;
    std::tuple<State, uptr<MoveStep>> stepMov(MoveDecision decision) const;
    std::tuple<State, uptr<AbilityStep>> stepAbil(const AbilityDecision& decision) const;
    std::tuple<State, uptr<ActionStep>> stepAct(ActionDecision decision) const;
    std::tuple<State, uptr<BeginStep>> beginTurn() const;
    std::tuple<State, uptr<Step>> resolveSpecialAction() const;
    std::tuple<State, uptr<Step>> advance(Agent& active, Agent& opponent) const;

    std::vector<MoveDecision> allMovementsForCharacter(const Character& character) const;
    std::vector<const Character*> allAttacksForCharacter(const Character* attacker, unsigned int attackingTeam) const;
    std::map<const Character*, std::vector<const Character*>> allAttacks() const;

    static State invalid() { State retVal; retVal.iActive = 3; return retVal; }
    inline static bool isInvalid(const State& state) { return state.iActive == 3; }

    friend void to_json(json& j, const State& state);
};

constexpr std::nullptr_t DEAD_UNIT = nullptr;

inline bool isDead(const Character* cha){ return cha == nullptr; }
inline bool isDead(const NullableShared<Character>& cha){ return cha.pt() == nullptr; }

#endif //REVELATION_STATE_HPP
