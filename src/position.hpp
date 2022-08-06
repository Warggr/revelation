#ifndef REVELATION_POSITION_HPP
#define REVELATION_POSITION_HPP

#include <cmath>
#include <cstdint>

struct position {
    int row;
    int column;

public:
    position() = default;
    position(int row, int column) {
        this->row = row;
        this->column = column;
    }
};

uint8_t manhattanDistance( position p1, position p2 ){
    return std::abs( p1.column - p2.column ) + abs( p1.row - p2.row );
};

enum Direction {
    UP = 0, LEFT = 1, RIGHT = 2, DOWN = 3
};

position getNeighbour(const position& pos, Direction dir) {
    switch(dir){
    case UP:
        return position(pos.row - 1, pos.column);
    case DOWN:
        return position(pos.row + 1, pos.column);
    case LEFT:
        return position(pos.row, pos.column - 1);
    case RIGHT:
        return position(pos.row, pos.column + 1);
    }
}

#endif //REVELATION_POSITION_HPP
