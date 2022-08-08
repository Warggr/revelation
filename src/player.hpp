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
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Player {
    static constexpr std::initializer_list<ActionCard> startingAbilityDeck = { DEFENSE, DEFENSE, HARDATK, HARDATK, HARDATK, HARDATK, SOFTATK, SOFTATK, SOFTATK, SOFTATK };
    std::vector<ActionCard> actions;
    std::vector<Faction> resources;

public:
    Deck<ActionCard> actionDeck;
//    Player() = default;
    Player();

    ActionCard drawAction();

    Faction drawResource(Deck<Faction> resourceDeck);

    void discard(ActionCard card);

    void useActionCard(ActionCard cardValue);

    const std::vector<ActionCard>& getActions() const { return actions; }

    json to_json(nlohmann::basic_json<> &j) const;
};

#endif //REVELATION_PLAYER_HPP
