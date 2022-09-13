#include "player.hpp"
#include "deck.hpp"
#include "constants.hpp"
#include <iostream>
#include <algorithm>
#include "random.hpp"

Player::Player(Generator generator, Deck<Faction> resourceDeck) {
    actionDeck = Deck<ActionCard>::create(startingAbilityDeck, generator);
    for(size_t i = 0; i < actionDeck.size(); i++) {
        this->deck.at(i) = actionDeck.at(i);
    }

    for(size_t i = 0; i < resourceDeck.size(); i++) {
        this->deck.at(i + actionDeck.size()) = resourceDeck.at(i);
    }
}

std::variant<ActionCard, Faction> Player::drawCard() {
    std::variant<ActionCard, Faction> cardDrawn = this->deck.draw();
    if(cardDrawn.index() == 0) {
        actions.insert(actions.end(), std::get<0>(cardDrawn));
    } else {
        resources.insert(resources.end(), std::get<1>(cardDrawn));
    }
    return cardDrawn;
}

void Player::discard(ActionCard card) {
    if(std::find(actions.begin(), actions.end(), card) == actions.end()) {
        std::cout << "Actions: \n";
        for(auto i: actions)
            std::cout << i << " , ";
        std::cout << "Card " << card;
        throw 1;
    }
    std::remove(actions.begin(), actions.end(), card);
    actionDeck.discard(card);
}

void Player::discard(unsigned int iCard) {
    auto card = actions[iCard];
    actions.erase(actions.begin() + iCard);
    actionDeck.discard(card);
}

void Player::useActionCard(ActionCard cardValue) {
    std::remove(actions.begin(), actions.end(), cardValue);
    actionDeck.discard(cardValue);
}