#ifndef REVELATION_BOARDTILE_HPP
#define REVELATION_BOARDTILE_HPP

#include "string"
class BoardTile {
public:
    uint8_t team;
    int index;

    BoardTile(uint8_t team, int index) {
        this->team = team;
        this->index = index;
    }

    bool eguals(BoardTile other) {
        if(&other == NULL)
            return false;
        else
            return other.team == this->team && other.index == this->index;
    }

    void repr(){
        // TODO
    };
};


#endif //REVELATION_BOARDTILE_HPP
