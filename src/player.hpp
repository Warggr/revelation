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
#include

class Player {
    std::vector<ActionCard> abilityCheck;
    std::vector<ActionCard> actions;
    std::vector<ActionCard> resources;
    std::initializer_list<ActionCard> startingAbilityDeck;

public:
    Deck<ActionCard> actionDeck;
    Deck<ActionCard> resourceDeck;
    Player(std::vector<ActionCard> abilitsyCheck);

    template<typename T>
    auto drawAction();

    template<typename T>
    auto drawResource(Deck<T> resourceDeck);

    template<typename T>
    void discard(T card);

    template<typename T>
    void useActionCard(T cardValue);
};

#endif //REVELATION_PLAYER_HPP
