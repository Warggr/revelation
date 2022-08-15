#ifndef REVELATION_BOARDTILE_HPP
#define REVELATION_BOARDTILE_HPP

class BoardTile {
public:
    uint8_t team;
    unsigned int index;

    constexpr BoardTile(): team(3), index(0){};
    constexpr BoardTile(uint8_t team, int index): team(team), index(index) {}

    constexpr bool operator==(const BoardTile& other) const {
        return other.team == this->team && other.index == this->index;
    }

    static constexpr BoardTile empty() { return BoardTile(); };
    static inline constexpr bool isEmpty(const BoardTile& tile){ return tile == BoardTile::empty(); }
};

static_assert(BoardTile::isEmpty(BoardTile::empty()) );

#endif //REVELATION_BOARDTILE_HPP
