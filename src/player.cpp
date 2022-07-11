//
// Created by Diana Amirova on 03.07.22.
//

#include "player.hpp"
#include "Deck.hpp"
#include <iostream>
#include "constants.hpp"

Player::Player(std::vector<ActionCard> abilityCheck): actionDeck(actionDeck), resourceDeck(resourceDeck) {
    this->abilityCheck = abilityCheck;
    this->actionDeck = Deck<ActionCard> { this->startingAbilityDeck };
}

ActionCard Player::drawAction() {
    ActionCard cardDrawn = actionDeck.draw();
    actions.insert(actions.end(), cardDrawn);
    return cardDrawn;
}

ActionCard Player::drawResource(Deck<ActionCard> resourceDeck) {
    ActionCard cardDrawn = resourceDeck.draw();
    resources.insert(resources.end(), cardDrawn);
    return cardDrawn;
}

void Player::discard(ActionCard card) {
    if(std::find(actions.begin(), actions.end(), card) != actions.end()) {
        std::cout << "Actions: \n";
        for(ActionCard i: actions)
            std::cout << i << ' , ';
        std::cout << "Card " << card;
    }
    std::remove(actions.begin(), actions.end(), card);
    actionDeck.discard(card);
}

void Player::useActionCard(ActionCard cardValue) {
    std::remove(actions.begin(), actions.end(), cardValue);
    actionDeck.discard(cardValue);
}

