#ifndef REVELATION_DECK_HPP
#define REVELATION_DECK_HPP

#include "constants.hpp"
#include <tuple>
#include <forward_list>
#include <random>
#include <chrono>
#include <initializer_list>

extern int seed;
using Generator = std::minstd_rand0;

template <typename T>
class Deck {
    std::vector<T> drawPile;
    std::vector<T> discardPile;
    Generator generator;

public:
    Deck() = default;
    Deck(std::vector<T> drawPile, std::vector<T> discardPile, Generator generator);
    static Deck create(std::initializer_list<T> cards);
    void discard(T card);
    T draw();
    std::tuple<int, int> sizeconfig() const;
};

#endif //REVELATION_DECK_HPP
