#ifndef REVELATION_POSITION_HPP
#define REVELATION_POSITION_HPP

#include <cmath>
#include <cstdint>

struct position {
    int row;
    int column;

public:
    position(bool row, uint8_t column) {
        this->row = row;
        this->column = column;
    }
};

uint8_t manhattanDistance( position p1, position p2 ){
    return std::abs( p1.column - p2.column ) + abs( p1.row - p2.row );
};

#endif //REVELATION_POSITION_HPP
