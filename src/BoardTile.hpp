#ifndef REVELATION_BOARDTILE_HPP
#define REVELATION_BOARDTILE_HPP

class BoardTile {
public:
    uint8_t team;
    int index;

    BoardTile() = default;
    constexpr BoardTile(uint8_t team, int index): team(team), index(index) {}

    constexpr bool operator==(const BoardTile& other) const {
        return other.team == this->team && other.index == this->index;
    }

    static constexpr BoardTile empty() { return { 3, 0 }; };
    static inline constexpr bool isEmpty(const BoardTile& tile){ return tile == BoardTile::empty(); }
};

static_assert(BoardTile::isEmpty(BoardTile::empty()) );

#endif //REVELATION_BOARDTILE_HPP
