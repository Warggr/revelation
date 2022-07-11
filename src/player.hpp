//
// Created by Diana Amirova on 11.07.22.
//

#ifndef REVELATOR_PLAYER_HPP
#define REVELATOR_PLAYER_HPP

#endif //REVELATOR_PLAYER_HPP

#include "constants.hpp"
#include "string"
#include "deck.hpp"
#include "constants.hpp"
#include "initializer_list"

class Player {
    std::vector<ActionCard> abilityCheck;
    std::vector<ActionCard> actions;
    std::vector<ActionCard> resources;
    std::initializer_list<ActionCard> startingAbilityDeck = std::initializer_list<ActionCard> { DEFENSE, DEFENSE, HARDATK, HARDATK, HARDATK, HARDATK, SOFTATK, SOFTATK, SOFTATK, SOFTATK };

public:
    Deck<ActionCard> actionDeck;
    Deck<ActionCard> resourceDeck;
    Player(std::vector<ActionCard> abilityCheck);
    ActionCard drawAction();
    ActionCard drawResource(Deck<ActionCard> resourceDeck);
    void discard(ActionCard card);
    void useActionCard(ActionCard cardValue);
};