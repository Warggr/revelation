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

using CombatDeckCard = std::variant<ActionCard, Faction>;
std::ostream& operator<<(const CombatDeckCard& card, std::ostream& os);

class Player {
    static constexpr std::initializer_list<CombatDeckCard> startingAbilityDeck = { DEFENSE, DEFENSE, HARDATK, HARDATK, HARDATK, HARDATK, SOFTATK, SOFTATK, SOFTATK, SOFTATK };
    std::vector<ActionCard> actions;
    std::vector<Faction> resources;

public:
    Deck<CombatDeckCard> deck;
    Player() = default;
    Player(Generator generator);

    CombatDeckCard drawCard();

    void discard(CombatDeckCard card);
    void discardAction(unsigned int card);
    void discardAction(ActionCard card);
    void discardResource(unsigned int card);
    void discardResource(Faction card);

    const std::vector<ActionCard>& getActions() const { return actions; }

    const std::vector<Faction>& getResourceCards() const { return resources; }

    friend void to_json(json&j, const Player& player);
};

#endif //REVELATION_PLAYER_HPP
