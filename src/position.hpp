#ifndef REVELATION_POSITION_H
#define REVELATION_POSITION_H

#include <cmath>
#include <cstdint>

struct position {
    bool row;
    uint8_t column;
};

uint8_t manhattanDistance( position p1, position p2 ){
    return std::abs( p1.column - p2.column ) + abs( p1.row - p2.row );
};

#endif //REVELATION_POSITION_H