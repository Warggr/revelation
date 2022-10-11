#ifndef REVELATION_POSITION_HPP
#define REVELATION_POSITION_HPP

#include "nlohmann/json_fwd.hpp"
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <cassert>

struct position {
    unsigned int row : 1;
    unsigned int column : 4;
public:
    position() = default;
    constexpr position(unsigned int row, unsigned int column): row(row), column(column) {}
    constexpr bool operator==(const position& other) const { return other.row == row and other.column == column; }

    friend std::ostream& operator<<(std::ostream& o, const position& pos);
    friend void to_json(nlohmann::json& j, const position& pos);
};

inline uint8_t manhattanDistance( position p1, position p2 ){
    return std::abs( p1.column - p2.column ) + abs( p1.row - p2.row );
}

enum Direction {
    UP = 0, LEFT = 1, RIGHT = 2, DOWN = 3
};

inline position getNeighbour(const position& pos, Direction dir) {
    switch(dir){
    case UP:
        return {static_cast<unsigned int>(pos.row - 1), pos.column};
    case DOWN:
        return {static_cast<unsigned int>(pos.row + 1), pos.column};
    case LEFT:
        return {pos.row, static_cast<unsigned int>(pos.column - 1)};
    case RIGHT:
        return {pos.row, static_cast<unsigned int>(pos.column + 1)};
    default:
        assert(false); return pos;
    }
}

#endif //REVELATION_POSITION_HPP
