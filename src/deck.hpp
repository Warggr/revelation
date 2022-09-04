#ifndef REVELATION_DECK_HPP
#define REVELATION_DECK_HPP

#include "constants.hpp"
#include "random.hpp"
#include <tuple>
#include <forward_list>
#include <chrono>
#include <initializer_list>

template <typename T>
class Deck {
    std::vector<T> drawPile;
    std::vector<T> discardPile;
    Generator generator;

public:
    Deck() = default;
    Deck(std::vector<T> drawPile, std::vector<T> discardPile, Generator generator);
    constexpr bool operator==(const Deck& other) const { return drawPile == other.drawPile and discardPile == other.discardPile and generator == other.generator; }
    static Deck create(std::initializer_list<T> cards, Generator generator);
    void discard(T card);
    T draw();
    std::tuple<int, int> sizeconfig() const;
};

#endif //REVELATION_DECK_HPP
