//
// Created by Diana Amirova on 11.07.22.
//

#include "deck.hpp"
#include <algorithm>
#include <random>

template <typename T>
Deck<T>::Deck(std::vector<T> drawPile, std::vector<T> discardPile, int seed) {
    this->drawPile = drawPile;
    this->discardPile = discardPile;
    this->seed = seed;
}

template <typename T>
void Deck<T>::discard(T card) {
    this->discardPile.insert(this->discardPile.end(), card);
}

template <typename T>
Deck<T> Deck<T>::create(std::initializer_list<T> cards) {
    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(cards), std::end(cards), rng);
    int seed = rand() % 100001;
    return Deck(cards, std::vector<T>(), seed);
}

template <typename T>
T Deck<T>::draw() {
    if(this->drawPile.empty()) {
        this->drawPile = this->discardPile;
        this->discardPile = std::vector<T>();

        //random.seed(self.seed) //FRAGEN

        auto rng = std::default_random_engine {};
        std::shuffle(std::begin(this->drawPile), std::end(this->drawPile), rng);
        this->seed = rand() % 100001;
    }
    return this->drawPile.pop_back();
}

template <typename T>
std::tuple<int, int> Deck<T>::sizeconfig() const {
    return std::make_tuple(this->drawPile.size(), this->discardPile.size());
}