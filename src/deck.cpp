//
// Created by Diana Amirova on 11.07.22.
//

#include "deck.hpp"
#include <algorithm>
#include <random>
#include <variant>

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
Deck<T> Deck<T>::create(std::initializer_list<T> cards, Generator generator) {
    std::vector<T> cardsAsVector = cards;

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

template <typename T>
size_t Deck<T>::size() {
    return this->drawPile.size();
}

template <typename T>
T Deck<T>::at(size_t i) {
    return this->drawPile.at(i);
}
template class Deck<std::variant<ActionCard, Faction>>;
//Explicitly instantiate all templates that will be used (else we get linker errors)
