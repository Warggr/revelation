
#ifndef REVELATION_CONSTANTS_HPP
#define REVELATION_CONSTANTS_HPP

#include "string"
#include "position.hpp"

constexpr int MAX_RESOURCES = 3;
constexpr int MAX_ACTIONS = 4;
constexpr int MAX_ABILITIES = 4;

constexpr int ARMY_WIDTH = 3;
constexpr int HALF_BOARD_WIDTH = ARMY_WIDTH + 2;
constexpr int FULL_BOARD_WIDTH = 2*HALF_BOARD_WIDTH;
constexpr int NB_CHARACTERS = 2*ARMY_WIDTH;
constexpr int ARMY_SIZE = 2*ARMY_WIDTH;

enum Faction {
    NONE = 0, BLOOD = 1, MERCURY = 2, HORROR = 3, SPECTRUM = 4, ETHER = 5,
};

enum Timestep {
    BEGIN = 0, DREW = 1, DISCARDED = 2, MOVEDfirst = 3, MOVEDlast = 4, ABILITYCHOSEN = 5, ACTED = 6
};

enum ActionCard {
    HARDATK , SOFTATK , DEFENSE
};

enum ActionOrResource {
    ACTION = true, RESOURCES = false
};

enum Direction {
    UP = 0, LEFT = 1, RIGHT = 2, DOWN = 3
};

struct AbilityDecision {
    std::string type;
};

class ActionDecision {
public:
    ActionCard card;
    position subjectPos;
    position objectPos;

    ActionDecision(ActionCard card, position subjectPos, position objectPos)
            : objectPos(objectPos), subjectPos(subjectPos) {
        this->card = card;
        this->subjectPos = subjectPos;
        this->objectPos = objectPos;
    }

};

#endif //REVELATION_CONSTANTS_HPP
