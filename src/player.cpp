#include "player.hpp"
#include "Deck.hpp"
#include <iostream>
#include "constants.hpp"

Player::Player(std::vector<ActionCard> abilityCheck): actionDeck(actionDeck), resourceDeck(resourceDeck) {
    this->abilityCheck = abilityCheck;
    this->actionDeck = Deck<ActionCard>::create(this->startingAbilityDeck);
    this->startingAbilityDeck = std::initializer_list<ActionCard> { DEFENSE, DEFENSE, HARDATK, HARDATK, HARDATK, HARDATK, SOFTATK, SOFTATK, SOFTATK, SOFTATK };
}

template<typename T>
auto Player::drawAction() {
    T cardDrawn = actionDeck.draw();
    actions.insert(actions.end(), cardDrawn);
    return cardDrawn;
}

template<typename T>
auto Player::drawResource(Deck<T> resourceDeck) {
    T cardDrawn = resourceDeck.draw();
    resources.insert(resources.end(), cardDrawn);
    return cardDrawn;
}

template<typename T>
void Player::discard(T card) {
    if(std::find(actions.begin(), actions.end(), card) != actions.end()) {
        std::cout << "Actions: \n";
        for(T i: actions)
            std::cout << i << ' , ';
        std::cout << "Card " << card;
    }
    std::remove(actions.begin(), actions.end(), card);
    actionDeck.discard(card);
}

template<typename T>
void Player::useActionCard(T cardValue) {
    std::remove(actions.begin(), actions.end(), cardValue);
    actionDeck.discard(cardValue);
}

