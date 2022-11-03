
#ifndef REVELATION_CONSTANTS_HPP
#define REVELATION_CONSTANTS_HPP

#include "position.hpp"
#include "nlohmann/json_fwd.hpp"
#include <cassert>
#include <string>

constexpr int MAX_CARDS_IN_HAND = 7;
constexpr int MAX_ABILITIES = 4;

constexpr int ARMY_WIDTH = 3;
constexpr int HALF_BOARD_WIDTH = ARMY_WIDTH + 2;
constexpr int FULL_BOARD_WIDTH = 2*HALF_BOARD_WIDTH;
constexpr int NB_CHARACTERS = 2*ARMY_WIDTH;
constexpr int ARMY_SIZE = 2*ARMY_WIDTH;

enum Faction {
    NONE = 0, BLOOD = 1, MERCURY = 2, HORROR = 3, SPECTRUM = 4, ETHER = 5,
};
void to_json(nlohmann::json& j, const Faction& faction);

enum Timestep {
    BEGIN = 0, DREW = 1, DISCARDED = 2, MOVEDfirst = 3, MOVEDlast = 4, ABILITYCHOSEN = 5, ACTED = 6
};
void to_json(nlohmann::json& j, const Timestep& step);

enum ActionCard {
    HARDATK , SOFTATK , DEFENSE, SPECIALACTION
};
void to_json(nlohmann::json& j, const ActionCard& card);

inline std::string_view to_string(enum ActionCard card){
    switch(card){
        case DEFENSE: return "Defense";
        case HARDATK: return "Hard Attack";
        case SOFTATK: return "Soft Attack";
        default: assert(false); return "<Internal Error>";
    }
}

#define to_string_line(x) case x: return #x

inline std::string_view to_string(enum Timestep step){
    switch(step){
        to_string_line(BEGIN);
        to_string_line(DREW);
        to_string_line(DISCARDED);
        to_string_line(MOVEDfirst);
        to_string_line(MOVEDlast);
        to_string_line(ABILITYCHOSEN);
        to_string_line(ACTED);
        default: assert(false); return "<Internal Error>";
    }
}

inline std::string_view to_string(enum Faction faction){
    switch(faction){
        to_string_line(NONE);
        to_string_line(BLOOD);
        to_string_line(MERCURY);
        to_string_line(HORROR);
        to_string_line(SPECTRUM);
        to_string_line(ETHER);
        default: assert(false); return "<Internal Error>";
    }
}

inline std::string_view to_string(enum Direction dir){
    switch(dir){
        to_string_line(UP);
        to_string_line(DOWN);
        to_string_line(LEFT);
        to_string_line(RIGHT);
        default: assert(false); return "<Internal Error>";
    }
}

#undef to_string_line

inline char to_symbol(enum Direction dir){
    switch(dir){
        case UP: return '^';
        case DOWN: return 'v';
        case LEFT: return '<';
        case RIGHT: return '>';
        default: assert(false); return '?';
    }
}

#endif //REVELATION_CONSTANTS_HPP
