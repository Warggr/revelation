//
// Created by Diana Amirova on 11.07.22.
//

#ifndef REVELATION_PLAYER_HPP
#define REVELATION_PLAYER_HPP

#include "constants.hpp"
#include "string"
#include "deck.hpp"
#include "constants.hpp"
#include "random.hpp"
#include "nlohmann/json.hpp"
#include <initializer_list>
#include <variant>

using json = nlohmann::json;

class Player {
    static constexpr std::initializer_list<ActionCard> startingAbilityDeck = { DEFENSE, DEFENSE, HARDATK, HARDATK, HARDATK, HARDATK, SOFTATK, SOFTATK, SOFTATK, SOFTATK };
    std::vector<ActionCard> actions;
    std::vector<Faction> resources;

public:
    Deck<ActionCard> actionDeck;
    Deck<std::variant<ActionCard, Faction>> deck;
    Player() = default;
    Player(Generator generator, Deck<Faction> resourceDeck);

    std::variant<ActionCard, Faction> drawCard();

    void discard(ActionCard card);

    void discard(unsigned int card);

    void useActionCard(ActionCard cardValue);

    const std::vector<ActionCard>& getActions() const { return actions; }

    const std::vector<Faction>& getResourceCards() const { return resources; }

    friend void to_json(json&j, const Player& player);
};

#endif //REVELATION_PLAYER_HPP
