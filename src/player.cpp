#include "player.hpp"
#include "deck.hpp"
#include "constants.hpp"
#include <iostream>
#include <algorithm>

Player::Player() {
    actionDeck = Deck<ActionCard>::create(startingAbilityDeck);
}

ActionCard Player::drawAction() {
    ActionCard cardDrawn = actionDeck.draw();
    actions.insert(actions.end(), cardDrawn);
    return cardDrawn;
}

Faction Player::drawResource(Deck<Faction> resourceDeck)  {
    Faction cardDrawn = resourceDeck.draw();
    resources.insert(resources.end(), cardDrawn);
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
