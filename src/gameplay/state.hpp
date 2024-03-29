#ifndef REVELATION_STATE_HPP
#define REVELATION_STATE_HPP

#include "deck.hpp"
#include "step_impl.hpp"
#include "character.hpp"
#include "player.hpp"
#include "decision.hpp"
#include "setup/team.hpp"
#include "setup/effect.hpp"
#include "BoardTile.hpp"
#include "random.hpp"
#include "memory.hpp"
#include "constants.hpp"
#include "nlohmann/json_fwd.hpp"
#include <vector>
#include <tuple>
#include <forward_list>

class Agent;

using json = nlohmann::json;

using Board = std::array<std::array<BoardTile, FULL_BOARD_WIDTH>, 2>;
using UnitList = std::array<NullableShared<Character>, ARMY_SIZE>;

class State {
    Board board;
    std::array<unsigned short int, 2> nbAliveUnits;
    int turnID;
    std::forward_list<Effect*> unresolvedSpecialAbility;
public:
    Timestep timestep;
    uint8_t iActive = 3;
    std::array<UnitList, 2> units;
    std::array<Player, 2> players;

    State() = default;
    State(Generator& generator): players({ Player(Generator(generator() + 1)), Player(Generator(generator() + 1)) }) {};
    State& operator=(const State& copy);
    State(const State& copy){ *this = copy; } //calling the operator=
    State(State&& move) = default;
    bool operator==(const State& other) const {
        return board == other.board and nbAliveUnits == other.nbAliveUnits and turnID == other.turnID and unresolvedSpecialAbility == other.unresolvedSpecialAbility;
    }
    static State createStart(const std::array<const Team*, 2>& teams, Generator generator);

    const Board& getBoard() const { return board; }
    const std::array<unsigned short int, 2>& getNbAliveUnits() const { return nbAliveUnits; }
    const BoardTile& getBoardField(position coords) const;
    Character* getBoardFieldDeref(position coords);
    const Character* getBoardFieldDeref(position coords) const;
    void setBoardField(position coords, BoardTile value);
    void setBoardFieldDeref(position coords, BoardTile value);

    unsigned short int getWinner() const;
    void checkConsistency() const;

    std::tuple<State, uptr<DrawStep>> stepDraw() const;
    std::tuple<State, uptr<DiscardStep>> stepDiscard(DiscardDecision decision) const;
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
inline bool isDead(const NullableShared<Character>& cha){ return cha.get() == nullptr; }

std::ostream& operator<<(std::ostream& o, const Board& board);

#endif //REVELATION_STATE_HPP
