#include "player.hpp"
#include "deck.hpp"
#include "constants.hpp"
#include "random.hpp"
#include <ostream>
#include <algorithm>
#include <cassert>

Player::Player(Generator generator) {
    deck = Deck<CombatDeckCard>::create(startingAbilityDeck, generator);
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

std::ostream& operator<<(std::ostream& os, const CombatDeckCard& card){
    if(card.index() == 0) return os << std::get<0>(card);
    else return os << std::get<1>(card);
}

void Player::discard(CombatDeckCard card) {
    if(card.index() == 0) discardAction(std::get<ActionCard>(card));
    else discardResource(std::get<Faction>(card));
}

void Player::discardAction(unsigned int iCard) {
    assert(iCard < actions.size());
    auto card = actions[iCard];
    actions.erase(actions.begin() + iCard);
    deck.discard(card);
}

void Player::discardAction(ActionCard card) {
    auto cardPosition = std::find(actions.begin(), actions.end(), card);
    assert(cardPosition != actions.end());
    actions.erase(cardPosition);
    deck.discard(card);
}

void Player::discardResource(unsigned int iCard) {
    assert(iCard < resources.size());
    auto card = resources[iCard];
    actions.erase(actions.begin() + iCard);
    deck.discard(card);
}

void Player::discardResource(Faction card) {
    auto cardPosition = std::find(resources.begin(), resources.end(), card);
    assert(cardPosition != resources.end());
    resources.erase(cardPosition);
    deck.discard(card);
}
