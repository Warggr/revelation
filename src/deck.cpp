//
// Created by Diana Amirova on 11.07.22.
//

#include "deck.hpp"
#include <algorithm>
#include <random>

int seed;

template <typename T>
Deck<T>::Deck(std::vector<T> drawPile, std::vector<T> discardPile, Generator generator): generator(generator) {
    this->drawPile = drawPile;
    this->discardPile = discardPile;
}

template <typename T>
void Deck<T>::discard(T card) {
    this->discardPile.insert(this->discardPile.end(), card);
}

template <typename T>
Deck<T> Deck<T>::create(std::initializer_list<T> cards) {
    std::vector<T> cardsAsVector = cards;
    Generator generator(seed++); //change the seed slightly so that the next Deck created does not use exactly the same

    std::shuffle(std::begin(cardsAsVector), std::end(cardsAsVector), generator);
    generator.discard(1);
    return Deck(std::move(cardsAsVector), std::vector<T>(), generator);
}

template <typename T>
T Deck<T>::draw() {
    if(this->drawPile.empty()) {
        drawPile = std::move(discardPile);
        this->discardPile = std::vector<T>();
        std::shuffle(std::begin(drawPile), std::end(drawPile), generator);
        generator.discard(1); //std::shuffle only copies and does not change the generator - we don't want it to stay always the same,
        //so we change it manually
    }
    T retVal = drawPile.back();
    drawPile.pop_back();
    return retVal;
}

template <typename T>
std::tuple<int, int> Deck<T>::sizeconfig() const {
    return std::make_tuple(this->drawPile.size(), this->discardPile.size());
}

template class Deck<ActionCard>;
template class Deck<Faction>;
