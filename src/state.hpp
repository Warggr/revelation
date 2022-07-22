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
#include "../cmake-build-debug/_deps/json-src/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

class State {
    std::vector<std::vector<BoardTile>> board;
    std::vector<int> nbAliveUnits;
    Deck<Faction> resDeck;
    uint8_t iActive;
    Timestep timestep;
    int turnID;

public:
    std::vector<std::vector<character>> units;
    std::vector<Player> players;
    State(std::vector<std::vector<BoardTile>> board, std::vector<std::vector<character>> units, std::vector<Player> players, Deck<Faction> resDeck, Timestep timestep, int turnID);
    State createStart(Team teams[2]);
    BoardTile* getBoardField(position coords);
    character* getBoardFieldDeref(position coords);
    void setBoardField(position coords, BoardTile* value);
    void setBoardFieldDeref(position coords, BoardTile* value);
    bool isFinished();
    std::tuple<State*, Step>  stepDraw(ActionOrResource decision);
    void checkConsistency();
    std::tuple<State*, Step> stepMov(MoveDecision decision);
    std::tuple<State*, Step>  stepAbil(AbilityDecision decision);
    std::tuple<State*, Step> stepAct(ActionDecision decision);
    position getNeighbour(position pos, Direction dir);
    std::tuple<State*, Step> beginTurn();
    std::tuple<State*, Step> advance(Agent agent);
    json to_json(nlohmann::basic_json<> &j, const State &state);
};

#endif //REVELATION_STATE_HPP
