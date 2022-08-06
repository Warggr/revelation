#ifndef REVELATION_DECK_HPP
#define REVELATION_DECK_HPP

#pragma once

#include <forward_list>
#include <random>
#include <chrono>
#include <initializer_list>
#include "constants.hpp"

template <typename T>
class Deck {
    std::vector<T> drawPile;
    std::vector<T> discardPile;
    static int seed;

public:
    Deck() = default;
    Deck(std::vector<T> drawPile, std::vector<T> discardPile, int seed);
    static Deck create(std::initializer_list<T> cards);
    void discard(T card);
    T draw();
    std::tuple<int, int> sizeconfig();
};

#endif //REVELATION_DECK_HPP
