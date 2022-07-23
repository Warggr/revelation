//
// Created by Diana Amirova on 11.07.22.
//

#ifndef REVELATION_PLAYER_HPP
#define REVELATION_PLAYER_HPP

#include "constants.hpp"
#include "string"
#include "deck.hpp"
#include "constants.hpp"
#include "initializer_list"

class Player {
    static constexpr std::initializer_list<ActionCard> startingAbilityDeck = { DEFENSE, DEFENSE, HARDATK, HARDATK, HARDATK, HARDATK, SOFTATK, SOFTATK, SOFTATK, SOFTATK };
    std::vector<ActionCard> abilityCheck;
    std::vector<ActionCard> actions;
    std::vector<Faction> resources;

public:
    Deck<ActionCard> actionDeck;
    Deck<ActionCard> resourceDeck;
    Player(std::vector<ActionCard> abilityCheck);

    ActionCard drawAction();

    Faction drawResource(Deck<Faction> resourceDeck);

    void discard(ActionCard card);

    void useActionCard(ActionCard cardValue);
};


#endif //REVELATION_PLAYER_HPP
