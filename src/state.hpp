#ifndef REVELATION_STATE_H
#define REVELATION_STATE_H

#include "constants.hpp"
#include "deck.hpp"
#include "team.hpp"
#include <vector>
#include <tuple>

class Character; class Player;

class State {
    std::vector<std::vector<character>> board;
    std::vector<std::vector<character>> units;
    std::vector<uint8_t> nbAliveUnits;
    std::vector<Player> players;
    Deck<ActionCard> resDeck;
    uint8_t iActive;
    Timestep timestep;

public:
    State(std::vector<std::vector<character>> board, std::vector<std::vector<character>> units,
          std::vector<uint8_t> nbAliveUnits, std::vector<Player> players, Deck<ActionCard> resDeck, uint8_t iActive, Timestep timestep);

    static State createStart(Team teams[2]);
};

#endif //REVELATION_STATE_H
